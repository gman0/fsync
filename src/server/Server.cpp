/*
	Copyright (C) 2011 Róbert Vašek <gman@codefreax.org>

    This file is part of fsync.

    fsync is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    fsync is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fsync.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <cstdlib>
#include <string>
#include <utility>
#include <unistd.h>
#include "FSException.h"
#include "Server.h"
#include "FileGatherer.h"
#include "PathTransform.h"
#include "ProcessFile_load.h"
#include "Rollback.h"
#include "pack_unpack.h"

using namespace std;
using namespace boost::filesystem;

Server::Server(int argc, char ** argv) :
	AppInterface(argc, argv),
	m_pathTransform(0),
	m_fileGatherer(0),
	m_networkManager(0),
	m_rollbackSolver(0),
	m_willRollback(false)
{
	ID_Path_pairList id_path_pairList = m_config->getPathList();
	m_pathTransform = new PathTransform(id_path_pairList);
	m_fileGatherer = new FileGatherer(m_config, m_pathTransform, id_path_pairList);

	if (!m_config->updateDbAndQuit())
	{
		m_rollbackSolver = new Rollback(m_config, m_fileGatherer);

		if (exists(m_config->getRollbackFilePath()) && !m_config->ignoreRb())
		{
			m_fileGatherer->setRollbackSolver(m_rollbackSolver);
			m_rollbackSolver->buildRollbackProxies();
			m_willRollback = true;

			cout << "fsync has detected " << m_rollbackSolver->getRollbackProxyVector().size() <<
					" rollback(s) from the previous session and will try to redo the actions." << endl;
		}

		cout << "Gathering changes..." << flush;
		m_proxyVector = m_fileGatherer->getChanges();

		m_networkManager = new NetworkManager(m_config->getPort(), m_config->getRecvTimeout(),
												m_config->getSendTimeout());

		getClient();

		m_buffer = new unsigned char[ProcessFileInterface::LARGE_CHUNK];


		transferFiles();

		// recieve the last packet
		PacketHeader ph_final;
		m_networkManager->recv(&ph_final, sizeof(PacketHeader));

		m_networkManager->closeConnection();

		if (ph_final.m_type == PACKET_FINISHED)
			cout << "Synchronization finished successfuly." << endl;
		else
			cout << "There was an error during synchronization." << endl;

		if (m_proxyVector.size() > 0)
			updateDb();
	}
	else
		updateDb();
}

Server::~Server()
{
	cout << "Quitting..." << endl;

	delete [] m_buffer;
	delete m_networkManager;
	delete m_rollbackSolver;
	delete m_fileGatherer;
	delete m_pathTransform;
}

void Server::updateDb()
{
	if (m_config->dontSaveDb())
		return;

	cout << "Updating database..." << flush;
	m_fileGatherer->updateDb();
	cout << "done." << endl;
}

void Server::getClient()
{
	cout << "Waiting for client (^C to quit)..." << endl;

	m_networkManager->listen();

	while (!m_networkManager->acceptConnection())
		sleep(1);

	char address[INET_ADDRSTRLEN];
	m_networkManager->getClientAddress(address, sizeof(address));

	cout << "Connection established with " << address << ':' << m_networkManager->getPort() << endl;
}

void Server::transferFilesLoop(const FileGatherer::FIProxyPtrVector & proxies)
{
	for (FileGatherer::FIProxyPtrVector::const_iterator proxyIt = proxies.begin();
		proxyIt != proxies.end(); proxyIt++)
	{
		FileGatherer::FileInfoProxy * proxy = *proxyIt;

		cout << m_fileGatherer->getFileInfo(proxy).m_path;

		PacketHeader ph = prepareFileTransfer(proxy);

		if (ph.m_type == PACKET_RESPONE_FREE_SPACE_A_NEW)
			handleNew(unpackFromHeader<bool>(&ph, PACKET_RESPONE_FREE_SPACE_A_NEW), proxy);
		else if (ph.m_type == PACKET_RESPONE_FREE_SPACE_A_CHANGE)
			handleChange(unpackFromHeader<bool>(&ph, PACKET_RESPONE_FREE_SPACE_A_CHANGE), proxy);
		else if (ph.m_type == PACKET_NEXT)
			continue;
		else // something has gone wrong
			addRollback(proxy);
	}
}

void Server::transferFiles()
{
	FileGatherer::FIProxyPtrVector & rollbackProxyVec = m_rollbackSolver->getRollbackProxyVector();

	sendObjectCount(m_proxyVector.size() + rollbackProxyVec.size());

	if (m_willRollback)
		transferFilesLoop(rollbackProxyVec);

	transferFilesLoop(m_proxyVector);
}

void Server::sendObjectCount(size_t size)
{
	PacketHeader ph = packToHeader<size_t>(&size, PACKET_OBJECT_COUNT);
	m_networkManager->send(&ph, sizeof(PacketHeader));
}

PacketHeader Server::prepareFileTransfer(const FileGatherer::FileInfoProxy * proxy)
{
	/* send request for a file */

	FileGatherer::FileInfo fi = m_fileGatherer->getFileInfo(proxy);
	PacketHeader ph = packFileInfo(&fi, proxy->m_flags);
	m_networkManager->send(&ph, sizeof(PacketHeader));


	/* recieve answer what to do next */

	PacketHeader ph_respone;
	m_networkManager->recv(&ph_respone, sizeof(PacketHeader));

	return ph_respone;
}

void Server::handleNew(bool hasFreeSpace, FileGatherer::FileInfoProxy * proxy)
{
	if (hasFreeSpace)
	{
		ProcessFile_load file(m_fileGatherer->getFileInfo(proxy).m_path, m_buffer);

		unsigned int blocksCount = file.getBlocksCount();

		for (unsigned int i = 0; i < blocksCount; i++)
			m_networkManager->send(file.nextBlock(), sizeof(PacketData));
	}
	else
		addRollback(proxy);
}

void Server::handleChange(bool hasFreeSpace, FileGatherer::FileInfoProxy * proxy)
{
	if (hasFreeSpace)
	{
		ProcessFile_load file(m_fileGatherer->getFileInfo(proxy).m_path, m_buffer);

		// search for chunks and send them
		recursiveChunkSearch(&file, file.getSize());

		// we're done
		PacketHeader ph_next;
		ph_next.m_type = PACKET_NEXT;

		m_networkManager->send(&ph_next, sizeof(PacketHeader));
	}
	else
		addRollback(proxy);
}

void Server::recursiveChunkSearch(ProcessFile_load * file, offset_t endRange)
{
	while (file->getOffset() < endRange)
	{
		PacketHeader ph;

		// send PacketHeader_ChunkInfo so that client knows what to look for
		ProcessFileInterface::ChunkInfo ci = file->getCurrentChunkInfo();
		PacketHeader_ChunkInfo ph_ci;

		ph_ci.m_chunkType = ci.first;;
		ph_ci.m_offset = ci.second;

		ph = packToHeader<PacketHeader_ChunkInfo>(&ph_ci, PACKET_CHUNK_INFO);
		m_networkManager->send(&ph, sizeof(PacketHeader));

		// recieve the answer from client
		m_networkManager->recv(&ph, sizeof(PacketHeader));
		hash_t cl_ciHash = unpackFromHeader<hash_t>(&ph, PACKET_CHUNK_HASH); // client's chunk info hash

		if (cl_ciHash == file->getHash(ci))
		{
			file->setOffset(file->getOffset() + (int)file->getZoom());
			continue;
		}
		else
		{
			if (file->zoomIn())
				recursiveChunkSearch(file, file->getOffsetRange(ci));
			else
			{
				// announce that we're going to send the whole chunk
				PacketHeader announce_ph = packToHeader<offset_t>(&ci.second, PACKET_CHUNK);
				m_networkManager->send(&announce_ph, sizeof(PacketHeader));

				// send the actual chunk
				PacketData * pd = file->getBlock(ci.second);
				m_networkManager->send(pd, sizeof(PacketData));
			}
		}
	}

	file->zoomOut();
}

PacketHeader Server::packFileInfo(const FileGatherer::FileInfo * fi, short int flags)
{
	PacketHeader_FileInfo ph_fi;

	ph_fi.m_pathId = fi->m_pathId;
	ph_fi.m_action = (PacketHeader_FileInfo::ACTION)m_fileGatherer->getAction(flags);

	if (ph_fi.m_action != PacketHeader_FileInfo::A_DELETE)
	{
		ph_fi.m_size = file_size(path(fi->m_path));

		if (m_config->storeChangeTimestamp())
			ph_fi.m_lastWrite = fi->m_lastWrite;

		if (m_config->storePermissions())
			ph_fi.m_permissions = fi->m_permissions;
	}
	else
		ph_fi.m_size = 0; // We can't use file_size on a deleted file

	cout << " (" << ph_fi.m_size << ")" << endl;

	m_pathTransform->cutPath(fi->m_pathId, fi->m_path, ph_fi.m_path);

	return PacketHeader(PACKET_FILE_INFO, &ph_fi, sizeof(PacketHeader_FileInfo));
}

void Server::processRollbacks()
{
	if (m_rollbackProxyVector.size() > 0)
	{
		cout << "Rolling back changes..." << flush;
		m_rollbackSolver->commitRollbacks(m_rollbackProxyVector);
		cout << "done." << endl;
	}
	else
		m_rollbackSolver->cleanRollbackFile();
}

void Server::addRollback(FileGatherer::FileInfoProxy * proxy)
{
	m_rollbackProxyVector.push_back(proxy);
}

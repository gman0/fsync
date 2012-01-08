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
#include "FSException.h"
#include "Server.h"
#include "PacketContainer.h"
#include "FileGatherer.h"
#include "PathTransform.h"
#include "ProcessFile_load.h"
#include "pack_unpack.h"

using namespace std;
using namespace boost::filesystem;

Server::Server(int argc, char ** argv) : AppInterface(argc, argv)
{
	ID_Path_pairList id_path_pairList = m_config->getPathList();
	m_pathTransform = new PathTransform(id_path_pairList);
	m_fileGatherer = new FileGatherer(m_config, m_pathTransform, id_path_pairList);

	cout << "Gathering changes..." << flush;
	m_proxyVector = m_fileGatherer->getChanges();

	m_networkManager = new NetworkManager(m_config->getPort());
	m_networkManager->openSocket();

	getClient();

	transferFiles();

	// recieve the last packet
	PacketHeader ph_final;
	m_networkManager->recv(&ph_final, sizeof(PacketHeader));

	if (ph_final.m_type == PACKET_FINISHED)
		cout << "Synchronization finished successfuly. Quitting..." << endl;

	m_networkManager->closeClientConnection();

	m_networkManager->closeConnection();
}

Server::~Server()
{
	delete m_fileGatherer;
	delete m_pathTransform;
	delete m_networkManager;
}

/*
 * TODO: this should be in an infinite loop
 */
void Server::getClient()
{
	cout << "Waiting for client..." << endl;

	while (!m_networkManager->acceptConnection())
		SDL_Delay(100);

	IPaddress * ip = m_networkManager->getClientAddress();
	char ipAddress[16];
	m_networkManager->getClientIPAddress(ipAddress);

	cout << "Connection established with " << ipAddress << ':' << ip->port << endl;
}

void Server::transferFiles()
{
	sendObjectCount(m_proxyVector.size());

	for (FileGatherer::FIProxyPtrVector::const_iterator proxyIt = m_proxyVector.begin();
		proxyIt != m_proxyVector.end(); proxyIt++)
	{
		PacketHeader ph = prepareFileTransfer(*proxyIt);

		if (ph.m_type == PACKET_RESPONE_FREE_SPACE)
			handleNew(unpackFromHeader<bool>(&ph, PACKET_RESPONE_FREE_SPACE), *proxyIt);
	}

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
		ProcessFile_load file(m_fileGatherer->getFileInfo(proxy).m_path);

		unsigned int blocksCount = file.getBlocksCount();

		for (unsigned long i = 0; i < blocksCount; i++)
			m_networkManager->send(file.nextBlock(), sizeof(PacketData));
	}
	else
		addRollBack(proxy);
}

PacketHeader Server::packFileInfo(const FileGatherer::FileInfo * fi, short int flags)
{
	PacketHeader_FileInfo ph_fi;

	ph_fi.m_pathId = fi->m_pathId;
	ph_fi.m_action = (PacketHeader_FileInfo::ACTION)m_fileGatherer->getAction(flags);
	ph_fi.m_size   = file_size(path(fi->m_path));

	m_pathTransform->cutPath(fi->m_pathId, fi->m_path, ph_fi.m_path);

	return PacketHeader(PACKET_FILE_INFO, &ph_fi, sizeof(PacketHeader_FileInfo));
}

void Server::rollBack(const FileGatherer::FileInfoProxy * proxy)
{
}

void Server::addRollBack(FileGatherer::FileInfoProxy * proxy)
{
	m_proxyRollBackVector.push_back(proxy);
}

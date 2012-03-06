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
#include <utility>
#include "Client.h"
#include "ProcessFile_store.h"
#include "pack_unpack.h"

using namespace std;
using namespace boost::filesystem;

Client::Client(int argc, char ** argv) : AppInterface(argc, argv)
{
	string host = m_config->getHost();

	if (host == "\0")
		throw FSException("Host is not defined");

	ID_Path_pairList id_path_pairList = m_config->getPathList();
	m_pathTransform = new PathTransform(id_path_pairList);
	m_networkManager = new NetworkManager(host.c_str(), m_config->getPort(), m_config->getRecvTimeout(), m_config->getSendTimeout());

	getServer(host);

	fileTransfer();

	PacketHeader ph_final;
	ph_final.m_type = PACKET_FINISHED;
	m_networkManager->send(&ph_final, sizeof(PacketHeader));

	m_networkManager->closeConnection();
}

Client::~Client()
{
	cout << "Synchronization finished, quitting..." << endl;

	delete m_networkManager;
	delete m_pathTransform;
}

void Client::getServer(const string & host)
{
	if (!m_networkManager->connectToServer())
	{
		cout << "Waiting for server..." << endl;

		while (!m_networkManager->connectToServer())
			SDL_Delay(100);
	}

	cout << "Connection established with " << host << ':' << m_config->getPort() << endl;
}

void Client::fileTransfer()
{
	PacketHeader ph_objectsCount;
	m_networkManager->recv(&ph_objectsCount, sizeof(PacketHeader));

	size_t objectsCount = unpackFromHeader<size_t>(&ph_objectsCount, PACKET_OBJECT_COUNT);
	cout << "Recieving " << objectsCount << " object(s):" << endl;

	for (size_t i = 0; i < objectsCount; i++)
	{
		PacketHeader_FileInfo ph_fi;
		PacketHeader ph;

		bool canProceed = prepareFileTransfer(&ph, &ph_fi);
		m_networkManager->send(&ph, sizeof(PacketHeader));

		if (canProceed)
		{
			if (ph.m_type == PACKET_NEXT || ph.m_type == PACKET_SKIP)
				continue;
			else if (ph.m_type == PACKET_RESPONE_FREE_SPACE_A_NEW)
				handleNew(&ph_fi);
			else if (ph.m_type == PACKET_RESPONE_FREE_SPACE_A_CHANGE)
				handleChange(&ph_fi);
		}
	}
}

bool Client::prepareFileTransfer(PacketHeader * ph_result, PacketHeader_FileInfo * ph_fi_out)
{
	m_networkManager->recv(ph_result, sizeof(PacketHeader));
	bool canProceed = false;

	if (ph_result->m_type == PACKET_FILE_INFO)
	{
		*ph_fi_out = *(PacketHeader_FileInfo*)ph_result->m_buffer;

		switch (ph_fi_out->m_action)
		{
			case PacketHeader_FileInfo::A_ADD:
			{
				path p = m_pathTransform->getPath(ph_fi_out->m_pathId);

				if (m_pathTransform->checkPathAndLog(p, ph_fi_out->m_pathId))
				{
					space_info s = space(m_pathTransform->getPath(ph_fi_out->m_pathId));

					bool hasFreeSpace = (s.available - ph_fi_out->m_size > 0);
					*ph_result = packToHeader<bool>(&hasFreeSpace, PACKET_RESPONE_FREE_SPACE_A_NEW);

					if (hasFreeSpace)
						canProceed = true;
				}
				else
					ph_result->m_type = PACKET_SKIP;
			}
				break;

			case PacketHeader_FileInfo::A_CHANGE:
			{
				path p = m_pathTransform->getPath(ph_fi_out->m_pathId);
				path filePath = p / ph_fi_out->m_path;

				space_info s = space(p);
				bool hasFreeSpace;

				/*
				 * If the file doesn't exist we can't apply the changes to it
				 * so we'll just send a request for the whole file.
				 */
				if (!exists(filePath))
				{
					hasFreeSpace = (s.available - ph_fi_out->m_size > 0);
					*ph_result = packToHeader<bool>(&hasFreeSpace, PACKET_RESPONE_FREE_SPACE_A_NEW);
				}
				else
				{
					hasFreeSpace = (s.available - file_size(filePath) + ph_fi_out->m_size > 0);
					*ph_result = packToHeader<bool>(&hasFreeSpace, PACKET_RESPONE_FREE_SPACE_A_CHANGE);
				}

				if (hasFreeSpace)
					canProceed = true;
			}
				break;

			case PacketHeader_FileInfo::A_DELETE:
				deleteFile(m_pathTransform->glueCutPath(ph_fi_out->m_pathId, ph_fi_out->m_path));
				ph_result->m_type = PACKET_NEXT;
				break;
			default:
				ph_result->m_type = PACKET_NEXT;
				break;
		}
	}
	else
		ph_result->m_type = PACKET_NEXT;

	return canProceed;
}

void Client::deleteFile(path p)
{
	if (is_regular_file(p))
	{
		try
		{
			remove(p);
		}
		catch (filesystem_error & e)
		{
			LogManager::getInstancePtr()->log(e.what(), LogManager::L_NOTICE);
		}
	}
}

void Client::handleNew(const PacketHeader_FileInfo * ph_fi)
{
	path filePath = m_pathTransform->glueCutPath(ph_fi->m_pathId, ph_fi->m_path);
	path parentPath = filePath.parent_path();

	if (!exists(parentPath))
		create_directories(parentPath);

	ProcessFile_store file(filePath.c_str(), ph_fi->m_size);

	unsigned int blocksCount = ProcessFileInterface::getBlocksCount(ph_fi->m_size);

	for (unsigned int i = 0; i < blocksCount; i++)
	{
		PacketData data;

		m_networkManager->recv(&data, sizeof(PacketData));
		file.feedNextBlock(&data);
	}
}

void Client::handleChange(const PacketHeader_FileInfo * ph_fi)
{
	path filePath = m_pathTransform->glueCutPath(ph_fi->m_pathId, ph_fi->m_path);
	resize_file(filePath, ph_fi->m_size);

	ProcessFile_store file(filePath.c_str());

	while (!chunkSearchAndStore(&file))
		continue;
}

bool Client::chunkSearchAndStore(ProcessFile_store * file)
{
	PacketHeader ph;
	m_networkManager->recv(&ph, sizeof(PacketHeader));

	if (ph.m_type == PACKET_NEXT)
		return true;
	else if (ph.m_type == PACKET_CHUNK_INFO)
	{
		PacketHeader_ChunkInfo ph_ci = unpackFromHeader<PacketHeader_ChunkInfo>(&ph, PACKET_CHUNK_INFO);
		ProcessFileInterface::ChunkInfo ci = make_pair(ph_ci.m_chunkType, ph_ci.m_offset);

		// send chunk's hash to server
		hash_t hash = file->getHash(ci);
		ph = packToHeader<hash_t>(&hash, PACKET_CHUNK_HASH);
		m_networkManager->send(&ph, sizeof(PacketHeader));
	}
	else if (ph.m_type == PACKET_CHUNK)
	{
		offset_t offset = unpackFromHeader<offset_t>(&ph, PACKET_CHUNK);

		PacketData chunk;
		m_networkManager->recv(&chunk, sizeof(PacketData));

		file->feedBlock(offset, &chunk);
	}

	return false;
}

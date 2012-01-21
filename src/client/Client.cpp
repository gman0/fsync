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
#include "Client.h"
#include "ProcessFile_store.h"
#include "pack_unpack.h"

using namespace std;
using namespace boost::filesystem;

Client::Client(int argc, char ** argv) : AppInterface(argc, argv)
{
	string host = m_config->getHost();

	if (host == "\0")
		throw FSException("Host is not defined", __FILE__, __LINE__);

	ID_Path_pairList id_path_pairList = m_config->getPathList();
	m_pathTransform = new PathTransform(id_path_pairList);
	m_networkManager = new NetworkManager(host.c_str(), m_config->getPort());

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
	if (!m_networkManager->tryOpenSocket())
	{
		cout << "Waiting for server..." << endl;

		while (!m_networkManager->tryOpenSocket())
			SDL_Delay(100);
	}

	cout << "Connection established with " << host << ':' << m_config->getPort() << endl;
}

void Client::fileTransfer()
{
	PacketHeader ph_objectsCount;
	m_networkManager->recv(&ph_objectsCount, sizeof(PacketHeader));

	size_t objectsCount = unpackFromHeader<size_t>(&ph_objectsCount, PACKET_OBJECT_COUNT);
	cout << "Recieving " << objectsCount << " objects:" << endl;

	for (size_t i = 0; i < objectsCount; i++)
	{
		PacketHeader_FileInfo ph_fi;
		PacketHeader ph = prepareFileTransfer(&ph_fi);

		m_networkManager->send(&ph, sizeof(PacketHeader));

		if (ph.m_type == PACKET_NEXT || ph.m_type == PACKET_SKIP)
			continue;
		else if (ph.m_type == PACKET_RESPONE_FREE_SPACE)
			handleNew(&ph_fi);
	}
}

PacketHeader Client::prepareFileTransfer(PacketHeader_FileInfo * ph_fi)
{
	PacketHeader ph;
	m_networkManager->recv(&ph, sizeof(PacketHeader));

	if (ph.m_type == PACKET_FILE_INFO)
	{
		*ph_fi = *(PacketHeader_FileInfo*)ph.m_buffer;

		switch (ph_fi->m_action)
		{
			case PacketHeader_FileInfo::A_ADD:
			{
				path p = m_pathTransform->getPath(ph_fi->m_pathId);

				if (m_pathTransform->checkPathAndLog(p, ph_fi->m_pathId))
				{
					ph.m_type = PACKET_RESPONE_FREE_SPACE;

					space_info s = space(m_pathTransform->getPath(ph_fi->m_pathId));

					bool hasFreeSpace = (s.available - ph_fi->m_size > 0);
					ph = packToHeader<bool>(&hasFreeSpace, PACKET_RESPONE_FREE_SPACE);
				}
				else
					ph.m_type = PACKET_SKIP;
			}
				break;

			case PacketHeader_FileInfo::A_CHANGE:
				// TODO: implement this
				/*
				 * add free space check
				 * 
				 * (free_space - file_size) + changedFile_size > 0
				 */
				break;

			case PacketHeader_FileInfo::A_DELETE:
				deleteFile(m_pathTransform->glueCutPath(ph_fi->m_pathId, ph_fi->m_path));
				break;
			default:
				ph.m_type = PACKET_NEXT;
				break;
		}
	}
	else
		ph.m_type = PACKET_NEXT;

	return ph;
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
	path p = m_pathTransform->glueCutPath(ph_fi->m_pathId, ph_fi->m_path);
	ProcessFile_store file(p.c_str(), ph_fi->m_size);

	unsigned long blocksCount = ProcessFile::getBlocksCount(ph_fi->m_size);

	for (unsigned long i = 0; i < blocksCount; i++)
	{
		PacketData data;

		m_networkManager->recv(&data, sizeof(PacketData));
		file.feedNextBlock(&data);
	}
}

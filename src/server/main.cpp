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
#include <fstream>
#include "Config.h"
#include "FSException.h"
#include "LogManager.h"
#include "NetworkManager.h"
#include "Packet.h"
#include "PacketContainer.h"

#include "FileGatherer.h"

using namespace std;

int main(int argc, char * argv[])
{
	try
	{
		/*======== TESTING =========*/
/*
		const char * fileName = "test.rar";

		ifstream file(fileName, ifstream::in);

		PacketHeader_File ph_file;
		memcpy(ph_file.m_path, fileName, strlen(fileName));

		PacketContainer pc(1, PCKT_FILE, sizeof(ph_file), (unsigned char*)&ph_file, file);

*/
		/*======== TESTING =========*/

		LogManager::instance("fsync_server.log");

		Config config(argc, (const char**)argv, "server.conf");
		//unsigned int port = config.getPort();
		ID_Path_pairList pathList = config.getPathList();

		for (ID_Path_pairList::const_iterator i = pathList.begin(); i != pathList.end(); i++)
		{
			cout << i->first << ": " << i->second << endl;
		}

		cout << "===\n\n";

		//FileGatherer fg(&config);
/*
		NetworkManager networkManager((port) ? port : 2000); // FIXME: see Valgrind output
		networkManager.openSocket();

		while (1)
		{
			break;
			if (networkManager.acceptConnection())
				cout << "client connected\n";
			else
			{
				SDL_Delay(100);
				continue;
			}

			IPaddress * ip = networkManager.getClientAddress();

			if (ip)
				cout << "Host connected: " << SDLNet_Read32(&ip->host) << " " << SDLNet_Read16(&ip->port) << endl;

			networkManager.closeConnection();
		}
*/

		LogManager::getInstancePtr()->destroy();
	}
	catch (FSException & e)
	{
		cerr << "An exception has occured: " << e.what() << endl;
	}

	return 0;
}

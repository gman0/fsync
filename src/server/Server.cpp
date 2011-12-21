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
#include "Server.h"
#include "Packet.h"
#include "PacketContainer.h"
#include "DataChunker.h"

using namespace std;
using namespace boost::filesystem;

Server::Server()
{
	/*
	 * this wasn't tested on windows
	 * I will have to deal with it later though
	 */
	path homePath(getenv("HOME"));

	path fsyncPath(homePath / ".fsync");
	checkAndCreate(fsyncPath);

	path serverPath(fsyncPath / "server");
	checkAndCreate(serverPath);
}

Server::~Server()
{
}

void Server::checkAndCreate(const path & p)
{
	if (exists(p))
	{
		if (!is_directory(p))
		{
			string errMsg = p.c_str();
			errMsg += " is not a directory";
			throw FSException(errMsg, __FILE__, __LINE__);
		}
	}
	else
	{
		if (!create_director(p))
		{
			string errMsg = "Cannot create directory ";
			errMsg += p.c_str();
			throw FSException(errMsg, __FILE__, __LINE__);
		}
	}
}

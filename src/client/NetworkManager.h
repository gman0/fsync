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

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "NetworkManagerInterface.h"

class NetworkManager : public NetworkManagerInterface
{
	public:
		NetworkManager(const char * ip, int port) : NetworkManagerInterface(ip, port) {}

		bool connectToServer();
		void closeConnection();

		int send(const void * data, int len);
		int recv(void * data, int len);
};

#endif // NETWORK_MANAGER_H

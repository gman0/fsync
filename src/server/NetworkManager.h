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

#include "NetworkManagerInterface.h"

class NetworkManager : public NetworkManagerInterface
{
	private:
		int m_clientSocketDescriptor;
		sockaddr_storage m_clientAddrInfo;

	public:
		NetworkManager(int port, int recvTimeout = 0, int sendTimeout = 0);

		void listen();
		bool acceptConnection();
		void closeConnection();

		/*
		 * out should point to an array of chars of at least INET_ADDRSTRLEN
		 * for IPv4 address or INET6_ADDRSTRLEN for IPv6 address.
		 */
		void getClientAddress(char * out, socklen_t size);
		int getPort();

		int send(const void * data, int len);
		int recv(void * data, int len);
};

#endif // NETWORK_MANAGER_H

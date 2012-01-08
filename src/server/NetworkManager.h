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

#include <SDL/SDL_net.h>
#include "NetworkManagerInterface.h"

class NetworkManager : public NetworkManagerInterface
{
	private:
		TCPsocket m_clientSocketDescriptor;
		IPaddress * m_clientIP;

	public:
		NetworkManager(int port) : NetworkManagerInterface(0, port), m_clientSocketDescriptor(0), m_clientIP(0)
		{}

		bool acceptConnection();
		void closeConnection();
		void closeClientConnection();
		IPaddress * getClientAddress();

		/*
		 * out should point to an array of chars of at least 16 items
		 * (15 for the actual IPv4 address and the last one for \0)
		 */
		void getClientIPAddress(char * out);

		bool send(const void * data, int len) const;
		void recv(void * data, int len) const;
};

#endif // NETWORK_MANAGER_H

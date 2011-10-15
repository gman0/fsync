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

#ifndef NETWORK_MANAGER
#define NETWORK_MANAGER

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include "Packet.h"

class NetworkManager
{
	private:
		TCPsocket m_serverSocketDescriptor;
		TCPsocket m_clientSocketDescriptor;
		IPaddress m_serverIP;
		IPaddress * m_clientIP;

	public:
		NetworkManager(int port);
		~NetworkManager();

		void openSocket();
		bool acceptConnection();
		void closeConnection();
		IPaddress * getClientAddress();
		bool send(const Packet & pckt) const;
		void recieve(Packet & pckt) const;
};

#endif // NETWORK_MANAGER

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

#include <cstdio>
#include "NetworkManager.h"

bool NetworkManager::acceptConnection()
{
	m_clientSocketDescriptor = SDLNet_TCP_Accept(m_serverSocketDescriptor);
	return (m_clientSocketDescriptor) ? true : false;
}

void NetworkManager::closeConnection()
{
	if (m_clientSocketDescriptor)
		SDLNet_TCP_Close(m_clientSocketDescriptor);

	SDLNet_TCP_Close(m_serverSocketDescriptor);
}

void NetworkManager::closeClientConnection()
{
	if (m_clientSocketDescriptor)
	{
		SDLNet_TCP_Close(m_clientSocketDescriptor);
		m_clientSocketDescriptor = 0;
	}
}

IPaddress * NetworkManager::getClientAddress()
{
	m_clientIP = SDLNet_TCP_GetPeerAddress(m_clientSocketDescriptor);

	if (!m_clientIP)
	{
		string err = "SDLNet_TCP_GetPeerAddress: ";
		err += SDLNet_GetError();
		LogManager::getInstancePtr()->log(err, LogManager::L_WARNING);
	}

	return m_clientIP;
}

void NetworkManager::getClientIPAddress(char * out)
{
	if (m_clientIP)
	{
		uint32_t ipAddress = SDL_SwapBE32(m_clientIP->host);
		sprintf(out, "%d.%d.%d.%d", ipAddress >> 24, (ipAddress >> 16) & 0xff, (ipAddress >> 8) & 0xff, ipAddress & 0xff);
	}
}

bool NetworkManager::send(const void * data, int len) const
{
	return NetworkManagerInterface::send(m_clientSocketDescriptor, data, len);
}

void NetworkManager::recv(void * data, int len) const
{
	NetworkManagerInterface::recv(m_clientSocketDescriptor, data, len);
}

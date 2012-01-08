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

#include "NetworkManager.h"

bool NetworkManager::tryOpenSocket()
{
	return (m_serverSocketDescriptor = SDLNet_TCP_Open(&m_serverIP));
}

void NetworkManager::closeConnection()
{
	if (m_serverSocketDescriptor)
		SDLNet_TCP_Close(m_serverSocketDescriptor);
}

bool NetworkManager::send(const void * data, int len) const
{
	return NetworkManagerInterface::send(m_serverSocketDescriptor, data, len);
}

void NetworkManager::recv(void * data, int len) const
{
	NetworkManagerInterface::recv(m_serverSocketDescriptor, data, len);
}

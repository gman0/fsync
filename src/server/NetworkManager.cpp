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

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "NetworkManager.h"
#include "LogManager.h"
#include "FSException.h"

NetworkManager::NetworkManager(int port) :
	NetworkManagerInterface(0, port),
	m_clientSocketDescriptor(0)
{
	string errorMsg;

	// enable address reuse so we don't get "Address already in use." error
	int enableReuseAddr = 1;

	if (setsockopt(m_serverSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, &enableReuseAddr, sizeof(int)) == -1)
	{
		errorMsg = "Cannot set flags to socket: ";
		errorMsg += strerror(errno);
		LogManager::getInstancePtr()->log(errorMsg, LogManager::L_ERROR);
		throw FSException(errorMsg);
	}

	if (bind(m_serverSocketDescriptor, m_serverInfo->ai_addr, m_serverInfo->ai_addrlen) == -1)
	{
		close(m_serverSocketDescriptor);

		errorMsg = "Cannot bind to socket: ";
		errorMsg += strerror(errno);
		LogManager::getInstancePtr()->log(errorMsg, LogManager::L_ERROR);
		throw FSException(errorMsg);
	}
}

void NetworkManager::listen()
{
	if (::listen(m_serverSocketDescriptor, 1) == -1)
	{
		string errorMsg = "Cannot listen() on socket: ";
		errorMsg += strerror(errno);
		LogManager::getInstancePtr()->log(errorMsg, LogManager::L_ERROR);
		throw FSException(errorMsg);
	}
}

bool NetworkManager::acceptConnection()
{
	socklen_t cliLen = sizeof m_clientAddrInfo;
	m_clientSocketDescriptor = accept(m_serverSocketDescriptor, (sockaddr*)&m_clientAddrInfo, &cliLen);

	return (m_clientSocketDescriptor > 0) ? true : false;
}

void NetworkManager::closeConnection()
{
	close(m_clientSocketDescriptor);
	close(m_serverSocketDescriptor);
}

void NetworkManager::getClientAddress(char * out, socklen_t size)
{
	inet_ntop(m_clientAddrInfo.ss_family, getInAddr((sockaddr*)&m_clientAddrInfo), out, size);
}

int NetworkManager::getPort()
{
	return ntohs(((sockaddr_in*)&m_clientAddrInfo)->sin_port);
}

int NetworkManager::send(const void * data, int len)
{
	return NetworkManagerInterface::send(m_clientSocketDescriptor, data, len);
}

int NetworkManager::recv(void * data, int len)
{
	return NetworkManagerInterface::recv(m_clientSocketDescriptor, data, len);
}

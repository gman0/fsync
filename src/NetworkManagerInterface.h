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

#ifndef NETWORK_MANAGER_INTERFACE_H
#define NETWORK_MANAGER_INTERFACE_H

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "LogManager.h"
#include "FSException.h"

class NetworkManagerInterface
{
	protected:
		int m_serverSocketDescriptor;
		addrinfo m_hints;
		addrinfo * m_serverInfo;

	public:
		NetworkManagerInterface(const char * ip, int port) : m_serverSocketDescriptor(0), m_serverInfo(0)
		{
			memset(&m_hints, 0, sizeof(addrinfo));

			m_hints.ai_family = AF_INET;
			m_hints.ai_socktype = SOCK_STREAM;
			if (!ip) m_hints.ai_flags = AI_PASSIVE;

			char portStr[6];
			sprintf(portStr, "%d", port);
			portStr[5] = '\0';

			int rv = getaddrinfo(ip, portStr, &m_hints, &m_serverInfo);
			string errorMsg;

			if (rv != 0)
			{
				errorMsg = "Cannot get address info: ";
				errorMsg += gai_strerror(rv);
				LogManager::getInstancePtr()->log(errorMsg, LogManager::L_ERROR);
				throw FSException(errorMsg);
			}

			m_serverSocketDescriptor = socket(m_serverInfo->ai_family, m_serverInfo->ai_socktype, m_serverInfo->ai_protocol);

			if (m_serverSocketDescriptor < 0)
			{
				errorMsg = "Cannot open socket: ";
				errorMsg += strerror(errno);
				LogManager::getInstancePtr()->log(errorMsg, LogManager::L_ERROR);
				throw FSException(errorMsg);
			}
		}

		virtual ~NetworkManagerInterface()
		{
			freeaddrinfo(m_serverInfo);
		}

		virtual void closeConnection() = 0;

		int send(int socketDescriptor, const void * data, int len)
		{
			int total = 0;
			int bytesLeft = len;
			int n = 0;

			while (total < len)
			{
				n = ::send(socketDescriptor, (const unsigned char*)data + total, bytesLeft, 0);

				if (n == -1)
					break;

				total += n;
				bytesLeft -= n;
			}

			return total;
		}

		int recv(int socketDescriptor, void * data, int len)
		{
			int total = 0;
			int bytesLeft = len;
			int n = 0;

			while (total < len)
			{
				n  = ::recv(socketDescriptor, (unsigned char*)data + total, bytesLeft, 0);

				if (n == -1)
					break;

				total += n;
				bytesLeft -= n;
			}

			return total;
		}
	
	protected:

		void * getInAddr(sockaddr * sa)
		{
			if (sa->sa_family == AF_INET)
				return &(((sockaddr_in*)sa)->sin_addr);

			return &(((sockaddr_in6*)sa)->sin6_addr);
		}

};

#endif // NETWORK_MANAGER_INTERFACE_H

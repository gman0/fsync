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

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include "LogManager.h"
#include "FSException.h"

class NetworkManagerInterface
{
	protected:
		TCPsocket m_serverSocketDescriptor;
		IPaddress m_serverIP;

	public:
		NetworkManagerInterface(const char * ip, int port) : m_serverSocketDescriptor(0)
		{
			string errorMsg;

			if (SDL_Init(0) == -1)
			{
				errorMsg = "SDL_Init: ";
				errorMsg += SDL_GetError();
				LogManager::getInstancePtr()->log(errorMsg, LogManager::L_ERROR);
				throw FSException(errorMsg, __FILE__, __LINE__);
			}

			if (SDLNet_Init() == -1)
			{
				errorMsg = "SDLNet_Init: ";
				errorMsg += SDLNet_GetError();
				LogManager::getInstancePtr()->log(errorMsg, LogManager::L_ERROR);
				throw FSException(errorMsg, __FILE__, __LINE__);
			}

			if (SDLNet_ResolveHost(&m_serverIP, ip, port) < 0)
			{
				errorMsg = "SDLNet_ResolveHost: ";
				errorMsg += SDLNet_GetError();
				LogManager::getInstancePtr()->log(errorMsg, LogManager::L_ERROR);
				throw FSException(errorMsg, __FILE__, __LINE__);
			}
		}

		virtual ~NetworkManagerInterface()
		{
			SDLNet_Quit();
			SDL_Quit();
		}

		void openSocket()
		{
			m_serverSocketDescriptor = SDLNet_TCP_Open(&m_serverIP);

			if (!m_serverSocketDescriptor)
			{
				string err = "SDLNet_TCP_Open: ";
				err += SDLNet_GetError();
				LogManager::getInstancePtr()->log(err, LogManager::L_ERROR);
				throw FSException(err, __FILE__, __LINE__);
			}
		}

		virtual void closeConnection() = 0;

		bool send(TCPsocket socketDescriptor, const void * data, int len) const
		{
			int total = 0;
			int bytesLeft = len;
			int n;

			while (total < len)
			{
				n = SDLNet_TCP_Send(socketDescriptor, ((unsigned char*)data + total), bytesLeft);

				if (n == -1)
					return false;

				total += n;
				bytesLeft -= n;
			}

			return true;
		}

		void recv(TCPsocket socketDescriptor, void * data, int len) const
		{
			int total = 0;
			int bytesLeft = len;
			int n;

			while (total < len)
			{
				n = SDLNet_TCP_Recv(socketDescriptor, ((unsigned char*)data + total), bytesLeft);

				if (n == -1)
					break;

				total += n;
				bytesLeft -= n;
			}
		}
};

#endif // NETWORK_MANAGER_INTERFACE_H

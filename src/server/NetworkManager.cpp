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
#include "NetworkManager.h"
#include "LogManager.h"
#include "FSException.h"

NetworkManager::NetworkManager(int port)
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

	if (SDLNet_ResolveHost(&m_serverIP, NULL, port) < 0)
	{
		errorMsg = "SDLNet_ResloveHost: ";
		errorMsg += SDLNet_GetError();
		LogManager::getInstancePtr()->log(errorMsg, LogManager::L_ERROR);
		throw FSException(errorMsg, __FILE__, __LINE__);
	}
}

NetworkManager::~NetworkManager()
{
	SDLNet_Quit();
	SDL_Quit();
}

void NetworkManager::openSocket()
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

bool NetworkManager::acceptConnection()
{
	m_clientSocketDescriptor = SDLNet_TCP_Accept(m_serverSocketDescriptor);
	return (m_clientSocketDescriptor) ? true : false;
}

void NetworkManager::closeConnection()
{
	SDLNet_TCP_Close(m_clientSocketDescriptor);
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

bool NetworkManager::send(const Packet & pckt) const
{
	int len = sizeof pckt;

	if (SDLNet_TCP_Send(m_clientSocketDescriptor, pckt.serialize(), len) < len)
	{
		string err = "SDLNet_TCP_Send: ";
		err += SDLNet_GetError();
		LogManager::getInstancePtr()->log(err, LogManager::L_ERROR);
		throw FSException(err, __FILE__, __LINE__);
	}

	return true;
}

void NetworkManager::recieve(Packet & pckt) const
{
	SDLNet_TCP_Recv(m_clientSocketDescriptor, (void*)&pckt, sizeof pckt);
}

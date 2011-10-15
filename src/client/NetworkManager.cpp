#include <string>
#include "NetworkManager.h"
#include "LogManager.h"
#include "FSException.h"

NetworkManager::NetworkManager(const char * ip, int port)
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

void NetworkManager::closeConnection()
{
	if (m_serverSocketDescriptor)
		SDLNet_TCP_Close(m_serverSocketDescriptor);
}

bool NetworkManager::send(const Packet & pckt) const
{
	int len = sizeof pckt;

	if (SDLNet_TCP_Send(m_serverSocketDescriptor, pckt.serialize(), len) < len)
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
	SDLNet_TCP_Recv(m_serverSocketDescriptor, (void*)&pckt, sizeof pckt);
}

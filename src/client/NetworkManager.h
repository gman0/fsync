#ifndef NETWORK_MANAGER
#define NETWORK_MANAGER

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include "Packet.h"

class NetworkManager
{
	private:
		TCPsocket m_serverSocketDescriptor;
		IPaddress m_serverIP;

	public:
		NetworkManager(const char * ip, int port);
		~NetworkManager();

		void openSocket();
		void closeConnection();
		bool send(const Packet & pckt) const;
		void recieve(Packet & pckt) const;
};

#endif // NETWORK_MANAGER

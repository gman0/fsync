#include <iostream>
#include <cstring>
#include <fstream>
#include "Config.h"
#include "FSException.h"
#include "LogManager.h"
#include "NetworkManager.h"

using namespace std;

int main(int argc, char * argv[])
{
	int len;
	ifstream file;
	file.open("test.zip", ifstream::binary);

	file.seekg(0, ifstream::end);
	len = file.tellg();
	file.seekg(0, ifstream::beg);

	char buffer[len];
	file.read(buffer, len);

	Packet p;
	p.m_type = PCKT_FILE_CHUNK;
	memcpy(p.m_buffer, buffer, len);
	p.len = len;

	file.close();

	try
	{
		LogManager::instance("fsync_client.log");

		NetworkManager networkManager("localhost", 2000);
		networkManager.openSocket();

		networkManager.send(p);

		cin.get();

		networkManager.closeConnection();
	}
	catch (FSException & e)
	{
		cerr << "An exception has occured: " << e.what() << endl;
	}

	return 0;
}

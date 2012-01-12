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

#ifndef CLIENT_H
#define CLIENT_H

#include <boost/filesystem.hpp>
#include <string>
#include "AppInterface.h"
#include "NetworkManager.h"
#include "PathTransform.h"
#include "Packet.h"

class Client : public AppInterface
{
	private:
		NetworkManager * m_networkManager;
		PathTransform * m_pathTransform;

	public:
		Client(int argc, char ** argv);
		~Client();

	private:
		Client(const Client & c);
		const Client & operator=(const Client & c);

		void getServer(const std::string & host);
		void fileTransfer();

		PacketHeader prepareFileTransfer(PacketHeader_FileInfo * ph_fi);
		void deleteFile(boost::filesystem::path path);

		void handleNew(const PacketHeader_FileInfo * ph_fi);
};

#endif // CLIENT_H

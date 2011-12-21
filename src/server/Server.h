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

#ifndef SERVER_H
#define SERVER_H

#include <boost/filesystem.hpp>
#include "Config.h"
#include "LogManager.h"
#include "NetworkManager.h"
#include "FileGatherer.h"

class Server
{
	private:
		Config * m_config;
		FileGatherer * m_fileGatherer;
		NetworkManager * m_networkManager;
	private:
		Server(const Server & s);
		const Server & operator=(const Server & s);

	public:
		Server();
		~Server();
	
	private:
		void checkAndCreate(const boost::filesystem::path & p);
};

#endif // SERVER_H

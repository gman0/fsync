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

#ifndef APP_INTERFACE_H
#define APP_INTERFACE_H

#include <boost/filesystem.hpp>
#include <string>
#include <cassert>
#include <cstring>
#include "Config.h"
#include "LogManager.h"
#include "FSException.h"
#include "Packet.h"

using namespace std;
using namespace boost::filesystem;

class AppInterface
{
	protected:
		Config * m_config;

	public:
		AppInterface(int argc, char ** argv)
		{
			path fsyncPath = m_config->getFsyncHomePath();
			checkAndCreate(fsyncPath);

#ifdef SERVER_H
			LogManager::instance(fsyncPath / "fsync_server.log");
			m_config = new Config(argc, argv, fsyncPath / "server_config.conf", fsyncPath);
#else
			LogManager::instance(fsyncPath / "fsync_client.log");
			m_config = new Config(argc, argv, fsyncPath / "client_config.conf", fsyncPath);
#endif
		}

		virtual ~AppInterface()
		{
			delete m_config;
			LogManager::getInstancePtr()->destroy();
		}

	protected:
		void checkAndCreate(const boost::filesystem::path & p)
		{
			if (exists(p))
			{
				if (!is_directory(p))
				{
					string errMsg = p.c_str();
					errMsg += " is not a directory";
					throw FSException(errMsg, __FILE__, __LINE__);
				}
			}
			else
			{
				if (!create_directory(p))
				{
					string errMsg = "Cannot create directory ";
					errMsg += p.c_str();
					throw FSException(errMsg, __FILE__, __LINE__);
				}
			}
		}


	private:
		AppInterface(const AppInterface & ai);
		const AppInterface & operator=(const AppInterface & ai);
};

#endif // APP_INTERFACE_H

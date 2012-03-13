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
#include <getopt.h>
#include "Config.h"
#include "ConfigHelpers.h"
#include "LogManager.h"
#include "FSException.h"
#include "Packet.h"

using namespace std;
using namespace boost::filesystem;

#define DESCRIPTION "fsync is a file/folder synchronizer which transmits files over network."


#ifdef SERVER_H

#define PACKAGE "fsync-server (fsync)"
#define VERSION "1.0.0"
#define HELP \
	"\t-h  --help\t\t\tShow this message.\n" \
	"\t-v  --version\t\t\tShow version.\n" \
	"\t-p port  --port\t\t\tSet port.\n" \
	"\t-c config file  --config-file\tRead settings from user specified config file.\n" \
	"\t-d database file  --db-file\tRead file database from user specified file.\n" \
	"\t-a  --dont-save-db\t\tDon't save file database on exit.\n" \
	"\t-u  --update-db\t\t\tUpdate file database and quit.\n" \
	"\t-i  --ignore-db\t\t\tIgnore file database, mark all files as new.\n" \
	"\t-r  --ignore-rb\t\t\tIgnore rollbacks."

#define OPT "hvp:c:d:auir"
#define OPT_HELP "[-hvauir] [-p PORT] [-c PATH TO CONFIG FILE] [-d PATH TO FILE DATABASE]"
#define LOG_FILE "fsync_server.log"
#define CONF_FILE "server.conf"

#else
#define PACKAGE "fsync-client (fsync)"
#define VERSION "1.0.0"
#define HELP \
	"\t-h  --help\t\t\tShow this message.\n" \
	"\t-v  --version\t\t\tShow version.\n" \
	"\t-s host  --host\t\t\tSet host.\n" \
	"\t-p port  --port\t\t\tSet port.\n" \
	"\t-c config file  --config-file\tRead settings from user specified config file.\n"

#define OPT "hvs:p:"
#define OPT_HELP "[-hv] [-s HOST] [-p PORT] [-c PATH TO CONFIG FILE]"
#define LOG_FILE "fsync_client.log"
#define CONF_FILE "client.conf"

#endif


class AppInterface
{
	protected:
		Config * m_config;


	public:
		AppInterface(int argc, char ** argv)
		{
			path fsyncPath = getFsyncHomePath();
			checkAndCreate(fsyncPath);

			LogManager::instance(fsyncPath / LOG_FILE);
			m_config = new Config(argc, argv, fsyncPath / CONF_FILE, OPT, PACKAGE, VERSION, DESCRIPTION, HELP, OPT_HELP);

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

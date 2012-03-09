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

#include <iostream>
#include <cstdlib>
#include "ConfigOptions.h"
#include "Exit.h"

using namespace std;

ConfigOptions::ConfigOptions(int argc, char ** argv, const char * optStr, const char * package,
								const char * version, const char * description, const char * help,
								const char * helpOptions)
{
	int c;

	while ((c = getopt_long(argc, argv, optStr, g_options, 0)) != -1)
	{
		switch (c)
		{
			case 'h':
				printHelp(description, argv[0], helpOptions, help);
				throw Exit();
				break;

			case 'v':
				printVersion(package, version);
				throw Exit();
				break;

			case 's':
				setHost(optarg);
				break;

			case 'p':
				setPort(optarg);
				break;

			case 'c':
				setConfigFile(optarg);
				break;

			case 'd':
				setDbFile(optarg);
				break;

			case 'a':
				setDontSaveDb();
				break;

			case 'u':
				setUpdateDb();
				break;

			case 'i':
				setIgnoreDb();
				break;

			case 'r':
				setIgnoreRb();
				break;
		}
	}
}

void ConfigOptions::printHelp(const char * description, const char * progName, const char * helpOptions,
								const char * help)
{
	cout << description << "\n\n"
		 << "Usage: " << progName << ' ' << helpOptions << '\n'
		 << help << endl;
}

void ConfigOptions::printVersion(const char * package, const char * version)
{
	cout << package << ' ' << version << endl;
}

bool ConfigOptions::checkEntry(const string & key)
{
	return (m_optionsMap[key].empty()) ? false : true;
}

void ConfigOptions::setHost(const char * host)
{
	m_optionsMap["host"] = host;
}

void ConfigOptions::setPort(const char * port)
{
	m_optionsMap["port"] = port;
}

void ConfigOptions::setConfigFile(const char * path)
{
	m_optionsMap["config_file"] = path;
}

void ConfigOptions::setDbFile(const char * path)
{
	m_optionsMap["db_file"] = path;
}

void ConfigOptions::setDontSaveDb()
{
	m_optionsMap["dont_save_db"] = '1';
}

void ConfigOptions::setUpdateDb()
{
	m_optionsMap["update_db"] = '1';
}

void ConfigOptions::setIgnoreDb()
{
	m_optionsMap["ignore_db"] = '1';
}

void ConfigOptions::setIgnoreRb()
{
	m_optionsMap["ignore_rb"] = '1';
}

string & ConfigOptions::getHost()
{
	return m_optionsMap["host"];
}

unsigned int ConfigOptions::getPort()
{
	return atoi(m_optionsMap["port"].c_str());
}

string & ConfigOptions::getConfigFile()
{
	return m_optionsMap["config_file"];
}

string & ConfigOptions::getDbFile()
{
	return m_optionsMap["db_file"];
}

bool ConfigOptions::dontSaveDb()
{
	return checkEntry("dont_save_db");
}

bool ConfigOptions::updateDb()
{
	return checkEntry("update_db");
}

bool ConfigOptions::ignoreDb()
{
	return checkEntry("ignore_db");
}

bool ConfigOptions::ignoreRb()
{
	return checkEntry("ignore_rb");
}

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
#include <string>
#include "Config.h"

using namespace std;
using namespace boost::filesystem;

Config::Config(int argc, char ** argv, const char * configFilePath, const char * optStr, const char * package,
				const char * version, const char * description, const char * help, const char * helpOptions) :
	ConfigOptions(argc, argv, optStr, package, version, description, help, helpOptions),
	ConfigParser((getConfigFile().empty() ? configFilePath : getConfigFile()))
{}

Config::Config(int argc, char ** argv, const path & configFilePath, const char * optStr, const char * package,
				const char * version, const char * description, const char * help, const char * helpOptions) :
	ConfigOptions(argc, argv, optStr, package, version, description, help, helpOptions),
	ConfigParser((getConfigFile().empty() ? configFilePath : getConfigFile()))
{}


unsigned int Config::getPort()
{
	unsigned int port = ConfigOptions::getPort();
	port = (port) ? port : ConfigParser::getPort();
	return (port) ? port : 2000;
}

string Config::getHost()
{
	string host = ConfigOptions::getHost();
	return (checkKey(host)) ? host : ConfigParser::getHost();
}

bool Config::recursiveFileSearchEnabled()
{
	return ConfigParser::recursiveFileSearchEnabled();
}

bool Config::partialFileTransferEnabled()
{
	return ConfigParser::partialFileTransferEnabled();
}

ID_Path_pairList Config::getPathList()
{
	return ConfigParser::getPathList();
}

path Config::getFileDbPath()
{
	string path = ConfigOptions::getDbFile();
	return (checkKey(path)) ? path : ConfigParser::getFileDbPath();
}

path Config::getRollbackFilePath()
{
	return path(getFileDbPath().string() + ".rollback");  
}

int Config::getRecvTimeout()
{
	unsigned int timeout = ConfigParser::getRecvTimeout();
	return (timeout) ? timeout : 3;
}

int Config::getSendTimeout()
{
	unsigned int timeout = ConfigParser::getSendTimeout();
	return (timeout) ? timeout : 3;
}

bool Config::ignoreDb()
{
	return ConfigOptions::ignoreDb();
}

bool Config::ignoreRb()
{
	return ConfigOptions::ignoreRb();
}

bool Config::dontSaveDb()
{
	return ConfigOptions::dontSaveDb();
}

bool Config::updateDbAndQuit()
{
	return ConfigOptions::updateDb();
}

bool Config::storeChangeTimestamp()
{
	bool store = ConfigParser::storeChangeTimestamp();
	return (ConfigOptions::storeTimestamp()) ? true : store;
}

bool Config::storePermissions()
{
	bool store = ConfigParser::storePermissions();
	return (ConfigOptions::storePermissions()) ? true : store;
}

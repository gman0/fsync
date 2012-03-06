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

Config::Config(int argc, char ** argv, const char * configFilePath, const char * fsyncHomePath) :
	ConfigParser(configFilePath, fsyncHomePath)
{
}

Config::Config(int argc, char ** argv, const path & configFilePath, const path & fsyncHomePath) :
	ConfigParser(configFilePath, fsyncHomePath)
{
}

unsigned int Config::getPort()
{
	unsigned int port = ConfigParser::getPort();
	return (port) ? port : 2000;
}

string Config::getHost()
{
	return ConfigParser::getHost();
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
	// TODO: add hash-based db name
	return ConfigParser::getFileDbPath();
}

path Config::getFsyncHomePath()
{
	return path(getenv("HOME")) / ".fsync";
}

path Config::getRollbackFilePath()
{
	return path(getFileDbPath().string() + ".rollback");  
}

int Config::getRecvTimeout()
{
	return 2;
}

int Config::getSendTimeout()
{
	return 2;
}

bool Config::forceNoChangeCheck()
{
	return true;
}

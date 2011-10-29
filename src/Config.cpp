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
#include "Config.h"

using namespace std;
using namespace boost::filesystem;

Config::Config(int argc, const char * argv[], const char * configFileName) : ConfigParser(configFileName)
{
}

unsigned int Config::getPort()
{
	return ConfigParser::getPort();
}

bool Config::recursiveFileSearchEnabled()
{
	return ConfigParser::recursiveFileSearchEnabled();
}

ID_Path_pairList Config::getPathList()
{
	return ConfigParser::getPathList();
}

path Config::getFilesDbPath()
{
	return ConfigParser::getFilesDbPath();
}

bool Config::forceNoChangeCheck()
{
	return true;
}

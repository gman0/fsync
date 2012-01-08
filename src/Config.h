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

#ifndef CONFIG_H
#define CONFIG_H

#include <list>
#include <boost/filesystem.hpp>
#include <string>
#include "ConfigParser.h"

class Config : public ConfigParser
{
	public:
		Config(int argc, char * argv[], const char * configFilePath, const char * fsyncHomePath);
		Config(int argc, char * argv[], const boost::filesystem::path & configFilePath,
										const boost::filesystem::path & fsyncHomePath);

		unsigned int getPort();
		std::string getHost();
		bool recursiveFileSearchEnabled();
		bool partialFileTransferEnabled();
		ID_Path_pairList getPathList();
		boost::filesystem::path getFileDbPath();
		bool forceNoChangeCheck();
};

#endif // CONFIG_H


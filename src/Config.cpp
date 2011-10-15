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

#include "Config.h"

int Config::getPortNum() const
{
	return m_port;
}

int Config::getChunkSize() const
{
	return m_chunkSize;
}

bool Config::recursiveFileSearchEnabled() const
{
	return m_recursiveFileSearch;
}

const char * Config::getLogPath() const
{
	return m_logPath;
}

std::list<boost::filesystem::path> Config::getPathList() const
{
	return m_pathList;
}

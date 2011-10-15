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

class Config
{
	private:
		int m_port;
		std::list<boost::filesystem::path> m_pathList;
		const char * m_logPath;
		bool m_recursiveFileSearch;
		int m_chunkSize;
	
	public:
		Config(int port, std::list<boost::filesystem::path> & pathList, char * logPath = 0,
				bool recursiveFileSearch = false, int chunkSize = 0) :
			m_port(port),
			m_pathList(pathList),
			m_logPath(logPath),
			m_recursiveFileSearch(recursiveFileSearch),
			m_chunkSize(chunkSize)
		{}

		int getPortNum() const;
		int getChunkSize() const;
		bool recursiveFileSearchEnabled() const;
		const char * getLogPath() const;
		std::list<boost::filesystem::path> getPathList() const;

};

#endif // CONFIG_H


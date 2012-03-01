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

#ifndef ROLLBACK_H
#define ROLLBACK_H

#include <fstream>
#include <boost/filesystem.hpp>
#include "Config.h"
#include "FileGatherer.h"

class Rollback
{
	public:

		struct RollbackEntry
		{
			FileGatherer::FILE_INFO_FLAG m_action;
			FileGatherer::FileInfo m_fileInfo;
		};

	private:
		Config * m_config;
		FileGatherer * m_fileGatherer;

		std::fstream m_rollbackFile;
		boost::filesystem::path m_rollbackFilePath;

		FileGatherer::FIProxyPtrVector m_rollbackFIProxy;

	public:
		Rollback(Config * config, FileGatherer * fileGatherer);
		~Rollback();

		void commitRollbacks(const FileGatherer::FIProxyPtrVector & rollbackVec);
		void buildRollbackProxies();
		FileGatherer::FIProxyPtrVector & getRollbackProxyVector();

		/*
		 * Before using this method you have to build rollback proxies first.
		 */
		FileGatherer::FileInfo getFileInfo(const FileGatherer::FileInfoProxy * proxy);

		void cleanRollbackFile();
};

#endif // ROLLBACK_H

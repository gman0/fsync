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

#ifndef FILE_GATHERER_H
#define FILE_GATHERER_H

#include <boost/filesystem.hpp>
#include <cstring>
#include <stdint.h>
#include <vector>
#include <map>
#include <utility>
#include <boost/thread/mutex.hpp>
//#include "AlignedIteratorPtrContainer.cpp"
#include "Config.h"
#include "defs.h"

class PathTransform;

class FileGatherer
{
	public:
		// obsolete
		enum FI_ACTION_ID
		{
			A_NONE,
			A_ADD,
			A_CHANGE,
			A_DELETE
		};


		enum FILE_INFO_FLAG
		{
			F_SRC_DB =			0x0000,
			F_SRC_FS =			0x0001,

			F_ACTION_ADD =		0x0002,
			F_ACTION_CHANGE =	0x0004,
			F_ACTION_DELETE =	0x0008
		};


		struct FileInfo
		{
			PathId m_pathId;
			char m_path[PATH_LENGTH];
			time_t m_lastWrite;

			FI_ACTION_ID m_action; // obsolete
			short int m_flags;

			FileInfo() : m_action(A_NONE), m_flags(0) {}

			bool operator==(const FileInfo & fi)
			{
				if (
					m_pathId == fi.m_pathId &&
					strcmp(m_path, fi.m_path) == 0
					)
						return true;

				return false;
			}

			bool operator!=(const FileInfo & fi)
			{
				return !(*this == fi);
			}

			const FileInfo & operator=(const FileInfo & fi)
			{
				m_pathId = fi.m_pathId;
				strcpy(m_path, fi.m_path);
				m_lastWrite = fi.m_lastWrite;
				m_action = fi.m_action;

				return *this;
			}
		};


		// FileInfo vector
		typedef std::vector<FileInfo> FIvector;


	private:

		struct FileInfoProxy
		{
			uint32_t m_hash;
			FIvector::iterator m_object;
			FILE_INFO_FLAG m_source;
		};

		typedef std::vector<std::vector<FileInfoProxy*>*> HashTree;
		typedef std::vector<std::vector<FileInfoProxy*>*>* HashTreePtr;
		typedef std::vector<FileInfoProxy*> HashLeaf;

		Config * m_config;
		PathTransform * m_pathTransform;
		boost::filesystem::path m_filesDbPath;

		FIvector m_fileInfoVector;
		FIvector m_changedFileInfoVector;

		// 3-dimensional array of pointers to std::vector<std::vector<FileInfoProxy*>*>*
		HashTreePtr *** m_hashTree;

		boost::mutex m_mutex;

	public:
		FileGatherer(Config * config);
		~FileGatherer();

		void updateDb();
		const FIvector & getChangedFiles();

	private:
		inline size_t getOffset(size_t n) { return n * sizeof(FileInfo); }
		inline int offsetToIndex(size_t offset) { return offset / sizeof(FileInfo); }
		void createFileList(const ID_Path_pairList & path_pairList);
		FileInfo assembleFileInfo(const PathId id, const boost::filesystem::path & path);

		/*
		 * indices should point to the beginning of an array of N items where N is the number of dimensions of m_hashTree array.
		 * Notice that this method will work by default only for 3D array, therefore when changing the number of dimensions this
		 * and other methods using this one have to be manually customized.
		 * source is defined as F_SRC_DB (Flag: Source - database) for readFromDb method and F_SRC_FS (Flag: Source - file system)
		 * for createFileList method.
		 */
		FileInfoProxy assembleFileInfoProxy(const FileInfo * fi, FIvector::iterator it, FILE_INFO_FLAG source,
											short int * indices);
		inline void insertIntoHashTree(const FileInfoProxy & fiProxy, short int * indices);
		void listFiles(const ID_Path_pairList & path_pairList);
		void readFromDb();
		void writeToDb(FIvector & fileInfoVec) const;

		void thread_add(FIvector & dbContents);
		void thread_change_delete(FIvector & dbContents);
};

#endif // FILE_GATHERER_H

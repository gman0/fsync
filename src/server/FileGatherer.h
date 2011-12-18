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
#include "Config.h"
#include "defs.h"

class PathTransform;

class FileGatherer
{
	public:


		struct FileInfo
		{
			PathId m_pathId;
			char m_path[PATH_LENGTH];
			uint32_t m_hash;
			time_t m_lastWrite;

			FileInfo() : m_hash(0) {}

			FileInfo(const FileInfo & fi)
			{
				m_pathId = fi.m_pathId;
				strcpy(m_path, fi.m_path);
				m_hash = fi.m_hash;
				m_lastWrite = fi.m_lastWrite;
			}

			bool operator==(const FileInfo & fi)
			{
				if (
					m_pathId == fi.m_pathId &&
					m_hash == fi.m_hash
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
				m_hash = fi.m_hash;
				m_lastWrite = fi.m_lastWrite;

				return *this;
			}
		};


		// FileInfo vector
		typedef std::vector<FileInfo> FIvector;



		enum FILE_INFO_FLAG
		{
			F_NONE =			0x0000,


			/*
			 * source is defined as F_SRC_DB (Flag: Source - database) for readFromDb method and F_SRC_FS (Flag: Source - file system)
			 * for createFileList method.
			 */
			F_SRC_DB =			0x0001,
			F_SRC_FS =			0x0002,

			F_ACTION_ADD =		0x0004,
			F_ACTION_CHANGE =	0x0008,
			F_ACTION_DELETE =	0x0010
		};


		struct FileInfoProxy
		{
			typedef size_t src_fs_t;
			typedef std::streampos src_db_t;

			void * m_object;
			short int m_flags;

			FileInfoProxy() : m_object(0), m_flags(F_NONE)
			{}

			FileInfoProxy(void * object, FILE_INFO_FLAG source) : m_object(object), m_flags(source)
			{}
		};

		typedef std::vector<FileInfoProxy*> FIProxyPtrVector;


	private:

		typedef std::vector<std::vector<FileInfoProxy*>*> HashTree;
		typedef std::vector<std::vector<FileInfoProxy*>*>* HashTreePtr;
		typedef std::vector<FileInfoProxy*> HashLeaf;

		Config * m_config;
		PathTransform * m_pathTransform;
		boost::filesystem::path m_filesDbPath;

		FIvector m_fileInfoVector;

		// if this was an object, it would crash...dunno why
		std::ifstream * m_dbFileIStream;

		// 3-dimensional array of pointers to std::vector<std::vector<FileInfoProxy*>*>*
		HashTreePtr *** m_hashTree;

		boost::mutex m_mutex;

	public:
		FileGatherer(Config * config);
		~FileGatherer();

		void updateDb();
		FIProxyPtrVector getChanges();

		FileInfo getFileInfo(const FileInfoProxy * p);

		FILE_INFO_FLAG getSource(short int flags);

		bool isOn(short int flags, FILE_INFO_FLAG f);
		inline bool isOff(short int flags, FILE_INFO_FLAG f) { return !isOn(flags, f); }

		inline bool flaggedAdd(short int flags) { return isOn(flags, F_ACTION_ADD); }
		inline bool flaggedChange(short int flags) { return isOn(flags, F_ACTION_CHANGE); }
		inline bool flaggedDelete(short int flags) { return isOn(flags, F_ACTION_DELETE); }

	private:
		inline size_t getOffset(size_t n) { return n * sizeof(FileInfo); }
		inline int offsetToIndex(size_t offset) { return offset / sizeof(FileInfo); }
		
		template <typename Container>
		inline size_t getSize(Container * c) { boost::mutex::scoped_lock lock(m_mutex); return c->size(); }

		void createFileList(const ID_Path_pairList & path_pairList);


		/*
		 * indices should point to the beginning of an array of N items where N is the number of dimensions of m_hashTree array.
		 * Notice that this method will work by default only for 3D array, therefore when changing the number of dimensions this
		 * and other methods using this one have to be manually customized.
		 */
		void generateIndices(const uint32_t hash, short int * indices);

		FileInfo assembleFileInfo(const PathId id, const boost::filesystem::path & path);

		void insertIntoHashTree(FileInfoProxy * fiProxy, short int * indices);
		bool checkPairExistence(const short int * indices, const uint32_t hash, FileInfoProxy * proxy);
		void listFiles(const ID_Path_pairList & path_pairList);

		void readFromDb();
		void writeToDb(FIvector & fileInfoVec) const;

		void thread_add(FIvector & dbContents);
		void thread_change_delete(FIvector & dbContents);
};

#endif // FILE_GATHERER_H

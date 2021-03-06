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
#include "Config.h"
#include "defs.h"
#include "hash.h"

class PathTransform;
class Rollback;

class FileGatherer
{
	public:


		struct FileInfo
		{
			PathId m_pathId;
			char m_path[PATH_LENGTH];
			hash_t m_hash;
			time_t m_lastWrite;
			unsigned m_permissions;

			FileInfo() : m_hash(0), m_lastWrite(0), m_permissions(0) {}

			FileInfo(const FileInfo & fi)
			{
				m_pathId = fi.m_pathId;
				strcpy(m_path, fi.m_path);
				m_hash = fi.m_hash;
				m_lastWrite = fi.m_lastWrite;
				m_permissions = fi.m_permissions;
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
				m_permissions = fi.m_permissions;

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
			 *
			 * F_SRC_RB is for rollback (which uses it's own FileInfo container outside of FileGatherer, therefore, we need to add
			 * another source type so we can decide from which container we want to take FileInfo's).
			 */
			F_SRC_DB =			0x0001,
			F_SRC_FS =			0x0002,
			F_SRC_RB =			0x0004,

			F_ACTION_ADD =		0x0008,
			F_ACTION_CHANGE =	0x0010,
			F_ACTION_DELETE =	0x0020
		};


		struct FileInfoProxy
		{
			typedef size_t src_fs_t; // file system
			typedef std::streampos src_db_t; // file database
			typedef std::streampos src_rb_t; // rollback

			void * m_object;
			short int m_flags;

			FileInfoProxy() : m_object(0), m_flags(F_NONE)
			{}

			FileInfoProxy(void * object, FILE_INFO_FLAG source) : m_object(object), m_flags(source)
			{}

			FileInfoProxy(void * object, short int flags) : m_object(object), m_flags(flags)
			{}
		};

		typedef std::vector<FileInfoProxy*> FIProxyPtrVector;


	private:

		typedef std::vector<std::vector<FileInfoProxy*>*> HashTree;
		typedef std::vector<std::vector<FileInfoProxy*>*>* HashTreePtr;
		typedef std::vector<FileInfoProxy*> HashLeaf;

		Config * m_config;
		PathTransform * m_pathTransform;
		Rollback * m_rollbackSolver;
		boost::filesystem::path m_fileDbPath;

		FIvector m_fileInfoVector;

		// if this was an object, it would crash...dunno why
		std::ifstream * m_dbFileIStream;

		// 3-dimensional array of pointers to std::vector<std::vector<FileInfoProxy*>*>*
		HashTreePtr *** m_hashTree;


	public:
		FileGatherer(Config * config, PathTransform * pathTransform, ID_Path_pairList & id_path_pairList);
		~FileGatherer();

		void setRollbackSolver(Rollback * rollbackSolver);

		void updateDb();
		FIProxyPtrVector getChanges();

		FileInfo getFileInfo(const FileInfoProxy * proxy);

		static FILE_INFO_FLAG getSource(short int flags);
		static FILE_INFO_FLAG getAction(short int flags);

		static bool isOn(short int flags, FILE_INFO_FLAG f);
		static inline bool isOff(short int flags, FILE_INFO_FLAG f) { return !isOn(flags, f); }

		static inline bool flaggedAdd(short int flags) { return isOn(flags, F_ACTION_ADD); }
		static inline bool flaggedChange(short int flags) { return isOn(flags, F_ACTION_CHANGE); }
		static inline bool flaggedDelete(short int flags) { return isOn(flags, F_ACTION_DELETE); }

	private:
		inline size_t getOffset(size_t n) { return n * sizeof(FileInfo); }
		inline int offsetToIndex(size_t offset) { return offset / sizeof(FileInfo); }

		void createFileList(const ID_Path_pairList & path_pairList);


		/*
		 * indices should point to the beginning of an array of N items where N is the number of dimensions of m_hashTree array.
		 * Notice that this method will work by default only for 3D array, therefore when changing the number of dimensions this
		 * and other methods using this one have to be manually customized.
		 */
		void generateIndices(const uint32_t & hash, short int * indices);

		FileInfo assembleFileInfo(const PathId id, const boost::filesystem::path & path);

		void insertIntoHashTree(FileInfoProxy * fiProxy, short int * indices);
		bool checkPairExistence(const short int * indices, const uint32_t & hash, FileInfoProxy * proxy);
		void listFiles(const ID_Path_pairList & path_pairList, bool ignoreDb);

		void readFromDb();
		void writeToDb(FIvector & fileInfoVec) const;
};

#endif // FILE_GATHERER_H

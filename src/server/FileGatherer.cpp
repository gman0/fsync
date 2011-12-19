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

#include <string>
#include <fstream>
#include <boost/thread.hpp>
#include <cstdio>
#include <cstdlib>
#include "FileGatherer.h"
#include "PathTransform.h"
#include "hash.h"
#include "FSException.h"
#include "LogManager.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

FileGatherer::FileGatherer(Config * config) :
	m_config(config),
	m_filesDbPath(config->getFilesDbPath()),
	m_dbFileIStream(0)
{
	if (exists(m_filesDbPath))
	{
		if (!is_regular_file(m_filesDbPath))
		{
			string errMsg = "Path to files database is not a file \"";
			errMsg += m_filesDbPath.c_str();
			errMsg += '\"';
			LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
			throw FSException(errMsg, __FILE__, __LINE__);
		}
	}

	cout << "Creating file list..." << flush;

	m_dbFileIStream = new ifstream;

	ID_Path_pairList id_path_pairList = config->getPathList();
	m_pathTransform = new PathTransform(id_path_pairList);
	listFiles(id_path_pairList);

	cout << "done." << endl;


	cout << "Gathering changes..." << flush;
	FIProxyPtrVector changed = getChanges();
	cout << "done." << endl;
	
	for (FIProxyPtrVector::iterator it = changed.begin(); it != changed.end(); it++)
	{
		FileInfo fi = getFileInfo(*it);
		short int flags = (*it)->m_flags;
		cout << "path: " << fi.m_path << " flags: " << ((getSource(flags) == F_SRC_FS) ? "F_SRC_FS" : "F_SRC_DB")
			 << "; flaggedAdd " << flaggedAdd(flags) << "; flaggedDelete " << flaggedDelete(flags)
			 << "; flaggedChange " << flaggedChange(flags) << endl;
	}
}

FileGatherer::~FileGatherer()
{
	for (int i = 0; i < HASH_LENGTH; i++)
	{
		for (int j = 0; j < HASH_LENGTH; j++)
		{
			for (int k = 0; k < HASH_LENGTH; k++)
			{
				if (m_hashTree[i][j][k])
				{
					for (vector<vector<FileInfoProxy*>*>::iterator t_it = m_hashTree[i][j][k]->begin();
							t_it != m_hashTree[i][j][k]->end(); t_it++)
					{
						for (vector<FileInfoProxy*>::iterator l_it = (*t_it)->begin(); l_it != (*t_it)->end(); l_it++)
						{
							if (getSource((*l_it)->m_flags) == F_SRC_FS)
								delete (FIvector::iterator*)(*l_it)->m_object;
							else if (getSource((*l_it)->m_flags) == F_SRC_DB)
								delete (streampos*)(*l_it)->m_object;

							delete *l_it;
						}

						delete *t_it;
					}

					delete m_hashTree[i][j][k];
				}
			}

			delete [] m_hashTree[i][j];
		}

		delete [] m_hashTree[i];
	}

	delete [] m_hashTree;

	delete m_pathTransform;

	if (m_dbFileIStream->is_open())
		m_dbFileIStream->close();

	delete m_dbFileIStream;
}

void FileGatherer::createFileList(const ID_Path_pairList & path_pairList)
{
	size_t index = 0;

	for (ID_Path_pairList::const_iterator i = path_pairList.begin(); i != path_pairList.end(); i++)
	{
		path p;

		if (i->first >= 0)
			p = i->second;
		else
			continue;

		if (!is_directory(p))
		{
			string errMsg = "Path \"";
			errMsg += p.c_str();
			errMsg += "\" is not a directory, skipping";
			LogManager::getInstancePtr()->log(errMsg, LogManager::L_WARNING);
			continue;
		}

		if (m_config->recursiveFileSearchEnabled())
		{
			for (recursive_directory_iterator dirIt(p); dirIt != recursive_directory_iterator(); dirIt++)
			{
				path p = dirIt->path();

				if (m_pathTransform->isExcluded(p, i->first))
					continue;

				if (is_regular_file(p))
				{
					short int indices[3];
					FileInfo fi = assembleFileInfo(i->first, p);

					generateIndices(fi.m_hash, indices);

					FileInfoProxy::src_fs_t * indexPtr;
					FileInfoProxy * fiProxyPtr;


					{
						mutex::scoped_lock lock(m_mutex);
						m_fileInfoVector.push_back(fi);
					}

					{
						mutex::scoped_lock lock(m_mutex);
						indexPtr = new FileInfoProxy::src_fs_t(index++);
					}

					{
						mutex::scoped_lock lock(m_mutex);
						fiProxyPtr = new FileInfoProxy((void*)indexPtr, F_SRC_FS);
					}

					{
						mutex::scoped_lock lock(m_mutex);

						// compensation for partial hash tree
						if (m_hashTree[indices[0]][indices[1]][indices[2]] == 0)
						{
							m_hashTree[indices[0]][indices[1]][indices[2]] = new HashTree;
							HashLeaf * leafPtr = new HashLeaf;
							leafPtr->push_back(fiProxyPtr);
							m_hashTree[indices[0]][indices[1]][indices[2]]->push_back(leafPtr);

							continue;
						}
					}

					if (!checkPairExistence(indices, fi.m_hash, fiProxyPtr))
						insertIntoHashTree(fiProxyPtr, indices);
				}
			}
		}
		else
		{
			for (directory_iterator dirIt(p); dirIt != directory_iterator(); dirIt++)
			{
				path p = dirIt->path();

				if (m_pathTransform->isExcluded(p, i->first))
					continue;

				if (is_regular_file(p))
				{
					short int indices[3];
					FileInfo fi = assembleFileInfo(i->first, p);

					generateIndices(fi.m_hash, indices);

					FileInfoProxy::src_fs_t * indexPtr;
					FileInfoProxy * fiProxyPtr;

					{
						mutex::scoped_lock lock(m_mutex);
						m_fileInfoVector.push_back(fi);
					}

					{
						mutex::scoped_lock lock(m_mutex);
						indexPtr = new FileInfoProxy::src_fs_t(index++);
					}

					{
						mutex::scoped_lock lock(m_mutex);
						fiProxyPtr = new FileInfoProxy((void*)indexPtr, F_SRC_FS);
					}

					{
						mutex::scoped_lock lock(m_mutex);

						if (m_hashTree[indices[0]][indices[1]][indices[2]] == 0)
						{
							m_hashTree[indices[0]][indices[1]][indices[2]] = new HashTree;
							HashLeaf * leafPtr = new HashLeaf;
							leafPtr->push_back(fiProxyPtr);
							m_hashTree[indices[0]][indices[1]][indices[2]]->push_back(leafPtr);

							continue;
						}
					}

					if (!checkPairExistence(indices, fi.m_hash, fiProxyPtr))
						insertIntoHashTree(fiProxyPtr, indices);
				}
			}
		}
	}
}

void FileGatherer::updateDb()
{
	writeToDb(m_fileInfoVector);
}

void FileGatherer::listFiles(const ID_Path_pairList & path_pairList)
{
	m_hashTree = new HashTreePtr**[HASH_LENGTH];

	for (int i = 0; i < HASH_LENGTH; i++)
	{
		m_hashTree[i] = new HashTreePtr*[HASH_LENGTH];

		for (int j = 0; j < HASH_LENGTH; j++)
		{
			m_hashTree[i][j] = new HashTreePtr[HASH_LENGTH];
			memset(m_hashTree[i][j], 0, HASH_LENGTH * sizeof(HashTreePtr));
		}
	}


	/*
	 * The performance here with multi-threading is frankly really bad. It's actually so bad,
	 * that I decided to run it in single-thread till I figure out how to make it work.
	 */
	// thread thrd_fs(&FileGatherer::createFileList, this, path_pairList);
	// thread thrd_db(&FileGatherer::readFromDb, this);

	// thrd_fs.join();
	// thrd_db.join();

	createFileList(path_pairList);
	readFromDb();
}

FileGatherer::FIProxyPtrVector FileGatherer::getChanges()
{
	FIProxyPtrVector proxyPtrVector;

	for (int i = 0; i < HASH_LENGTH; i++)
	{
		for (int j = 0; j < HASH_LENGTH; j++)
		{
			for (int k = 0; k < HASH_LENGTH; k++)
			{
				if (m_hashTree[i][j][k])
				{
					for (HashTree::iterator t_it = m_hashTree[i][j][k]->begin(); t_it != m_hashTree[i][j][k]->end(); t_it++)
					{
						size_t size = (*t_it)->size();

						if (size == 1)
						{
							FileInfoProxy * proxyPtr = (*t_it)->front();
							FILE_INFO_FLAG source = getSource(proxyPtr->m_flags);

							if (source == F_SRC_FS)
							{
								proxyPtr->m_flags |= F_ACTION_ADD;
								proxyPtrVector.push_back(proxyPtr);
							}
							else if (source == F_SRC_DB)
							{
								proxyPtr->m_flags |= F_ACTION_DELETE;
								proxyPtrVector.push_back(proxyPtr);
							}
						}
						else if (size == 2)
						{
							FileInfoProxy * p1 = (*t_it)->front();
							FileInfoProxy * p2 = (*t_it)->back();

							if (getFileInfo(p1).m_lastWrite != getFileInfo(p2).m_lastWrite)
							{
								// doesn't matter wich proxy
								p1->m_flags |= F_ACTION_CHANGE;
								proxyPtrVector.push_back(p1);
							}

						}
					}
				}
			}
		}
	}

	return proxyPtrVector;
}

FileGatherer::FileInfo FileGatherer::assembleFileInfo(const PathId id, const path & p)
{
	FileInfo fi;
	fi.m_pathId = id;
	strcpy(fi.m_path, p.c_str());
	fi.m_hash = calculateHash((const unsigned char*)fi.m_path, strlen(fi.m_path));
	fi.m_lastWrite = last_write_time(p);

	return fi;
}

void FileGatherer::generateIndices(const uint32_t & hash, short int * indices)
{
	char digits[HASH_LENGTH + 1];
	char tmp[2];

	tmp[1] = '\0';

	sprintf(digits, "%u", hash);

	for (int i = 0; i < 3; i++)
	{
		tmp[0] = digits[i];
		indices[i] = atoi(tmp);
	}
}

bool FileGatherer::checkPairExistence(const short int * indices, const uint32_t & hash, FileGatherer::FileInfoProxy * proxy)
{
	HashTreePtr ht_ptr = m_hashTree[indices[0]][indices[1]][indices[2]];

	for (size_t i = 0; i < getSize<HashTree>(ht_ptr); i++)
	{
		HashLeaf * t_it = ht_ptr->at(i);

		size_t s = getSize<HashLeaf>(t_it);
		for (size_t j = 0; j < s; j++)
		{
			FileInfoProxy * l_proxy = t_it->at(j);

			/*
			 * It would be great if I could do something like this:
			 *
			 * if (getSource(proxy->m_flags) == getSource(l_proxy->m_flags))
			 * 		continue;
			 *
			 * The performance speed-up is tremendous: about 5x faster on my machine.
			 * It would skip lots of uneeded cycle loops because we don't need to check for hash equality
			 * if the source is the same.
			 *
			 * But it doesn't work always - when interating through a small number of files it works just perfect
			 * but with a lot of files (about 100k) it will give us false positives when calling getChanges()
			 * many false results with flags F_SRC_FS and F_ACTION_ADD set to true.
			 *
			 * So I'll implement this later (or maybe never if I actually find out that this piece of code was actually
			 * a bad idea).
			 */

			FileInfo l_fi = getFileInfo(l_proxy);

			if (hash == l_fi.m_hash)
			{
				{
					mutex::scoped_lock lock(m_mutex);
					t_it->push_back(proxy);
				}

				return true;
			}
		}
	}

	return false;
}

FileGatherer::FileInfo FileGatherer::getFileInfo(const FileGatherer::FileInfoProxy * proxy)
{
	FILE_INFO_FLAG source = getSource(proxy->m_flags);

	if (source == F_SRC_FS)
		return m_fileInfoVector.at(*(FileInfoProxy::src_fs_t*)proxy->m_object);
	else if (source == F_SRC_DB)
	{
		char buffer[sizeof(FileInfo)];

		{
			mutex::scoped_lock ioLock(m_ioMutex);

			m_dbFileIStream->seekg(*(FileInfoProxy::src_db_t*)proxy->m_object);
			m_dbFileIStream->readsome(buffer, sizeof(FileInfo));
		}

		FileInfo * fiPtr = (FileInfo*)buffer;

		return *fiPtr;
	}

	return FileInfo();
}

bool FileGatherer::isOn(short int flags, FileGatherer::FILE_INFO_FLAG f)
{
	return ((flags & f) == f);
}

FileGatherer::FILE_INFO_FLAG FileGatherer::getSource(short int flags)
{
	if (isOn(flags, F_SRC_FS))
		return F_SRC_FS;
	else if (isOn(flags, F_SRC_DB))
		return F_SRC_DB;
	else
		return F_NONE;
}

void FileGatherer::insertIntoHashTree(FileGatherer::FileInfoProxy * proxy, short int * indices)
{
	mutex::scoped_lock lock(m_mutex);

	HashLeaf * leafPtr = new HashLeaf;
	leafPtr->push_back(proxy);

	m_hashTree[indices[0]][indices[1]][indices[2]]->push_back(leafPtr);
}

void FileGatherer::readFromDb()
{
	if (!exists(m_filesDbPath))
	{
		string errMsg = "Files database file \"";
		errMsg += m_filesDbPath.string() + "\" not found";
		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg, __FILE__, __LINE__);
	}

	m_dbFileIStream->open(m_filesDbPath.c_str());

	if (!m_dbFileIStream->is_open())
	{
		string errMsg = "Cannot open files database file \"";
		errMsg += m_filesDbPath.string() + "\" for reading";
		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg, __FILE__, __LINE__);
	}

	int count = file_size(m_filesDbPath) / sizeof(FileInfo);
	char buffer[sizeof(FileInfo)];
	size_t offset = 0;

	for (int i = 0; i < count; i++)
	{
		offset = getOffset(i);

		{
			mutex::scoped_lock ioLock(m_ioMutex);

			m_dbFileIStream->seekg(offset);
			m_dbFileIStream->readsome(buffer, sizeof(FileInfo));
		}

		short int indices[3];

		FileInfo * fiPtr = (FileInfo*)buffer;
		FileInfo fi = *fiPtr;

		generateIndices(fi.m_hash, indices);

		FileInfoProxy::src_db_t * streamPosPtr;
		FileInfoProxy * fiProxyPtr;

		{
			mutex::scoped_lock lock(m_mutex);

			streamPosPtr = new FileInfoProxy::src_db_t(offset);
			fiProxyPtr = new FileInfoProxy((void*)streamPosPtr, F_SRC_DB);
		}

		{
			mutex::scoped_lock lock(m_mutex);

			if (!m_hashTree[indices[0]][indices[1]][indices[2]])
			{
				m_hashTree[indices[0]][indices[1]][indices[2]] = new HashTree;
				HashLeaf * leafPtr = new HashLeaf;
				leafPtr->push_back(fiProxyPtr);
				m_hashTree[indices[0]][indices[1]][indices[2]]->push_back(leafPtr);
				
				continue;
			}
		}

		if (!checkPairExistence(indices, fi.m_hash, fiProxyPtr))
			insertIntoHashTree(fiProxyPtr, indices);
	}
}

void FileGatherer::writeToDb(FileGatherer::FIvector & fileInfoVec) const
{
	ofstream fout(m_filesDbPath.c_str());

	if (!fout.is_open())
	{
		string errMsg = "Cannot open files database file \"";
		errMsg += m_filesDbPath.string() + "\" for writing";
		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg, __FILE__, __LINE__);
	}

	for (FIvector::const_iterator i = fileInfoVec.begin(); i != fileInfoVec.end(); i++)
		fout.write((char*)&(*i), sizeof(FileInfo));

	fout.close();
}

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
#include <cstdio>
#include <cstdlib>
#include "FileGatherer.h"
#include "PathTransform.h"
#include "Rollback.h"
#include "FSException.h"
#include "LogManager.h"

using namespace std;
using namespace boost::filesystem;

FileGatherer::FileGatherer(Config * config, PathTransform * pathTransform, ID_Path_pairList & id_path_pairList) :
	m_config(config),
	m_pathTransform(pathTransform),
	m_rollbackSolver(0),
	m_fileDbPath(config->getFileDbPath()),
	m_dbFileIStream(0)
{
	if (exists(m_fileDbPath))
	{
		if (!is_regular_file(m_fileDbPath))
		{
			string errMsg = "Path to file database is not a file \"";
			errMsg += m_fileDbPath.c_str();
			errMsg += '\"';
			LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
			throw FSException(errMsg, __FILE__, __LINE__);
		}
	}

	cout << "Creating file list..." << flush;

	m_dbFileIStream = new ifstream;
	listFiles(id_path_pairList);

	cout << "done." << endl;
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

	if (m_dbFileIStream->is_open())
		m_dbFileIStream->close();

	delete m_dbFileIStream;
}

void FileGatherer::setRollbackSolver(Rollback * rollbackSolver)
{
	m_rollbackSolver = rollbackSolver;
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

					m_fileInfoVector.push_back(fi);

					indexPtr = new FileInfoProxy::src_fs_t(index++);
					fiProxyPtr = new FileInfoProxy(indexPtr, F_SRC_FS);

					// compensation for partial hash tree
					if (!m_hashTree[indices[0]][indices[1]][indices[2]])
						m_hashTree[indices[0]][indices[1]][indices[2]] = new HashTree;

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

					m_fileInfoVector.push_back(fi);

					indexPtr = new FileInfoProxy::src_fs_t(index++);
					fiProxyPtr = new FileInfoProxy((void*)indexPtr, F_SRC_FS);

					if (!m_hashTree[indices[0]][indices[1]][indices[2]])
						m_hashTree[indices[0]][indices[1]][indices[2]] = new HashTree;

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

	createFileList(path_pairList);
	readFromDb();
}

FileGatherer::FIProxyPtrVector FileGatherer::getChanges()
{
	FIProxyPtrVector proxyPtrVector;
	int add, change, del;
	add = change = del = 0;

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
								add++;
							}
							else if (source == F_SRC_DB)
							{
								proxyPtr->m_flags |= F_ACTION_DELETE;
								proxyPtrVector.push_back(proxyPtr);
								del++;
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
								change++;
							}
						}
					}
				}
			}
		}
	}

	if (!proxyPtrVector.empty())
		cout << add << " file(s) marked as new, " << change << " as changed, " << del << " marked for deletion.";
	else
		cout << "no changes.";
	
	cout << endl;

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

void FileGatherer::generateIndices(const hash_t & hash, short int * indices)
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

bool FileGatherer::checkPairExistence(const short int * indices, const hash_t & hash, FileGatherer::FileInfoProxy * proxy)
{
	HashTreePtr ht_ptr = m_hashTree[indices[0]][indices[1]][indices[2]];

	size_t s = ht_ptr->size();
	for (size_t i = 0; i < s; i++)
	{
		HashLeaf * t_it = ht_ptr->at(i);
		FileInfoProxy * l_proxy = t_it->front();
		FileInfo l_fi = getFileInfo(l_proxy);

		if (hash == l_fi.m_hash)
		{
			t_it->push_back(proxy);
			return true;
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

		m_dbFileIStream->seekg(*(FileInfoProxy::src_db_t*)proxy->m_object);
		m_dbFileIStream->readsome(buffer, sizeof(FileInfo));

		FileInfo * fiPtr = (FileInfo*)buffer;

		return *fiPtr;
	}
	else if (source == F_SRC_RB)
	{
		assert(m_rollbackSolver);
		return m_rollbackSolver->getFileInfo(proxy);
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
	else if (isOn(flags, F_SRC_RB))
		return F_SRC_RB;
	else
		return F_NONE;
}

FileGatherer::FILE_INFO_FLAG FileGatherer::getAction(short int flags)
{
	if (isOn(flags, F_ACTION_ADD))
		return F_ACTION_ADD;
	else if (isOn(flags, F_ACTION_CHANGE))
		return F_ACTION_CHANGE;
	else if (isOn(flags, F_ACTION_DELETE))
		return F_ACTION_DELETE;
	else
		return F_NONE;
}

void FileGatherer::insertIntoHashTree(FileGatherer::FileInfoProxy * proxy, short int * indices)
{
	HashLeaf * leafPtr = new HashLeaf;
	leafPtr->push_back(proxy);

	m_hashTree[indices[0]][indices[1]][indices[2]]->push_back(leafPtr);
}

void FileGatherer::readFromDb()
{
	/*
	 * If database doesn't exist, we'll create it later.
	 */
	if (!exists(m_fileDbPath))
		return;

	m_dbFileIStream->open(m_fileDbPath.c_str());

	if (!m_dbFileIStream->is_open())
	{
		string errMsg = "Cannot open file database file \"";
		errMsg += m_fileDbPath.string() + "\" for reading";
		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg, __FILE__, __LINE__);
	}

	int count = file_size(m_fileDbPath) / sizeof(FileInfo);
	char buffer[sizeof(FileInfo)];
	size_t offset = 0;

	for (int i = 0; i < count; i++)
	{
		offset = getOffset(i);

		m_dbFileIStream->seekg(offset);
		m_dbFileIStream->readsome(buffer, sizeof(FileInfo));

		short int indices[3];

		FileInfo * fiPtr = (FileInfo*)buffer;
		FileInfo fi = *fiPtr;

		generateIndices(fi.m_hash, indices);

		FileInfoProxy::src_db_t * streamPosPtr;
		FileInfoProxy * fiProxyPtr;

		streamPosPtr = new FileInfoProxy::src_db_t(offset);
		fiProxyPtr = new FileInfoProxy((void*)streamPosPtr, F_SRC_DB);

		if (!m_hashTree[indices[0]][indices[1]][indices[2]])
		{
			m_hashTree[indices[0]][indices[1]][indices[2]] = new HashTree;
			insertIntoHashTree(fiProxyPtr, indices);

			continue;
		}

		if (!checkPairExistence(indices, fi.m_hash, fiProxyPtr))
			insertIntoHashTree(fiProxyPtr, indices);
	}
}

void FileGatherer::writeToDb(FileGatherer::FIvector & fileInfoVec) const
{
	ofstream fout(m_fileDbPath.c_str());

	if (!fout.is_open())
	{
		string errMsg = "Cannot open file database file \"";
		errMsg += m_fileDbPath.string() + "\" for writing";
		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg, __FILE__, __LINE__);
	}

	for (FIvector::const_iterator i = fileInfoVec.begin(); i != fileInfoVec.end(); i++)
		fout.write((char*)&(*i), sizeof(FileInfo));

	fout.close();
}

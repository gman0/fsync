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
	m_filesDbPath(config->getFilesDbPath())
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

	cout << "creating file list..." << endl;
	ID_Path_pairList id_path_pairList = config->getPathList();
	m_pathTransform = new PathTransform(id_path_pairList);
	listFiles(id_path_pairList);
	cout << "done\n";

	//updateDb();

	cout << "processing file list...";
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
							delete *l_it;
						}

						delete *t_it;
					}
				}

				delete m_hashTree[i][j][k];
			}

			delete [] m_hashTree[i][j];
		}

		delete [] m_hashTree[i];
	}

	delete [] m_hashTree;

	delete m_pathTransform;
}

void FileGatherer::createFileList(const ID_Path_pairList & path_pairList)
{
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
					FileInfo fi = assembleFileInfo(i->first, p);
					FIvector::iterator it;

					{
						mutex::scoped_lock lock(m_mutex);

						m_fileInfoVector.push_back(fi);
						it = m_fileInfoVector.end();
					}

					short int indices[3];
					FileInfoProxy fiProxy = assembleFileInfoProxy(&fi, it, F_SRC_FS, indices);

					{
						mutex::scoped_lock lock(m_mutex);

						// compensation for partial hash tree
						if (m_hashTree[indices[0]][indices[1]][indices[2]] == 0)
							m_hashTree[indices[0]][indices[1]][indices[2]] = new HashTree;
					}

					HashTreePtr ht_ptr = m_hashTree[indices[0]][indices[1]][indices[2]];

					size_t size = 0;

					{
						mutex::scoped_lock lock(m_mutex);
						size = ht_ptr->size();
					}

					if (size > 0)
					{
						for (HashTree::const_iterator t_it = ht_ptr->begin(); t_it != ht_ptr->end(); t_it++)
						{
							if (*t_it)
							{
								bool found = false;

								for (HashLeaf::const_iterator l_it = (*t_it)->begin(); l_it != (*t_it)->end(); l_it++)
								{
									if ((*l_it)->m_hash == fiProxy.m_hash)
									{
										found = true;
										(*t_it)->push_back(new FileInfoProxy(fiProxy));
										break;
									}
								}

								// FIXME: a bug here
								/*
								if (!found)
								{
									mutex::scoped_lock lock(m_mutex);
									insertIntoHashTree(fiProxy, indices);
								}
								*/
							}
						}
					}
					else
					{
						mutex::scoped_lock lock(m_mutex);
						insertIntoHashTree(fiProxy, indices);
					}
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
					FileInfo fi = assembleFileInfo(i->first, p);
					FIvector::iterator it;

					{
						mutex::scoped_lock lock(m_mutex);

						m_fileInfoVector.push_back(fi);
						it = m_fileInfoVector.end();
					}

					short int indices[3];
					FileInfoProxy fiProxy = assembleFileInfoProxy(&fi, it, F_SRC_FS, indices);

					{
						mutex::scoped_lock mutex(m_mutex);
						
						if (!m_hashTree[indices[0]][indices[1]][indices[2]])
							m_hashTree[indices[0]][indices[1]][indices[2]] = new HashTree;
					}

					HashTree * ht_ptr = m_hashTree[indices[0]][indices[1]][indices[2]];

					if (ht_ptr->size() > 0)
					{
						for (HashTree::iterator i = ht_ptr->begin(); i != ht_ptr->end(); i++)
						{
							if (*i == 0)
								continue;

							bool found = false;

							for (vector<FileInfoProxy*>::iterator j = (*i)->begin(); j != (*i)->end(); j++)
							{
								if ((*j)->m_hash == fiProxy.m_hash)
								{
									found = true;
									break;
								}
							}

							if (!found)
							{
								mutex::scoped_lock lock(m_mutex);
								insertIntoHashTree(fiProxy, indices);
							}
						}
					}
					else
					{
						mutex::scoped_lock lock(m_mutex);
						insertIntoHashTree(fiProxy, indices);
					}
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

	//thread thrd_fs(&FileGatherer::createFileList, this, path_pairList);
	//thread thrd_db(&FileGatherer::readFromDb, this);

	//thrd_fs.join();
	//thrd_db.join();
	//
	createFileList(path_pairList);
}

// FIXME: performance issues
const FileGatherer::FIvector & FileGatherer::getChangedFiles()
{
	cout << "~reading db...";

	return m_changedFileInfoVector;
}

FileGatherer::FileInfo FileGatherer::assembleFileInfo(const PathId id, const path & p)
{
	FileInfo fi;
	fi.m_pathId = id;
	strcpy(fi.m_path, p.c_str());
	fi.m_lastWrite = last_write_time(p);

	return fi;
}

FileGatherer::FileInfoProxy FileGatherer::assembleFileInfoProxy(const FileGatherer::FileInfo * fi,
																FileGatherer::FIvector::iterator it,
																FileGatherer::FILE_INFO_FLAG source, short int * indices)
{
	uint32_t hash = calculateHash((const unsigned char*)fi->m_path, strlen(fi->m_path));
	char digits[HASH_LENGTH];
	char tmp[2];

	tmp[1] = '\0';

	sprintf(digits, "%d", hash);

	tmp[0] = digits[0];
	indices[0] = atoi(tmp);

	tmp[0] = digits[1];
	indices[1] = atoi(tmp);

	tmp[0] = digits[2];
	indices[2] = atoi(tmp);

	FileInfoProxy fiProxy;
	fiProxy.m_hash = hash;
	fiProxy.m_object = it;
	fiProxy.m_source = source;

	return fiProxy;
}

/*
 * or here...
 * or some entirely other place
 */
void FileGatherer::insertIntoHashTree(const FileGatherer::FileInfoProxy & fi, short int * indices)
{
	vector<FileInfoProxy*> * leafPtr = new vector<FileInfoProxy*>;
	FileInfoProxy * proxy = new FileInfoProxy(fi);

	//cout << "~ allocation of " << typeid(leafPtr).name() << " at " << leafPtr << endl;

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

	ifstream fin(m_filesDbPath.c_str());

	if (!fin.is_open())
	{
		string errMsg = "Cannot open files database file \"";
		errMsg += m_filesDbPath.string() + "\" for reading";
		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg, __FILE__, __LINE__);
	}

	int count = file_size(m_filesDbPath) / sizeof(FileInfo);
	//FIvector fileInfoVector;
	char buffer[sizeof(FileInfo)];
	size_t offset = 0;

	for (int i = 0; i < count; i++)
	{
		offset = getOffset(i);
		fin.seekg(offset);
		fin.readsome(buffer, sizeof(FileInfo));

		FileInfo fi = *(FileInfo*)buffer;

		//fileInfoVector.push_back(fi);
		// !!!
		FIvector::iterator it;// = fileInfoVector.end();


		short int indices[3];
		FileInfoProxy fiProxy = assembleFileInfoProxy(&fi, it, F_SRC_FS, indices);

		{
			mutex::scoped_lock lock(m_mutex);

			if (!m_hashTree[indices[0]][indices[1]][indices[2]])
				m_hashTree[indices[0]][indices[1]][indices[2]] = new HashTree;
		}

		HashTreePtr ht_ptr = m_hashTree[indices[0]][indices[1]][indices[2]];

		size_t size = 0;

		{
			mutex::scoped_lock lock(m_mutex);
			size = ht_ptr->size();
		}

		if (size > 0)
		{
			for (HashTree::iterator i = ht_ptr->begin(); i != ht_ptr->end(); i++)
			{
				if (*i == 0)
					continue;

				bool found = false;

				for (vector<FileInfoProxy*>::iterator j = (*i)->begin(); j != (*i)->end(); j++)
				{
					if ((*j)->m_hash == fiProxy.m_hash)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					mutex::scoped_lock lock(m_mutex);
					insertIntoHashTree(fiProxy, indices);
				}
			}
		}
		else
		{
			mutex::scoped_lock lock(m_mutex);
			insertIntoHashTree(fiProxy, indices);
		}
	}

	fin.close();
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

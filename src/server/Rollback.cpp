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

#include <cstring>
#include <string>
#include "Rollback.h"
#include "LogManager.h"
#include "FSException.h"

using namespace std;
using namespace boost::filesystem;

Rollback::Rollback(Config * config, FileGatherer * fileGatherer) :
	m_config(config),
	m_fileGatherer(fileGatherer)
{
	m_rollbackFilePath = m_config->getRollbackFilePath();
}

Rollback::~Rollback()
{
	if (m_rollbackFile.is_open())
		m_rollbackFile.close();

	for (FileGatherer::FIProxyPtrVector::iterator proxyIt = m_rollbackFIProxy.begin();
		 proxyIt != m_rollbackFIProxy.end(); proxyIt++)
	{
		delete (FileGatherer::FileInfoProxy::src_rb_t*)(*proxyIt)->m_object;
		delete *proxyIt;
	}
}

void Rollback::commitRollbacks(const FileGatherer::FIProxyPtrVector & rollbackVec)
{
	m_rollbackFile.open(m_rollbackFilePath.c_str(), ios::out | ios::trunc);

	if (!m_rollbackFile.is_open())
	{
		string errMsg = "Cannot write to rollback file \"";
		errMsg += m_rollbackFilePath.string() + "\"";

		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg);
	}

	for (FileGatherer::FIProxyPtrVector::const_iterator proxyIt = rollbackVec.begin();
		 proxyIt != rollbackVec.end(); proxyIt++)
	{
		RollbackEntry entry;
		entry.m_action = m_fileGatherer->getAction((*proxyIt)->m_flags);
		entry.m_fileInfo = m_fileGatherer->getFileInfo(*proxyIt);

		m_rollbackFile.write((char*)&entry, sizeof(RollbackEntry));
	}

	m_rollbackFile.close();
}

void Rollback::buildRollbackProxies()
{
	m_rollbackFile.open(m_rollbackFilePath.c_str(), ios::in);

	if (!m_rollbackFile.is_open())
	{
		string errMsg = "Cannot read from rollback file \"";
		errMsg += m_rollbackFilePath.string() + "\"";

		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg);
	}

	size_t fileSize = file_size(m_rollbackFilePath);

	if ((fileSize % sizeof(RollbackEntry)) != 0)
	{
		string errMsg = "Rollback file \"";
		errMsg += m_rollbackFilePath.string() +
				  "\" is corrupted, skipping rollback and deleting rollback file";

		remove(m_rollbackFilePath);
		LogManager::getInstancePtr()->log(errMsg, LogManager::L_WARNING);

		return;
	}

	unsigned int entries = fileSize / sizeof(RollbackEntry);

	for (unsigned int i = 0; i < entries; i++)
	{
		FileGatherer::FILE_INFO_FLAG action;

		offset_t offset = m_rollbackFile.tellg();

		// we only need RollbackEntry::m_action for now
		m_rollbackFile.readsome((char*)&action, sizeof(FileGatherer::FILE_INFO_FLAG));
		m_rollbackFile.seekg(offset + sizeof(RollbackEntry));

		FileGatherer::FileInfoProxy::src_rb_t * offsetPtr =
			new FileGatherer::FileInfoProxy::src_rb_t(offset);

		FileGatherer::FileInfoProxy * proxyPtr = new FileGatherer::FileInfoProxy(offsetPtr,
														FileGatherer::F_SRC_RB | action);

		m_rollbackFIProxy.push_back(proxyPtr);
	}
}

FileGatherer::FileInfo Rollback::getFileInfo(const FileGatherer::FileInfoProxy * proxy)
{
	RollbackEntry entry;
	offset_t offset = *(FileGatherer::FileInfoProxy::src_rb_t*)proxy->m_object;

	m_rollbackFile.seekg(offset);
	m_rollbackFile.readsome((char*)&entry, sizeof(RollbackEntry));

	return entry.m_fileInfo;
}

FileGatherer::FIProxyPtrVector & Rollback::getRollbackProxyVector()
{
	return m_rollbackFIProxy;
}

void Rollback::cleanRollbackFile()
{
	remove(m_rollbackFilePath);
}

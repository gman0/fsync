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
#include <cassert>
#include <cmath>
#include "ProcessFile.h"
#include "LogManager.h"
#include "FSException.h"

using namespace std;

void ProcessFile::prepare()
{
	m_file.seekg(0, istream::end);
	m_dataLen = m_file.tellg();
	m_file.seekg(0);
	m_blocksCount = ceil(m_dataLen / (float)BLOCK_SIZE);

	setPreferredChunkType();
}

void ProcessFile::setPreferredChunkType()
{
	if (m_dataLen > (MEDIUM_CHUNK + (MEDIUM_CHUNK / 2)))
	{
		m_chunkInfo.first = LARGE_CHUNK;
		return;
	}
	else if (m_dataLen > 4 * CHUNK)
	{
		m_chunkInfo.first = MEDIUM_CHUNK;
		return;
	}
	else
		m_chunkInfo.first = CHUNK;
}

size_t ProcessFile::getSize() const
{
	return m_dataLen;
}

size_t ProcessFile::getBlockSize(const ProcessFile::offset_t offset)
{
	size_t len = BLOCK_SIZE;

	if (offset + len >= m_dataLen)
		len = m_dataLen - offset;
	
	return len;
}

unsigned int ProcessFile::getBlocksCount() const
{
	return m_blocksCount;
}

unsigned long ProcessFile::getBlocksCount(const size_t size)
{
	return ceil(size / (float)BLOCK_SIZE);
}

hash_t ProcessFile::getHash()
{
	unsigned char buf[m_chunkInfo.first];

	m_file.seekg(getChunkOffset());
	m_file.readsome((char*)buf, m_chunkInfo.first);

	return calculateHash(buf, m_chunkInfo.first);
}

ProcessFile::offset_t ProcessFile::getChunkOffset(const ProcessFile::ChunkInfo & cInfo) const
{
	size_t offset = cInfo.first * cInfo.second;

	assert(offset < m_dataLen);

	return offset;
}

ProcessFile::offset_t ProcessFile::getChunkOffset() const
{
	return getChunkOffset(m_chunkInfo);
}

void ProcessFile::normalizeChunkOffset()
{
	if (m_chunkInfo.first == CHUNK)
		return;

	m_chunkInfo.first = CHUNK;
	m_chunkInfo.second = getChunkOffset() / CHUNK;
}

void ProcessFile::normalizeChunkOffset(ProcessFile::ChunkInfo & cInfo)
{
	cInfo = m_chunkInfo;
	normalizeChunkOffset();
}

size_t ProcessFile::getOffsetRange(const ProcessFile::ChunkInfo & start, const ProcessFile::ChunkInfo & end)
{
	return 0;
}

ProcessFile::offset_t ProcessFile::getGOffset() const
{
	return m_file.tellg();
}

ProcessFile::offset_t ProcessFile::getPOffset() const
{
	return m_file.tellp();
}

void ProcessFile::setZoom(const ProcessFile::CHUNK_TYPE chunkType)
{
	m_chunkInfo.second = chunkType;
}

ProcessFile::CHUNK_TYPE ProcessFile::getZoom() const
{
	return m_chunkInfo.first;
}

bool ProcessFile::zoomIn()
{
	if (m_chunkInfo.first == CHUNK)
		return false;

	switch (m_chunkInfo.first)
	{
		case LARGE_CHUNK:
			m_chunkInfo.first = MEDIUM_CHUNK;
			break;
		case MEDIUM_CHUNK:
			m_chunkInfo.first = CHUNK;
			break;
		default:
			m_chunkInfo.first = CHUNK;
	}

	return true;
}

bool ProcessFile::zoomOut()
{
	if (m_chunkInfo.first == LARGE_CHUNK)
		return false;

	switch (m_chunkInfo.first)
	{
		case CHUNK:
			m_chunkInfo.first = MEDIUM_CHUNK;
			break;
		case MEDIUM_CHUNK:
			m_chunkInfo.first = LARGE_CHUNK;
			break;
		default:
			m_chunkInfo.first = LARGE_CHUNK;
	}

	return true;
}

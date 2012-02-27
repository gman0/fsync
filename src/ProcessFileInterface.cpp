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

void ProcessFileInterface::prepare()
{
	m_file.seekg(0, istream::end);
	m_dataLen = m_file.tellg();
	m_file.seekg(0);
	m_blocksCount = ceil(m_dataLen / (float)BLOCK_SIZE);

	setPreferredChunkType();
}

void ProcessFileInterface::setPreferredChunkType()
{
	if (m_dataLen > (MEDIUM_CHUNK + (MEDIUM_CHUNK / 2)))
		m_chunkType = LARGE_CHUNK;
	else if (m_dataLen > 4 * CHUNK)
		m_chunkType = MEDIUM_CHUNK;
	else
		m_chunkType = CHUNK;
}

size_t ProcessFileInterface::getSize() const
{
	return m_dataLen;
}

size_t ProcessFileInterface::getBlockSize(const offset_t offset) const
{
	size_t len = BLOCK_SIZE;

	if (offset + len >= m_dataLen)
		len = m_dataLen - offset;

	return len;
}

unsigned int ProcessFileInterface::getBlocksCount() const
{
	return m_blocksCount;
}

unsigned int ProcessFileInterface::getBlocksCount(const size_t size)
{
	return ceil(size / (float)BLOCK_SIZE);
}

ProcessFileInterface::ChunkInfo ProcessFileInterface::getCurrentChunkInfo()
{
	return make_pair(m_chunkType, getOffset());
}

offset_t ProcessFileInterface::getOffsetRange(const ProcessFileInterface::ChunkInfo & ci)
{
	offset_t range = ci.first + ci.second;

	if (range > m_dataLen)
		range = m_dataLen;
	
	return range;
}

hash_t ProcessFileInterface::getHash(const ProcessFileInterface::ChunkInfo & ci)
{
	assert(ci.second <= m_dataLen);

	unsigned char * buf = new unsigned char[ci.first];
	int len;

	size_t spaceLeft = ci.first + ci.second;

	if (spaceLeft >= m_dataLen)
		len = m_dataLen - ci.second;
	else
		len = ci.first;

	m_file.seekg(ci.second);
	m_file.readsome((char*)buf, len);
	m_file.seekg(ci.second); // go back where we were before

	hash_t hash = calculateHash(buf, len);

	delete [] buf;

	return hash;
}

void ProcessFileInterface::setZoom(const ProcessFileInterface::CHUNK_TYPE chunkType)
{
	m_chunkType = chunkType;
}

ProcessFileInterface::CHUNK_TYPE ProcessFileInterface::getZoom() const
{
	return m_chunkType;
}

bool ProcessFileInterface::zoomIn()
{
	if (m_chunkType == CHUNK)
		return false;

	switch (m_chunkType)
	{
		case LARGE_CHUNK:
			m_chunkType = MEDIUM_CHUNK;
			break;
		case MEDIUM_CHUNK:
			m_chunkType = CHUNK;
			break;
		default:
			m_chunkType = CHUNK;
	}

	return true;
}

bool ProcessFileInterface::zoomOut()
{
	if (m_chunkType == LARGE_CHUNK)
		return false;

	switch (m_chunkType)
	{
		case CHUNK:
			m_chunkType = MEDIUM_CHUNK;
			break;
		case MEDIUM_CHUNK:
			m_chunkType = LARGE_CHUNK;
			break;
		default:
			m_chunkType = LARGE_CHUNK;
	}

	return true;
}

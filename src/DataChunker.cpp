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
#include <SDL/SDL.h>
#include "DataChunker.h"
#include "LogManager.h"
#include "FSException.h"
#include "hash.h"

using namespace std;

DataChunker::DataChunker(istream & data, size_t dataLen) : m_data(data)
{
	if (dataLen == 0)
	{
		m_data.seekg(0, istream::end);
		m_dataLen = m_data.tellg();
		m_data.seekg(0);
	}
	else
		m_dataLen = dataLen;
}

void DataChunker::getChunk(DataChunker::CHUNK_TYPE_ID chunkType, int chunkId, unsigned char * s) const
{
	getData(chunkType, chunkId, s);
}

Uint32 DataChunker::getChunkHash(DataChunker::CHUNK_TYPE_ID chunkType, int chunkId, const unsigned char * s) const
{
	if (s)
		return calculateHash(s, getDataLen(chunkType, getOffset(chunkType, chunkId)));
	else
	{
		unsigned char data[chunkType];
		size_t len = getData(chunkType, chunkId, data);
	
		return calculateHash(data, len);
	}
}

size_t DataChunker::getOffset(DataChunker::CHUNK_TYPE_ID chunkType, int chunkId) const
{
	size_t offset = CHUNK_SIZE * chunkId;
	
	if (offset > m_dataLen)
	{
		const char * errMsg = "DataChunker::getOffset(int) ran of its boundaries. This shouldn't have happend and is probably a bug. Please see README for bug-reporting information and file a bug including a brief description of what you were doing when this happened and fsync's log file. I'm sorry for inconvinience.";

		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg, __FILE__, __LINE__);
	}

	return offset;
}

size_t DataChunker::getDataLen(DataChunker::CHUNK_TYPE_ID chunkSize, size_t offset) const
{
	size_t len = chunkSize;

	if (offset + len > m_dataLen)
		len = m_dataLen - offset;

	return len;
}

size_t DataChunker::getData(DataChunker::CHUNK_TYPE_ID chunkType, int chunkId, unsigned char * s) const
{
	size_t offset = getOffset(chunkType, chunkId);
	int lenght = getDataLen(chunkType, offset);
	m_data.seekg(offset);
	m_data.readsome((char*)s, lenght);

	return lenght;
}

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

#ifndef DATA_CHUNKER_H
#define DATA_CHUNKER_H

#include <istream>
#include <utility>
#include <SDL/SDL.h>

#define CHUNK_SIZE 2048

#ifndef HASH_LENGTH
#define HASH_LENGTH 16
#endif

class DataChunker
{
	public:
		enum CHUNK_TYPE_ID
		{
			LARGE_CHUNK  = 5242880, // 5MB
			MEDIUM_CHUNK = 2097152, // 2MB
			CHUNK		 =  262144  // 256KB
		};
	
	private:
		std::istream & m_data;
		size_t m_dataLen;
	
	public:
		DataChunker(std::istream & data, size_t dataLen = 0);

		/*
		 * s should point to the beginning of an unsigned char array
		 * which should have size of at least chunkType
		 */
		void getChunk(CHUNK_TYPE_ID chunkType, int chunkId, unsigned char * s) const;
		Uint32 getChunkHash(CHUNK_TYPE_ID chunkType, int chunkId, const unsigned char * s = 0) const;

	private:
		size_t getOffset(CHUNK_TYPE_ID chunkType, int chunkId) const;
		size_t getDataLen(CHUNK_TYPE_ID chunkSize, size_t offset) const;
		size_t getData(CHUNK_TYPE_ID chunkType, int chunkId, unsigned char * s) const;
};

#endif // DATA_CHUNKER_H

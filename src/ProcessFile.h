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

#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

#include <fstream>
#include <stdint.h>
#include <utility>
#include "defs.h"
#include "hash.h"

class ProcessFile
{
	public:
		enum CHUNK_TYPE
		{
			CHUNK_NONE	 = 0,

			LARGE_CHUNK  = 4194304, // 4MB
			MEDIUM_CHUNK = 2097152, // 2MB
			CHUNK		 =  262144  // 256KB
		};

		// CHUNK_TYPE - chunkId pair
		typedef std::pair<CHUNK_TYPE, int> ChunkInfo;

	protected:
		std::fstream & m_file;
		size_t m_dataLen;
		unsigned int m_blocksCount;
		ChunkInfo m_chunkInfo;

	public:
		ProcessFile(std::fstream & file) : m_file(file), m_dataLen(0), m_blocksCount(0),
											m_chunkInfo(CHUNK_NONE, 0)
		{}

		void prepare();

		size_t getSize() const;
		size_t getBlockSize(offset_t offset) const;
		unsigned int getBlocksCount() const;

		/*
		 * Calculates the number of blocks for size
		 */
		static unsigned long getBlocksCount(const size_t size);

		/*
		 * Sets the CHUNK_TYPE in m_chunkInfo to CHUNK and calculates it's chunk id
		 */
		void normalizeChunkOffset();

		/*
		 * Normalizes the offset and saves the previous state into cInfo
		 */
		void normalizeChunkOffset(ChunkInfo & cInfo);

		static offset_t getOffsetRange(const ChunkInfo & start, const ChunkInfo & end);
		static offset_t getOffset(const ChunkInfo & ci);

		ChunkInfo getCurrentChunkInfo() const;

		/*
		 * Gets GET offset of m_file (tellg())
		 */
		offset_t getGOffset() const;

		/*
		 * Gets PUT offset of m_file (tellp())
		 */
		offset_t getPOffset() const;

		void setGOffset(offset_t offset);
		void setPOffset(offset_t offset);

		void setZoom(const CHUNK_TYPE chunkType);
		CHUNK_TYPE getZoom() const;
		bool zoomIn();
		bool zoomOut();

		hash_t getHash();
		hash_t getHash(int chunkId);


	protected:
		/*
		 * Get offset for cInfo
		 */
		offset_t getChunkOffset(const ChunkInfo & cInfo) const;

		/*
		 * Get current offset (from m_chunkInfo)
		 */
		offset_t getChunkOffset() const;

		void setPreferredChunkType();

};


/*
 * When sending data trough PacketData, no other chunk types will be accepted
 * simply because they won't fit into PacketData's buffer.
 */
// #define BUFFER_TRESHOLD ProcessFile::CHUNK


#endif // PROCESS_FILE_H

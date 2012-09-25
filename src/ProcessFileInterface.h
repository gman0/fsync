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

#ifndef PROCESS_FILE_INTERFACE_H
#define PROCESS_FILE_INTERFACE_H

#include <fstream>
#include <stdint.h>
#include <utility>
#include "defs.h"
#include "hash.h"

class ProcessFileInterface
{
	public:
		enum CHUNK_TYPE
		{
			CHUNK_NONE	 = 0,

			LARGE_CHUNK  = 4194304, // 4MB
			MEDIUM_CHUNK = 2097152, // 2MB
			CHUNK		 =  262144  // 256KB
		};

		typedef std::pair<CHUNK_TYPE, offset_t> ChunkInfo;

	protected:
		std::fstream & m_file;
		size_t m_dataLen;
		unsigned int m_blocksCount;
		unsigned char * m_buffer;
		CHUNK_TYPE m_chunkType;

	public:
		ProcessFileInterface(std::fstream & file, unsigned char * buf) :
			m_file(file), m_dataLen(0), m_blocksCount(0), m_buffer(buf), m_chunkType(CHUNK_NONE)
		{}

		void prepare();

		size_t getSize() const;
		size_t getBlockSize(offset_t offset) const;
		unsigned int getBlocksCount() const;

		/*
		 * Calculates the number of blocks for size
		 */
		static unsigned int getBlocksCount(const size_t size);
	
		offset_t getOffsetRange(const ChunkInfo & ci);

		ChunkInfo getCurrentChunkInfo();

		virtual void setOffset(offset_t offset) = 0;
		virtual offset_t getOffset() = 0;

		void setZoom(const CHUNK_TYPE chunkType);
		CHUNK_TYPE getZoom() const;
		bool zoomIn();
		bool zoomOut();

		hash_t getHash(const ChunkInfo & ci);


	protected:

		void setPreferredChunkType();

};


#endif // PROCESS_FILE_INTERFACE_H

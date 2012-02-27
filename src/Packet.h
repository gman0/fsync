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

#ifndef PACKET_H
#define PACKET_H

#include <SDL/SDL.h>
#include <boost/filesystem.hpp>
#include <cstring>
#include "defs.h"
#include "hash.h"
#include "ProcessFileInterface.h"

enum PACKET_TYPE_ID
{
	PACKET_NONE = 0,

	PACKET_OBJECT_COUNT,

	PACKET_FILE_INFO,
	PACKET_FILE,

	PACKET_CHUNK_HASH,
	PACKET_CHUNK_INFO,
	PACKET_CHUNK,

	PACKET_REQUEST_FREE_SPACE,

	/*
	 * Respone for server's request for free space.
	 * Also, we want the whole file.
	 */
	PACKET_RESPONE_FREE_SPACE_A_NEW,

	/*
	 * Respone for server's request for free space.
	 * We want the server to send us PACKET_CHUNK_INFOs
	 */
	PACKET_RESPONE_FREE_SPACE_A_CHANGE,

	PACKET_NEXT,    // the task is done, continue
	PACKET_SKIP,    // there was an error, but continue anyway
	PACKET_FINISHED // everything went better than expected :)
};

struct PacketHeader
{
	PACKET_TYPE_ID m_type;
	unsigned char m_buffer[HEADER_BUFF_SIZE];

	PacketHeader(const PACKET_TYPE_ID type, const void * data, size_t len)
	{
		m_type = type;
		memcpy(m_buffer, data, len);
	}

	PacketHeader() : m_type(PACKET_NONE) {}

	PacketHeader(const PacketHeader & ph)
	{
		m_type = ph.m_type;
		memcpy(m_buffer, ph.m_buffer, sizeof(PacketHeader));
	}

	const PacketHeader & operator=(const PacketHeader & ph)
	{
		m_type = ph.m_type;
		memcpy(m_buffer, ph.m_buffer, sizeof(PacketHeader));

		return *this;
	}
};

struct PacketData
{
	unsigned char m_buffer[BLOCK_SIZE];
};

/*
 * ========= PACKET HEADERS =========
 */

struct PacketHeader_Interface
{
	char m_path[480];
};

struct PacketHeader_FileInfo
{
	/*
	 * ACTION is compatible with FileGatherer::FILE_INFO_FLAG
	 */
	enum ACTION
	{
		A_NONE		= 0x0000,

		A_ADD		= 0x0004,
		A_CHANGE	= 0x0008,
		A_DELETE	= 0x0010
	};

	short int m_pathId;
	ACTION m_action;
	uint64_t m_size;
	char m_path[PATH_LENGTH];

	PacketHeader_FileInfo() : m_pathId(0), m_action(A_NONE), m_size(0) {}

	PacketHeader_FileInfo(short int pathId, ACTION action, uint64_t size, const char * path)
	{
		m_pathId = pathId;
		m_action = action;
		m_size = size;
		strcpy(m_path, path);
	}

	PacketHeader_FileInfo(const PacketHeader_FileInfo & ph_fi)
	{
		m_pathId = ph_fi.m_pathId;
		m_action = ph_fi.m_action;
		m_size = ph_fi.m_size;
		strcpy(m_path, ph_fi.m_path);
	}

	const PacketHeader_FileInfo & operator=(const PacketHeader_FileInfo & ph_fi)
	{
		m_pathId = ph_fi.m_pathId;
		m_action = ph_fi.m_action;
		strcpy(m_path, ph_fi.m_path);
		m_size = ph_fi.m_size;

		return *this;
	}
};

struct PacketHeader_ChunkInfo
{
	ProcessFileInterface::CHUNK_TYPE m_chunkType;
	offset_t m_offset;

	PacketHeader_ChunkInfo() : m_chunkType(ProcessFileInterface::CHUNK_NONE), m_offset(0) {}
};

#endif // PACKET_H

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

#define BLOCK_SIZE 32448
#define HEADER_BUFF_SIZE 512
#define PATH_LENGTH 480

#ifndef HASH_LENGTH
#define HASH_LENGTH 32
#endif

enum PCKT_TYPE_ID
{
	PCKT_FILE,
	PCKT_FILE_CHUNK,
	PCKT_CHUNK_INFO
};

struct PacketHeader
{
	int m_id;
	PCKT_TYPE_ID m_type;
	unsigned char m_buffer[HEADER_BUFF_SIZE];
};

struct PacketData
{
	int m_id;
	unsigned char m_buffer[BLOCK_SIZE];
};

/*
 * ========= PACKET HEADERS =========
 */

struct PacketHeader_Interface
{
	char m_path[PATH_LENGTH];
};

struct PacketHeader_File : public PacketHeader_Interface {};

struct PacketHeader_FileChunk : public PacketHeader_Interface
{
	int m_chunkId;
};

struct PacketHeader_ChunkInfo : public PacketHeader_Interface
{
	int m_chunkId;
	char m_hash[HASH_LENGTH];
};


// obsolete
struct Packet
{
	Uint8 m_type;
	unsigned char m_buffer[512];
	Uint32 len;
	
	void deserialize(void * buffer)
	{
		Packet * p = (Packet*)buffer;
		m_type = p->m_type;
		memcpy(m_buffer, p->m_buffer, 512);
	}

	void * serialize() const
	{
		return (void*)this;
	}
};

#endif // PACKET_H

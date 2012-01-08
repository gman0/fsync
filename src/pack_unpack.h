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

#include <cassert>
#include "defs.h"
#include "Packet.h"

/*
 * pack/unpack helper methods
 */


template <typename T> PacketHeader packToHeader(const T * t, PACKET_TYPE_ID packetType)
{
	assert(HEADER_BUFF_SIZE >= sizeof(T));
	return PacketHeader(packetType, t, sizeof(T));
}

template <typename T> T unpackFromHeader(const PacketHeader * ph, PACKET_TYPE_ID packetType)
{
	assert(packetType == ph->m_type);
	return *(T*)ph->m_buffer;
}



// specialization

template <> PacketHeader packToHeader(const bool * t, PACKET_TYPE_ID packetType)
{
	/*
	 * type check is not really needed because HEADER_BUFF_SIZE is bigger
	 * than size of bool
	 */
	 

	PacketHeader ph;
	ph.m_type = packetType;
	ph.m_buffer[0] = *t;

	return ph;
}


template <> bool unpackFromHeader(const PacketHeader * ph, PACKET_TYPE_ID packetType)
{
	assert(packetType == ph->m_type);
	return ph->m_buffer[0];
}

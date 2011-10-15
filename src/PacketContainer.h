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

#ifndef PACKET_CONTAINER_H
#define PACKET_CONTAINER_H

#include <ios>
#include <istream>
#include "Packet.h"

class PacketContainer
{
	private:
		int m_id;
		std::istream & m_data;
		PacketHeader * m_header;
		size_t m_dataLen;
		size_t m_dataSent;
		
		int m_blocks;
		PacketData * m_currentBlock;
		int m_currentBlockSize;
	
	public:
		PacketContainer(int id, PCKT_TYPE_ID type, int headerDataLen, const unsigned char * headerData,
						std::istream & data, size_t dataLen = 0);
		~PacketContainer();

		const PacketHeader * getHeader() const;
		
		PacketData * nextBlock();
		int getBlockSize() const;
		
		bool dataFullySent() const;
		size_t getDataSize() const;
		size_t dataSent() const;
};

#endif // PACKET_CONTAINER_H

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
#include <cmath>
#include "PacketContainer.h"

using namespace std;

PacketContainer::PacketContainer(int id, PCKT_TYPE_ID type, int headerDataLen, const unsigned char * headerData,
								istream & data, size_t dataLen) :
	m_id(id),
	m_data(data),
	m_dataSent(0),
	m_blocks(0),
	m_currentBlock(0)
{
	m_header = new PacketHeader;

	m_header->m_id = id,
	m_header->m_type = type;
	memcpy(m_header->m_buffer, headerData, headerDataLen);

	if (dataLen == 0)
	{
		m_data.seekg(0, istream::end);
		m_dataLen = m_data.tellg();
		m_data.seekg(0);
	}
	else
		m_dataLen = dataLen;
	
	// TODO: add the m_id's type size
	m_blocks = ceil(m_dataLen / BLOCK_SIZE);
}

PacketContainer::~PacketContainer()
{
	if (m_currentBlock)
		delete m_currentBlock;
	
	delete m_header;
}

const PacketHeader * PacketContainer::getHeader() const
{
	return m_header;
}

PacketData * PacketContainer::nextBlock()
{
	if (m_currentBlock)
		delete m_currentBlock;

	m_currentBlock = new PacketData;
	m_currentBlock->m_id = m_id;

	int dataLen = BLOCK_SIZE;

	if (m_dataSent + dataLen > m_dataLen)
		dataLen = m_dataLen - m_dataSent;

	m_data.readsome((char*)m_currentBlock->m_buffer, dataLen);
	m_data.seekg(m_dataSent + dataLen);

	m_currentBlockSize = dataLen;

	return m_currentBlock;
}

int PacketContainer::getBlockSize() const
{
	return m_currentBlockSize;
}

bool PacketContainer::dataFullySent() const
{
	return (m_dataLen == m_dataSent);
}

size_t PacketContainer::getDataSize() const
{
	return m_dataLen;
}

size_t PacketContainer::dataSent() const
{
	return m_dataSent;
}

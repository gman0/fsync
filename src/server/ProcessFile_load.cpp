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

#include "ProcessFile_load.h"
#include "defs.h"

using namespace std;

ProcessFile_load::ProcessFile_load(const char * filePath, unsigned char * buf) :
	ProcessFileInterface(m_file, buf), m_block(0)
{
	m_file.open(filePath, ios::in);
	prepare();
	m_block = (PacketData*)buf;
}

ProcessFile_load::~ProcessFile_load()
{
	m_file.close();
}

PacketData * ProcessFile_load::getBlock(offset_t offset)
{
	if (offset >= m_dataLen)
		return 0;

	m_file.seekg(offset);
	m_file.readsome((char*)m_block->m_buffer, getBlockSize(offset));

	return m_block;
}

PacketData * ProcessFile_load::nextBlock()
{
	offset_t offset = getOffset();
	PacketData * block = getBlock(offset);
	m_file.seekg((streamoff)offset + getBlockSize(offset));

	return block;
}

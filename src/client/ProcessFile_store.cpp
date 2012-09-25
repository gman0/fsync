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

#include "ProcessFile_store.h"

using namespace std;

ProcessFile_store::ProcessFile_store(const char * filePath, size_t size, unsigned char * buf) :
	ProcessFileInterface(m_file, buf)
{
	m_file.open(filePath, ios::out);
	prepare();

	m_dataLen = size;
}

ProcessFile_store::ProcessFile_store(const char * filePath, unsigned char * buf) :
	ProcessFileInterface(m_file, buf)
{
	m_file.open(filePath);
	prepare();
}

ProcessFile_store::~ProcessFile_store()
{
	m_file.close();
}

void ProcessFile_store::feedBlock(offset_t offset, const PacketData * data)
{
	int blockSize = getBlockSize(offset);
	printf("%d %lu\n", blockSize, offset);
	m_file.seekp(offset);
	m_file.write((char*)data->m_buffer, getBlockSize(offset));
}

void ProcessFile_store::feedNextBlock(const PacketData * data)
{
	streamoff offset = getOffset();

	feedBlock(offset, data);

	m_file.seekp(offset + getBlockSize(offset));
}

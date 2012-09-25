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

#ifndef PROCESS_FILE_LOAD_H
#define PROCESS_FILE_LOAD_H

#include <fstream>
#include "ProcessFileInterface.h"
#include "Packet.h"
#include "defs.h"

class ProcessFile_load : public ProcessFileInterface
{
	private:
		std::fstream m_file;
		PacketData * m_block;

	public:
		ProcessFile_load(const char * filePath, unsigned char * buf);
		~ProcessFile_load();

		inline void setOffset(offset_t offset) { m_file.seekg(offset); }
		inline offset_t getOffset() { return m_file.tellg(); }

		PacketData * getBlock(offset_t offset);
		PacketData * nextBlock();
};

#endif // PROCESS_FILE_LOAD_H

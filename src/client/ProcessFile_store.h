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

#ifndef PROCESS_FILE_STORE
#define PROCESS_FILE_STORE

#include <fstream>
#include "ProcessFileInterface.h"
#include "Packet.h"

class ProcessFile_store : public ProcessFileInterface
{
	private:
		std::fstream m_file;

	public:
		ProcessFile_store(const char * filePath, size_t size, unsigned char * buf);

		/*
		 * This constructor assumes that the file exists and also
		 * opens it both for reading and writing.
		 */
		ProcessFile_store(const char * filePath, unsigned char * buf);
		~ProcessFile_store();

		inline void setOffset(offset_t offset) { m_file.seekp(offset); }
		inline offset_t getOffset() { return m_file.tellg(); }

		void feedBlock(offset_t offset, const PacketData * data);
		void feedNextBlock(const PacketData * data);
};

#endif // PROCESS_FILE_STORE

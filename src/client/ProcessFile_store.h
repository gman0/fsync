#ifndef PROCESS_FILE_STORE
#define PROCESS_FILE_STORE

#include <fstream>
#include "ProcessFile.h"
#include "Packet.h"

class ProcessFile_store : public ProcessFile
{
	private:
		std::fstream m_file;

	public:
		ProcessFile_store(const char * filePath, size_t size);

		/*
		 * This constructor assumes that the file exists and also
		 * opens it both for reading and writing.
		 */
		ProcessFile_store(const char * filePath);
		~ProcessFile_store();

		void feedBlock(offset_t offset, const PacketData * data);
		void feedNextBlock(const PacketData * data);
};

#endif // PROCESS_FILE_STORE

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
		ProcessFile_store(const char * filePath, size_t size);

		/*
		 * This constructor assumes that the file exists and also
		 * opens it both for reading and writing.
		 */
		ProcessFile_store(const char * filePath);
		~ProcessFile_store();

		inline void setOffset(offset_t offset) { m_file.seekp(offset); }
		inline offset_t getOffset() { return m_file.tellg(); }

		void feedBlock(offset_t offset, const PacketData * data);
		void feedNextBlock(const PacketData * data);
};

#endif // PROCESS_FILE_STORE

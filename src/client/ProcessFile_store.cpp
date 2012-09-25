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
	m_file.seekp(offset);
	m_file.write((char*)data->m_buffer, getBlockSize(offset));
}

void ProcessFile_store::feedNextBlock(const PacketData * data)
{
	streamoff offset = getOffset();

	feedBlock(offset, data);

	m_file.seekp(offset + getBlockSize(offset));
}

#include "ProcessFile_store.h"
#include <iostream>

using namespace std;

ProcessFile_store::ProcessFile_store(const char * filePath, size_t size) : ProcessFile(m_file)
{
	m_file.open(filePath, ios::out);
	prepare();

	m_dataLen = size;
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
	streamoff offset = getPOffset();

	feedBlock(offset, data);

	m_file.seekp(offset + getBlockSize(offset));
}

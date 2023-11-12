#if defined(IMPLEMENT_WINDOWS_APIS) || defined(__APPLE__)

#include "CFile.h"
#include <filesystem>

CFile::CFile()
{
}

CFile::CFile(const char* path, unsigned int mode): CFile()
{
	Open(path, mode);
}

bool CFile::Open(const char* path, unsigned int mode)
{
	if (_file.is_open()) {
		_file.close();
	}

	_path = path;

	std::ios_base::openmode openMode = 0;
	if (mode & OpenMode::modeRead) {
		openMode |= std::ios_base::in;
	}
	if (mode & OpenMode::modeWrite) {
		openMode |= std::ios_base::out;
	}
	if (mode & OpenMode::modeReadWrite) {
		openMode |= (std::ios_base::in | std::ios_base::out);
	}
	if (mode & OpenMode::modeCreate) {
		openMode |= std::ios_base::trunc;
	}

	_file.open(path, openMode);
	return _file.is_open();
}

CFile::~CFile() {
    Close();
}

void CFile::Close()
{
	if (_file.is_open()) {
		_file.close();
	}
	_path.clear();
}

void CFile::Remove(const char* path)
{
	std::filesystem::remove(path);
}

bool CFile::Rename(std::string from, std::string to)
{
	try {
		std::filesystem::rename(from, to);
		return true;
	}
	catch (...) {
		return false;
	}
}

unsigned int CFile::Read(void* data, unsigned int size)
{
	_file.read((char*)data, size);
	if (fileStreamFailed()) {
		return 0;
	}
	else {
		return _file.gcount();
	}
}

bool CFile::Write(void* data, unsigned int size)
{
	_file.write((const char*)data, size);
	return !fileStreamFailed();
}

void CFile::Seek(uint64_t offset, SeekPosition pos)
{
	auto streamPos = std::ios_base::beg;
	switch (pos) {
	case begin: 
		streamPos = std::ios_base::beg;
		break;
	case current:
		streamPos = std::ios_base::cur;
		break;
	case end:
		streamPos = std::ios_base::end;
		break;
	}
	_file.seekg(offset, streamPos);
    if(fileStreamFailed()) {
        int x = 0;
    }
	_file.seekp(offset, streamPos);
    if(fileStreamFailed()) {
        int x = 0;
    }
}

uint64_t CFile::SeekToEnd()
{
	Seek(0, SeekPosition::end);
	return _file.tellg();
}

void CFile::SetLength(uint64_t length)
{
	// TODO: Check whether it actually works
	std::filesystem::resize_file(_path, length);
}

uint64_t CFile::GetLength() const
{
	return std::filesystem::file_size(_path);
}

bool CFile::fileStreamFailed()
{
	return (_file.bad() || _file.fail());
}

void CStdioFile::WriteString(const char* str)
{
	size_t strLength = strlen(str);
	Write((void*)str, strLength);
}

char* CStdioFile::ReadString(char* str, unsigned int maxLength)
{
	Read((void*)str, maxLength - 1);
	return str;
}

#endif // IMPLEMENT_WINDOWS_APIS

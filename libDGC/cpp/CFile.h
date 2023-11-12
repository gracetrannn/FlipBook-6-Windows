#pragma once

#include <cstdint>
#include <string>

class CFile {
private:
    class Impl;
    Impl* _pimpl;
	std::string _path;

public:
	enum OpenMode : unsigned int {
		modeRead = 0x0000,
		modeWrite = 0x0001,
		modeReadWrite = 0x0002,
		modeCreate = 0x1000,
		typeText = 0x4000
	};

	enum SeekPosition: unsigned int { begin = 0x0, current = 0x1, end = 0x2 };

public:
	CFile();
	CFile(const char* path, unsigned int mode);
    virtual ~CFile();
	bool Open(const char* path, unsigned int mode);
	void Close();
	unsigned int Read(void* data, unsigned int size);
	bool Write(void* data, unsigned int size);
	void Seek(uint64_t offset, SeekPosition pos);
	uint64_t SeekToEnd();
	void SetLength(uint64_t length);
	uint64_t GetLength() const;

	static void Remove(const char* path);
	static bool Rename(std::string from, std::string to);

private:
	bool fileStreamFailed();
};

class CStdioFile : public CFile {
public:
	void WriteString(const char* str);
	char* ReadString(char* str, unsigned int maxLength);
};

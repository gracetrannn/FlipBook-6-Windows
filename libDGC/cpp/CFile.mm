#include "CFile.h"
#import <Foundation/Foundation.h>

struct CFile::Impl {
    NSFileHandle* _handle;
};

CFile::CFile()
{
    _pimpl = new Impl();
}

CFile::CFile(const char* path, unsigned int mode): CFile()
{
    Open(path, mode);
}

bool CFile::Open(const char* path, unsigned int mode)
{
    NSString* filePath = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];
    _path = path;
    
    if (mode == OpenMode::modeCreate || mode & OpenMode::modeCreate) {
        NSFileManager* fm = [NSFileManager defaultManager];
        if (![fm fileExistsAtPath:filePath]) {
            [fm createFileAtPath:filePath contents:[NSData data] attributes:nil];
        }
    }
    
    if (mode == OpenMode::modeRead || mode & OpenMode::modeRead) {
        _pimpl->_handle = [NSFileHandle fileHandleForReadingAtPath:filePath];
    }
    if (mode == OpenMode::modeWrite || mode & OpenMode::modeWrite) {
        _pimpl->_handle = [NSFileHandle fileHandleForWritingAtPath:filePath];
    }
    if (mode == OpenMode::modeReadWrite || mode & OpenMode::modeReadWrite) {
        _pimpl->_handle = [NSFileHandle fileHandleForUpdatingAtPath:filePath];
    }

    return _pimpl->_handle != nil;
}

CFile::~CFile() {
    Close();
    delete _pimpl;
}

void CFile::Close()
{
    if(_pimpl->_handle) {
        [_pimpl->_handle closeFile];
    }
    _path.clear();
}

void CFile::Remove(const char* path)
{
    NSString* filePath = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];
    BOOL is_dir = NO;
    NSFileManager* fm = [NSFileManager defaultManager];
    if ([fm fileExistsAtPath:filePath isDirectory:&is_dir] && (is_dir == NO)) {
        [fm removeItemAtPath:filePath error:nil];
    }
}

bool CFile::Rename(std::string from, std::string to)
{
    NSFileManager* fm = [NSFileManager defaultManager];
    NSString* fromPath = [NSString stringWithCString:from.c_str() encoding:NSUTF8StringEncoding];
    NSString* toPath = [NSString stringWithCString:from.c_str() encoding:NSUTF8StringEncoding];
    NSError* error = nil;
    [fm moveItemAtPath:fromPath toPath:toPath error:&error];
    return error != nil;
}

unsigned int CFile::Read(void* data, unsigned int size)
{
    NSUInteger bytes_read = 0;
    
    if (_pimpl->_handle) {
        @try {
            NSData* d = [_pimpl->_handle readDataOfLength:size];
            if (d) {
//                memmove(data, d.bytes, d.length);
//                bytes_read = d.length;
                
                NSRange r;
                r.location = 0;
                r.length = size;
                [d getBytes:data range:r];
                bytes_read = [d length];
            }
        }
        @catch (NSException* e) {
            NSLog (@"error reading from file handle: %@", [_pimpl->_handle description]);
            NSLog (@"exception is: %@", [e description]);
        }
    }
    
    return (unsigned int)bytes_read;
}

bool CFile::Write(void* data, unsigned int size)
{
    if (_pimpl->_handle) {
        NSData* d = [NSData dataWithBytes:data length:size];
        NSError* err = nil;
        if (@available(iOS 13.0, *)) {
            [_pimpl->_handle writeData:d error:&err];
        } else {
            [_pimpl->_handle writeData:d];
        }
        return err == nil;
    }
    else {
        NSLog (@"error writing to file, invalid handle");
        return false;
    }
}

void CFile::Seek(uint64_t offset, SeekPosition pos)
{
    int seekPos = SEEK_SET;
    switch (pos) {
    case begin:
        seekPos = SEEK_SET;
        break;
    case current:
        seekPos = SEEK_CUR;
        break;
    case end:
        seekPos = SEEK_END;
        break;
    }
    
    lseek([_pimpl->_handle fileDescriptor], offset, seekPos);
}

uint64_t CFile::SeekToEnd()
{
    return [_pimpl->_handle seekToEndOfFile];
}

void CFile::SetLength(uint64_t length)
{
    [_pimpl->_handle truncateFileAtOffset:length];
}

uint64_t CFile::GetLength() const
{
    uint64_t current = [_pimpl->_handle offsetInFile];
    [_pimpl->_handle seekToFileOffset:0];
    uint64_t len = [_pimpl->_handle seekToEndOfFile];
    [_pimpl->_handle seekToFileOffset:current];
    return len;
}

void CStdioFile::WriteString(const char* str)
{
    size_t strLength = strlen(str);
    Write((void*)str, (unsigned int)strLength);
}

char* CStdioFile::ReadString(char* str, unsigned int maxLength)
{
    Read((void*)str, maxLength - 1);
    return str;
}

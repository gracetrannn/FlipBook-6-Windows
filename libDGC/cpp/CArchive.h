#pragma once

#include "CFile.h"

class CArchive
{
public:

    CArchive();
    CArchive(CFile* inFile, int inMode);
    virtual ~CArchive();

    enum {
        load = 1
    };

    CFile* GetFile();
    //bool IsStoring();
    char* ReadString(char* outString, int inSize);
    void Close();

protected:

    CFile* fFile;
};
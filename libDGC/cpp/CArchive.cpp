#if defined(IMPLEMENT_WINDOWS_APIS) || defined(__APPLE__)

#include "CArchive.h"

CArchive::CArchive() : fFile(NULL)
{
}

CArchive::CArchive(CFile* inFile, int inMode)
    : fFile(inFile)
{
}

CArchive::~CArchive()
{
}

CFile* CArchive::GetFile()
{
    return fFile;
}

//bool CArchive::IsStoring()
//{
//    // fixme
//    return false;
//}

char* CArchive::ReadString(char* outString, int inSize)
{
    // read up to newline or end of file or max chars

    bool done = false;
    bool hit_first_char = false;
    char c;
    int count = 0;

    while (!done) {

        int num_read = fFile->Read(&c, 1);
        count++;

        if ((num_read == 0) || (c == '\n') || (c == '\r')) {
            if (hit_first_char) {
                done = true;
                outString[count - 1] = '\0';
            }
            else {
                count--;
            }
        }
        else {
            outString[count - 1] = c;
            hit_first_char = true;
        }

        if (count >= (inSize - 1)) {
            outString[inSize - 1] = '\0';
            done = true;
        }
    }

    return outString;
}

void CArchive::Close()
{
    fFile->Close();
}

#endif // IMPLEMENT_WINDOWS_APIS
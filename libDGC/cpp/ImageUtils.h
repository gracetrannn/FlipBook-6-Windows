#pragma once

typedef enum {
	kRawDataType = 0, // no header, w,h,d from elsewhere
	kBMPDataType = 1  // bitmapinfoheader preceeds data
} FBDataKind;

typedef unsigned int (*ImageDataCallback)(FBDataKind inDataKind, void* inData, int inDataSize, void* inUserObject);

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

/*
	FBImportStill
*/
int FBImportStill(const char* inFilePath, ImageDataCallback inFunction, void* inUserObject);
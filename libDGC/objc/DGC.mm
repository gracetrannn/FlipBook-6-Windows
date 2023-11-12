//
//  FBDGCSceneDatabase.m
//  FlipPad
//
//  Created by Alex Vihlayew on 11/2/20.
//  Copyright © 2020 Alex. All rights reserved.
//

#import "DGC.h"

#include "CSceneBase.h"
#include "CMyImage.h"
#include "CCell.h"
#include "MyIO.h"
#include "Resource.h"
#include "MacResourceLoader.h"
#include "CLayers.h"
#include "CNewPals.h"
#include "SceneOpt.h"

#import "UIImage_Extras.h"

#include <cstdlib>

@interface DGC ()

@property (strong, nonatomic) NSString* path;

@property (assign, nonatomic) CMyIO* c_io;
@property (assign, nonatomic) CSceneBase* c_scene;
@property (assign, nonatomic) CLayers* layers;

@end

@implementation DGC

#pragma mark - Init

- (id)initWithPath:(NSString *)inPath {    
    self = [super init];
    if (self) {
        [self setPath:inPath];
        
        NSFileManager* fm = [NSFileManager defaultManager];
        NSString* dgxPath = [inPath stringByReplacingCharactersInRange:NSMakeRange(inPath.length - 3, 3) withString:@"dgx"];
        if([fm fileExistsAtPath:dgxPath]) {
            [fm removeItemAtPath:dgxPath error:nil];
        }
        
        const char* fileName = [_path cStringUsingEncoding:NSUTF8StringEncoding];
        
        int result = 0;
        
        // if file doesn't exist
        if (![self exists]) {
            // Initialize IO
            _c_io = new CMyIO();
            result = _c_io->Create(fileName);
            // Create scene
            _c_scene = new CSceneBase(_c_io, std::make_unique<MacResourceLoader>());
            result = _c_scene->Make(0, 0, 1920, 1080, 30, 10, 2, 1, 0, 0, 0);
            result = _c_scene->SaveCache(fileName);
            result = _c_scene->Write();
            result = _c_scene->Read(FALSE);
            
            _c_scene->SetMinBG(BGMIN, 1);
            
            // Save file
            result = _c_io->Save(fileName, 0);
            result = _c_io->Close();
            
            delete _c_scene;
            delete _c_io;
        }

        [self openDocument];
        
//        // Initialize IO
//        _c_io = new CMyIO();
//        _c_io->Close(TRUE);
//        // Open file
//        result = _c_io->Open(fileName);
//        // Create scene
//        _c_scene = new CSceneBase(_c_io, std::make_unique<MacResourceLoader>());
//        result = _c_scene->Read(FALSE);
//        // File is open and ready for usage
//
////        if(result != 0) {
////            return nil;
////        }

        self.layers = new CLayers();
        self.layers->Setup(_c_scene, true);
    }

    return self;
}

- (BOOL)openDocument {
    const char* fileName = [_path cStringUsingEncoding:NSUTF8StringEncoding];
    
    CMyIO* pIO = new CMyIO();
    int result = pIO->Test(fileName);
    delete pIO;
    
    if (result == 0) {
        if (_c_scene) {
            _c_scene->ClipEmpty(1);
        }
        delete _c_scene;
        _c_scene = 0;
        
        if (_c_io) {
            _c_io->Close();
        } else {
            _c_io = new CMyIO();
        }
        result = _c_io->Open(fileName);
        if (!_c_scene) {
            _c_scene = new CSceneBase(_c_io, std::make_unique<MacResourceLoader>());
        }
        int m_dwFeatures = 25728;
        int m_id = 0;
        result = _c_scene->Read(false, m_dwFeatures, m_id);
    }
    
    UINT c = result & 0x3000;
    UINT r = result & 0x4000;
    UINT q = result & 0x8000;
    result &= 0xfff;
    
    if (result == 0) {
        _c_scene->LoadCache(fileName);
    }
    
    if (c ? true : false) {
        _c_scene->MakeModified();
    }

    return result == 0;
}

- (BOOL)exists {
    return [[NSFileManager new] fileExistsAtPath:_path];
}

#pragma mark - Properties

- (NSInteger)levels {
    UINT levels = _c_scene->LevelCount();
    return MAX(2, levels);
}

- (NSInteger)frames {
    return self.c_scene->FrameCount();
}

- (NSInteger)frameRate {
    return self.c_scene->FrameRate();
}

- (CGSize)frameSize {
    CGFloat width = self.c_scene->Width();
    CGFloat height = self.c_scene->Height();
    return CGSizeMake(width, height);
}

- (CGSize)frameComSize {
    CGFloat width = self.c_scene->ComW();
    CGFloat height = self.c_scene->ComH();
    return CGSizeMake(width, height);
}

- (BOOL)levelUsed:(NSInteger)level {
    return self.c_scene->LevelFlags((UINT)level) & 0x1;
}

- (NSString*)levelNameAtIndex:(NSInteger)index {
    char buf[300];
    self.c_scene->LevelName(buf, (UINT)index, false);
    return @(buf);
}

- (void)setLevelName:(NSString*)name atIndex:(NSInteger)index {
    char buf[300];
    strcpy(buf, [name UTF8String]);
    self.c_scene->LevelName(buf, (UINT)index, true);
}

#pragma mark - Get images

- (UIImage *)getThumbnail {
    return [self getThumbnailForFrame:0 level:0];
}

//- (UIImage *)overlayImageForFrame:(NSInteger)frame level:(NSInteger)level {
//    return [self getCompositedImageForFrame:frame level:level];
//}
//
- (UIImage *)backgroundImageForFrame:(NSInteger)frame {
    //NSLog(@"==== bg: %td", frame);
    UIImage* paintImage = [self getRgbImageOfKind:CCell::LAYER_BG row:frame level:0];
    return paintImage;
}
//
//- (UIImage *)imagePencilForFrame:(NSInteger)frame level:(NSInteger)level {
//    return [self getImageOfKind:CCell::LAYER_INK frame:frame level:level];
//}
//
//- (UIImage *)imagePaintForFrame:(NSInteger)frame level:(NSInteger)level {
//    return [self getImageOfKind:CCell::LAYER_PAINT frame:frame level:level];
//}

#pragma mark - Get images Data

- (NSData *) paintImageDataForFrame:(NSInteger)frame level:(NSInteger)level convert: (bool)convert32 depth: (uint32_t*)depth {
//    *depth = 32;
//    return [self getIndexedImageDataOfKind:CCell::LAYER_PAINT frame:frame level:level];
    return [self otherImageDataOfKind:CCell::LAYER_PAINT frame:frame level:level convert: convert32 depth: depth];
}

- (NSData *) pencilImageDataForFrame:(NSInteger)frame level:(NSInteger)level convert: (bool)convert32 depth: (uint32_t*)depth {
//    *depth = 32;
//    return [self getIndexedImageDataOfKind:CCell::LAYER_INK frame:frame level:level];
    return [self otherImageDataOfKind:CCell::LAYER_INK frame:frame level:level convert: convert32 depth: depth];
}

- (NSData *)backgroundImageDataForFrame:(NSInteger)frame convert: (bool)convert32 depth: (uint32_t*)depth {
    NSInteger level = 0;
//    UINT min = 100;
    UINT min = 50;
    bool bCamera = true;
    bool bBroadcast = false;
    if(frame != self.layers->CurrentFrame() || level != self.layers->CurrentLevel()) {
        self.layers->Select((UINT)frame, (UINT)level);
    }
    
    uint32_t size = _c_scene->GetSizeForLevel0(bCamera);
    BYTE* pData = new BYTE[size];
    memset(pData, 0, size);
    
    bool result = _c_scene->GetLevel0Data(pData, (UINT)frame, true, min, bCamera, bBroadcast);
    if (!result) {
        delete [] pData;
        return nil;
    }
    
    *depth = _c_scene->GetDepthImage();
    
    NSData* data = [self getDataConvert32: pData size:size convert32:convert32 depth:depth isComSize:true];
    delete [] pData;
    
    return data;
}

- (NSData *)otherImageDataOfKind:(UINT)kind frame:(NSInteger)frame level:(NSInteger)level convert: (bool)convert32 depth: (uint32_t*)depth {
    int result = 0;
    if(frame != self.layers->CurrentFrame() || level != self.layers->CurrentLevel()) {
        self.layers->Select((UINT)frame, (UINT)level);
    }
    
    DWORD key;
    result = self.c_scene->GetImageKey(key, (UINT)frame, (UINT)level, kind);
    if(key == 0) {
        return nil;
    }
    if(result != 0) {
        return nil;
    }

    UINT img_width, img_height, img_depth;
    result = self.c_scene->ImageInfo(img_width, img_height, img_depth, key);
    if(result != 0) {
        return nil;
    }
    
    uint32_t size = img_width * img_height * img_depth / 8;
    *depth = img_depth;
    
    BYTE* pData = nullptr;
    if(*depth == 16) {
        if(!self.layers->GetPointerToLayerImage(pData, kind))
            return nil;
    } else {
        BYTE* pData = new BYTE[size];
        memset(pData, 0, size);
        
        self.layers->m_nFlags = 3;
        
        result = self.c_scene->ReadImage(pData, key);
        if(result != 0) {
            delete [] pData;
            return nil;
        }
    }
    
    NSData* data = [self getDataConvert32: pData size:size convert32:convert32 depth:depth isComSize:false];
    if (*depth != 16) {
        delete [] pData;
    }
    
    return data;
}

- (NSData *) getDataConvert32: (BYTE* )pData size: (uint32_t)size convert32: (bool)convert32 depth: (uint32_t*)depth isComSize: (bool)isComSize {
    NSData* data = nil;
    if (convert32) {
        int height, width;
        if(isComSize) {
            height = [self frameComSize].height;
            width = [self frameComSize].width;
        } else {
            height = [self frameSize].height;
            width = [self frameSize].width;
        }
        uint32_t size32 = 4 * height * width;
        BYTE* pData32 = new BYTE[size32];
//        memset(pData32, 255, size32);
        memset(pData32, 0, size32);
        
        switch (*depth) {
            case 16: {
                [self convertData16To32: pData imgBuffer32:pData32 width:width height:height];
                break;
            }
            case 24: {
                [self convertData24To32:pData imgBuffer32:pData32 width:width height:height];
                break;
            }
            case 32: {
                memmove(pData32, pData, size32);
                break;
            }
            default:
                break;
        }
        data = [[NSData alloc] initWithBytes:pData32 length:size32];
        delete [] pData32;
    } else {
        data = [[NSData alloc] initWithBytes:pData length:size];
    }
    
//    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
//    NSString *documentsDirectory = [paths objectAtIndex:0];
//    NSString *dataPath = [documentsDirectory stringByAppendingPathComponent:@"bg.dat"];
//    [data writeToFile:dataPath atomically:YES];
//    NSLog(@"Data was written to %@", dataPath);
    
    return data;
}

//- (NSData *)getRgbaBitmapImageDataOfKind:(UINT)kind frame:(NSInteger)frame level:(NSInteger)level {
//    int result = 0;
//    DWORD key;
//    result = _c_scene->GetImageKey(key, (UINT)frame, (UINT)level, kind);
//    if (key == 0) {
//        return nil;
//    }
//    if (result != 0) {
//        return nil;
//    }
//    // Create image
//    CMyImage* image = new CMyImage(_c_io);
//    image->SetKey(key);
//    image->Read(NULL);
//    // Get props
//    DWORD width = image->Width();
//    DWORD height = image->Height();
//    //
//    int bufferSize = width * height * 4;
//    uint8_t* buffer = new uint8_t[bufferSize];
//    image->Read(buffer);
//    NSData* data = [[NSData alloc] initWithBytes:buffer length:bufferSize];
//    delete[] buffer;
//    return data;
//}

//- (NSData *)getIndexedImageDataOfKind:(UINT)kind frame:(NSInteger)frame level:(NSInteger)level {
//    DWORD key;
//    self.c_scene->GetImageKey(key, (UINT)frame, (UINT)level, 99);
//    if(key == 0) {
//        return nil;
//    }
//
//    DWORD w = self.c_scene->Width();
//    DWORD h = self.c_scene->Height();
//
//    CNewPals* pals = self.c_scene->LevelPalette((UINT)level);
//    const int bpp = 4;
//    uint8_t* imgBuffer = new uint8_t[bpp*w*h];
//    memset(imgBuffer, 0, bpp*w*h);
//    if(frame != self.layers->CurrentFrame() || level != self.layers->CurrentLevel()) {
//        self.layers->Select((UINT)frame, (UINT)level);
//    }
//    self.layers->m_nFlags = 3;
//    self.layers->GetLayerImage(imgBuffer, kind, false, true);
//
//    NSData* data = [[NSData alloc] initWithBytes:imgBuffer length:bpp*w*h];
//    delete [] imgBuffer;
//    return data;
//}

#pragma mark - Composite image

- (NSData *)compositeImageDataForFrame:(NSInteger)frame {
    NSInteger level = 0;
    bool bCamera = true;
    bool bBroadcast = false;
    if(frame != self.layers->CurrentFrame() || level != self.layers->CurrentLevel()) {
        self.layers->Select((UINT)frame, (UINT)level);
    }
    
    uint32_t size = _c_scene->GetSizeForLevel0(bCamera);
    BYTE* pData = new BYTE[size];
    memset(pData, 0, size);
    
    _c_scene->CompositeFrame(pData, (UINT)level, _c_scene->LevelCount() - 1, (UINT)frame, bBroadcast);
    
    uint32_t depth;
    depth = _c_scene->GetDepthImage();
    
    NSData* data = [self getDataConvert32: pData size:size convert32:true depth:&depth isComSize:true];
    delete [] pData;
    
    return data;
}

#pragma mark - Convert images to 32

- (void)convertData16To32:(uint8_t*)imgBuffer16 imgBuffer32:(uint8_t*)imgBuffer32 width:(NSInteger)width height:(NSInteger)height {
    width = self.layers->Width();
    height = self.layers->Height();
    
    uint8_t* pAlpha = imgBuffer16;
    uint8_t* pImage = pAlpha + width * height;
    const int bpp = 4;
    const int stride = (int)width * bpp;
    
    CNewPals * pPals = self.c_scene->LevelPalette(self.layers->CurrentLevel());
    BYTE color[4];
    
    for(int y = self.layers->m_maxy; y >= self.layers->m_miny; y--) {
        for(int x = self.layers->m_minx; x < self.layers->m_maxx; x++) {
            int index = (self.layers->m_maxy - y) * (int)width + x;
            pPals->Color(color, pImage[index], x, y);
            uint8_t alpha = pAlpha[index];
            
            uint8_t red = color[0];
            uint8_t blue = color[2];
            color[0] = blue;
            color[2] = red;
            
            if(alpha > 0) {
                color[3] = (alpha * color[3]) / 255;
                memcpy(imgBuffer32 + y * stride + x * bpp, color, 4);
            }
        }
    }
}

- (void)convertData24To32:(uint8_t*)imgBuffer24 imgBuffer32:(uint8_t*)imgBuffer32 width:(NSInteger)width height:(NSInteger)height {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int position24 = y * (int)width + x;
            int position32 = ((int)height - 1 - y) * (int)width + x;
            
            imgBuffer32[position32 * 4 + 0] = imgBuffer24[position24 * 3 + 0];
            imgBuffer32[position32 * 4 + 1] = imgBuffer24[position24 * 3 + 1];
            imgBuffer32[position32 * 4 + 2] = imgBuffer24[position24 * 3 + 2];
            imgBuffer32[position32 * 4 + 3] = 255;
        }
    }
}

#pragma mark - Set images

//- (void) setPencilImage:(UIImage*)image forFrame:(NSInteger)frame level:(NSInteger)level {
//    if(level > 0) {
//        [self writeLayerImage:image ofKind:CCell::LAYER_INK frame:frame level:level];
//    }
//}
//
//- (void) setPaintImage:(UIImage*)image forFrame:(NSInteger)frame level:(NSInteger)level {
//    if(level > 0) {
//        [self writeLayerImage:image ofKind:CCell::LAYER_PAINT frame:frame level:level];
//    }
//}

#pragma mark -

//- (UIImage *)getImageOfKind:(UINT)kind frame:(NSInteger)frame level:(NSInteger)level {
//    DWORD key;
//    self.c_scene->GetImageKey(key, (UINT)frame, (UINT)level, kind);
//    if(key == 0) {
//        return nil;
//    }
//
//    CMyImage* image = new CMyImage(_c_io);
//    image->SetKey(key);
//    image->Read(NULL);
//
//    uint32_t width = image->Width();
//    uint32_t height = image->Height();
//
//    switch (image->Depth()) {
//        case 16: return [self getIndexedImageOfKind:kind frame:frame level:level];
//        case 24: return [self getRgbImageOfKind:kind row:frame level:level];
//        case 32: return [self getRgbaImageOfKind:kind row:frame level:level];
//        default: return nil;
//    }
//}
//
//- (UIImage *)getIndexedImageOfKind:(UINT)kind frame:(NSInteger)frame level:(NSInteger)level {
//    DWORD key;
//    self.c_scene->GetImageKey(key, (UINT)frame, (UINT)level, 99);
//    if(key == 0) {
//        return nil;
//    }
//
//    DWORD w = self.c_scene->Width();
//    DWORD h = self.c_scene->Height();
//
//    CNewPals* pals = self.c_scene->LevelPalette((UINT)level);
//    const int bpp = 4;
//    uint8_t* imgBuffer = new uint8_t[bpp*w*h];
//    memset(imgBuffer, 0, bpp*w*h);
//    //NSLog(@"==== Cell: %td, %td", frame, level);
//    if(frame != self.layers->CurrentFrame() || level != self.layers->CurrentLevel()) {
//        self.layers->Select((UINT)frame, (UINT)level);
//    }
//    self.layers->m_nFlags = 3;
//    self.layers->GetLayerImage(imgBuffer, kind);
//    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
//    CGContextRef ctx = CGBitmapContextCreate((void*)imgBuffer, w, h, 8, w * bpp, colorSpace, kCGImageAlphaPremultipliedLast);
//    CGImageRef cgImage = CGBitmapContextCreateImage(ctx);
//    UIImage* uiImage = [[UIImage alloc] initWithCGImage:cgImage];
//    CGImageRelease(cgImage);
//    CGContextRelease(ctx);
//    delete [] imgBuffer;
//    return uiImage;
//}

- (UIImage *)getRgbImageOfKind:(UINT)kind row:(NSInteger)row level:(NSInteger)level {
    int result = 0;
    DWORD key;
    result = _c_scene->GetImageKey(key, (UINT)row, (UINT)level, kind);
    if (key == 0) {
        return nil;
    }
    if (result != 0) {
        return nil;
    }
    
    // Create image
    CMyImage* image = new CMyImage(_c_io);
    image->SetKey(key);
    image->Read(NULL);
    // Get props
    DWORD w = image->Width();
    DWORD h = image->Height();
    DWORD d = image->Depth();
    // Create buffer & read data
    DWORD size = h * 4 * ((d * w + 31) / 32);
    UINT offset = 0;
    if (d < 9) {
        offset = 1024;
    }
    DWORD dataLength = size + offset;
    BYTE * pData = new BYTE[dataLength];
    result = image->Read(pData + offset);
    if (result != 0) {
        return nil;
    }

//    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
//    NSString *documentsDirectory = [paths objectAtIndex:0];
//    NSString *dataPath = [documentsDirectory stringByAppendingPathComponent:@"bg.dat"];
//    NSData* data = [NSData dataWithBytes:pData length:dataLength];
//    [data writeToFile:dataPath atomically:YES];
//    NSLog(@"Data was written to %@", dataPath);

    UIImage* uiImage = [[UIImage alloc] initFromPixelBytes:(pData + offset) width:w height:h bitDepth:d];

    delete[] pData;
    return uiImage;
}

//- (UIImage *)getRgbaImageOfKind:(UINT)kind row:(NSInteger)row level:(NSInteger)level {
//    int result = 0;
//    DWORD key;
//    result = _c_scene->GetImageKey(key, (UINT)row, (UINT)level, kind);
//    if (key == 0) {
//        return nil;
//    }
//    // Create image
//    CMyImage* image = new CMyImage(_c_io);
//    image->SetKey(key);
//    image->Read(NULL);
//    // Get props
//    DWORD width = image->Width();
//    DWORD height = image->Height();
//    //
//    uint8_t* buffer = new uint8_t[width*height*4];
//    image->Read(buffer);
//    UIImage* ui_image = [[UIImage alloc] initFromRgbaBuffer:buffer width:width height:height premultiplyAlpha:YES flipped:YES];
//    delete[] buffer;
//    return ui_image;
//}

#pragma mark -

- (NSInteger)getLevelWidth:(NSInteger)level twidth:(NSInteger)twidth {
    return (NSInteger)self.c_scene->GetLevelWidth((UINT)level, (UINT)twidth);
}

- (UIImage *)getThumbnailForFrame:(NSInteger)frame level:(NSInteger)level {
    const int bpp = 4;
    const int height = 80;
    const int width = height*self.c_scene->Width()/self.c_scene->Height();
    const int bufferSize = width*height*bpp;
    uint8_t* buffer = new uint8_t[bufferSize];
    int result = self.c_scene->GetThumb(buffer, (UINT)frame, (UINT)level, width, height, bpp, true);
    if(result != 0) {
        return nil;
    }

    UIImage* uiImage = [[UIImage alloc] initFromPixelBytes:buffer width:width height:height bitDepth:bpp*8];
    
    //UIImage* uiImage = [self getImageOfKind:/*CCell::LAYER_THUMB*/99 row:frame level:level];
    return uiImage;
}

//- (UIImage *)getCompositedImageForFrame:(NSInteger)frame level:(NSInteger)level {
//    DWORD key;
//    self.c_scene->GetImageKey(key, (UINT)frame, (UINT)level, 99);
//    if(key == 0) {
//        return nil;
//    }
//
//    DWORD w = self.c_scene->Width();
//    DWORD h = self.c_scene->Height();
//
//    CNewPals* pals = self.c_scene->LevelPalette((UINT)level);
//    const int bpp = 4;
//    uint8_t* imgBuffer = new uint8_t[bpp*w*h];
//    memset(imgBuffer, 0, bpp*w*h);
//    self.layers->Select((UINT)frame, (UINT)level);
//    self.layers->m_nFlags = 3;
//    self.layers->GetImage(imgBuffer, w*bpp);
//    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
//    CGContextRef ctx = CGBitmapContextCreate((void*)imgBuffer, w, h, 8, w * bpp, colorSpace, kCGImageAlphaPremultipliedLast);
//    CGImageRef cgImage = CGBitmapContextCreateImage(ctx);
//    UIImage* uiImage = [[UIImage alloc] initWithCGImage:cgImage scale:1 orientation:UIImageOrientationDownMirrored];
//    CGImageRelease(cgImage);
//    CGContextRelease(ctx);
//    delete [] imgBuffer;
//    return uiImage;
//}

#pragma mark - Data

// Set

//- (void)setPaintImageData:(NSData *)data frame:(NSInteger)frame level:(NSInteger)level {
//////    [self setRgbaBitmapImageData:data ofKind:CCell::LAYER_INK frame:frame level:level];
////    [self setRgbaBitmapImageData:data ofKind:CCell::LAYER_PAINT frame:frame level:level];
//}
//
//- (void)setPencilImageData:(NSData *)data frame:(NSInteger)frame level:(NSInteger)level {
//////    [self setRgbaBitmapImageData:data ofKind:CCell::LAYER_PAINT frame:frame level:level];
////    [self setRgbaBitmapImageData:data ofKind:CCell::LAYER_INK frame:frame level:level];
//}
//
//- (void) setRgbaBitmapImageData:(NSData*)data ofKind:(NSInteger)kind frame:(NSInteger)frame level:(NSInteger)level { // TO CHECK ???
//    size_t pixels = data.length / 4;
//    uint8_t* rgbaBuffer = (uint8_t*)data.bytes;
//
//    DWORD key;
//    int result = _c_scene->GetImageKey(key, (UINT)frame, (UINT)level, (UINT)kind);
//    if (key == 0) {
//        return;
//    }
//
//    // Update image format
//    CMyImage* myImg = new CMyImage(_c_io);
//    myImg->SetKey(key);
//    myImg->Read(NULL);
//    myImg->SetDepth(32);
//    myImg->Compress(rgbaBuffer, pixels*4);
//
//    self.layers->UpdateLayer(rgbaBuffer, pixels*4, (UINT)kind);
//    self.c_io->Save();
//}

//- (NSData *)getRgbImageDataOfKind:(UINT)kind row:(NSInteger)row level:(NSInteger)level {
//    int result = 0;
//    DWORD key;
//    result = _c_scene->GetImageKey(key, (UINT)row, (UINT)level, kind);
//    if (key == 0) {
//        return nil;
//    }
//    // Create image
//    CMyImage* image = new CMyImage(_c_io);
//    image->SetKey(key);
//    image->Read(NULL);
//    // Get props
//    DWORD w = image->Width();
//    DWORD h = image->Height();
//    DWORD d = image->Depth();
//    // Create buffer & read data
//    DWORD size = h * 4 * ((d * w + 31) / 32);
//    UINT offset = 0;
//    if (d < 9) {
//        offset = 1024;
//    }
//    DWORD dataLength = size + offset;
//    BYTE * pData = new BYTE[dataLength];
//    result = image->Read(pData + offset);
//    if (result != 0) {
//        return nil;
//    }
//
//    NSInteger dataCapacity = 4*w*h;
//    NSInteger stride = 4*w;
//    NSInteger originalStride =  4 * ((d * w + 31) / 32);
//    int originalBpp = (int)d/8;
//    uint8_t* imgBuffer = new uint8_t[dataCapacity];
//    for(NSInteger y = h - 1; y >= 0; --y) {
//        for(NSInteger x = 0; x < w; ++x) {
//            uint8_t* pOutPixel = imgBuffer + y*stride + x*4;
//            uint8_t* pInPixel = pData + offset + (h - y - 1)*originalStride + x*originalBpp;
//
//            pOutPixel[0] = pInPixel[2];
//            pOutPixel[1] = pInPixel[1];
//            pOutPixel[2] = pInPixel[0];
//            pOutPixel[3] = 255; // Opacity
//        }
//    }
//
//    NSData* data = [[NSData alloc] initWithBytes:imgBuffer length:dataCapacity];
//    delete[] pData;
//    delete[] imgBuffer;
//
//    return data;;
//}

- (void)setPaintImageData:(NSData *)data sizeImage:(CGSize)size frame:(NSInteger)frame level:(NSInteger)level {
//    [self writeIndexedLayerImageData:data sizeImage:size ofKind:CCell::LAYER_PAINT frame:frame level:level];
}

- (void)setPencilImageData:(NSData *)data sizeImage:(CGSize)size frame:(NSInteger)frame level:(NSInteger)level {
    // TODO: incorrect search index in palette
//    [self writeIndexedLayerImageData:data sizeImage:size ofKind:CCell::LAYER_INK frame:frame level:level];
}

- (void)setPencilImageData:(NSData *)data size:(CGSize)size depth:(NSInteger)depth frame:(NSInteger)frame level:(NSInteger)level {
    [self writeLayerImageData:data size:size depth:depth ofKind:CCell::LAYER_INK frame:frame level:level];
}

- (void)setPaintImageData:(NSData *)data size:(CGSize)size depth:(NSInteger)depth frame:(NSInteger)frame level:(NSInteger)level {
    [self writeLayerImageData:data size:size depth:depth ofKind:CCell::LAYER_PAINT frame:frame level:level];
}

#pragma mark - Utils

- (void) writeLayerImageData:(NSData*)data size:(CGSize)size depth:(NSInteger)depth ofKind:(NSInteger)kind frame:(NSInteger)frame level:(NSInteger)level {
    if (data == nil) {
        return;
    }
    
    switch (depth) {
        case 16:
            [self writeIndexedLayerImageData:data size:size ofKind:kind frame:frame level:level];
            break;
            
        default:
            break;
    }
}

- (void) writeIndexedLayerImageData:(NSData*)data size:(CGSize)size ofKind:(NSInteger)kind frame:(NSInteger)frame level:(NSInteger)level {
    self.layers->Select((UINT)frame, (UINT)level);
    
    uint8_t* pBuffer = (uint8_t*)data.bytes;
    
    self.layers->UpdateLayer(pBuffer, data.length, (UINT)kind);
    self.layers->Put();
    self.c_io->Save();
    
    // Just for creating new thumbnail if it doesn't exist
//    [self getThumbnailForFrame:frame level:level];
}

- (void) writeLayerImage:(UIImage*)image ofKind:(NSInteger)kind frame:(NSInteger)frame level:(NSInteger)level {
//    size_t pixels = image.size.width*image.size.height;
//    uint8_t* rgbaBuffer = (uint8_t*)calloc(pixels*4, sizeof(uint8_t));
//    [image toPixelBuffer:rgbaBuffer flipped:YES];
//    
//    DWORD key;
//    int result = _c_scene->GetImageKey(key, (UINT)frame, (UINT)level, (UINT)kind);
//    if (key == 0) {
//        return;
//    }
//    
//    // Update image format
//    CMyImage* myImg = new CMyImage(_c_io);
//    myImg->SetKey(key);
//    myImg->Read(NULL);
//    myImg->SetDepth(32);
//    myImg->Compress(rgbaBuffer, pixels*4);
//    
//    self.layers->ReplaceLayer(rgbaBuffer, pixels*4, (UINT)kind);
//    self.c_io->Save();
//    
//    // Just for creating new thumbnail if it doesn't exist
//    [self getThumbnailForFrame:frame level:level];
//    
//    free(rgbaBuffer);
}

- (void) writeIndexedLayerImage:(UIImage*)image ofKind:(NSInteger)kind frame:(NSInteger)frame level:(NSInteger)level {
    size_t pixels = image.size.width*image.size.height;
    uint8_t* rgbaBuffer = (uint8_t*)calloc(pixels*4, sizeof(uint8_t));
    uint8_t* paBuffer = (uint8_t*)calloc(pixels*2, sizeof(uint8_t));
    [image toPixelBuffer:rgbaBuffer flipped:NO];
    
    CNewPals* pals = self.c_scene->LevelPalette((UINT)level);
    self.layers->Select((UINT)frame, (UINT)level);
    
    const int width = (int)image.size.width;
    const int height = (int)image.size.height;
    for(int y = height - 1; y >= 0; --y) {
        for(int x = 0; x < width; ++x) {
            uint8_t* pAlpha = paBuffer + (height - y - 1)*width + x;
            uint8_t* pIndex = paBuffer + pixels + (height - y - 1)*width + x;
            uint8_t* pSrc = rgbaBuffer + y*width*4 + x*4;
            if(pSrc[3] > 0) {
                *pAlpha = 255; //pSrc[3];
            }
            *pIndex = [self findIndexOfColor:pSrc inPalette:pals];
        }
    }
    
    self.layers->UpdateLayer(paBuffer, pixels*2, (UINT)kind);
    self.layers->Put();
    self.c_io->Save();
    
    // Just for creating new thumbnail if it doesn't exist
    [self getThumbnailForFrame:frame level:level];
    
    free(rgbaBuffer);
    free(paBuffer);
}

- (uint8_t)findIndexOfColor:(uint8_t*)color inPalette:(CNewPals*)palette {
    uint8_t tmpColor[4];
    for(int i = 0; i < 256; ++i) {
        palette->Color(tmpColor, i);
        if(color[0] == tmpColor[0] && color[1] == tmpColor[1] && color[2] == tmpColor[2]) {
            return i;
        }
    }
    
//    if(color[0] != 0 && color[1] != 0 && color[2] != 0) {
//        NSLog(@"Color not found: [%d, %d, %d, %d]", (int)color[0], (int)color[1], (int)color[2], (int)color[3]);
//    }
    
    return 0;
}

- (void) writeIndexedLayerImageData:(NSData*)image sizeImage:(CGSize)size ofKind:(NSInteger)kind frame:(NSInteger)frame level:(NSInteger)level {
    const int width = (int)size.width;
    const int height = (int)size.height;
    const int length = (int)image.length;
    
    const int pixels = height * width;
    
    // rgba or bgra...
    // const int bytePerPixel = (int)((length) / pixels);
    const int bytePerPixel = 4;
    // alpha, index
    const int bytePerPixelIndex = 2;
    
    CNewPals* pals = self.c_scene->LevelPalette((UINT)level);
    self.layers->Select((UINT)frame, (UINT)level);
    
    uint8_t* pBuffer = (uint8_t*)image.bytes;
    uint8_t* pIndexBuffer = (uint8_t*)calloc(pixels, sizeof(uint8_t) * bytePerPixelIndex);
    memset(pIndexBuffer, 0, pixels * sizeof(uint8_t) * bytePerPixelIndex);
    
    const int offSetForAlphaBuffer = 0;
    // half buffer
    const int offSetForIndexBuffer = int(pixels * sizeof(uint8_t) * bytePerPixelIndex / 2);
    
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; ++x) {
            int positionBuffer = y * width + x;
            uint8_t* pSrc = pBuffer + positionBuffer * bytePerPixel;
            
            int positionIndexBuffer = (height - 1 - y) * width + x;
            uint8_t* pAlpha = pIndexBuffer + positionIndexBuffer;
            uint8_t* pIndex = pIndexBuffer + positionIndexBuffer + offSetForIndexBuffer;

            uint8_t a = pSrc[3];
            if(a > 0) {
                *pAlpha = 255;
            }
            *pIndex = [self findIndexOfColor:pSrc inPalette:pals isBGR:TRUE];
        }
    }
    
    self.layers->UpdateLayer(pIndexBuffer, pixels * sizeof(uint8_t) * bytePerPixelIndex, (UINT)kind);
    self.layers->Put();
    self.c_io->Save();
    
    // Just for creating new thumbnail if it doesn't exist
    [self getThumbnailForFrame:frame level:level];
    
    free(pIndexBuffer);
}

- (uint8_t)findIndexOfColor:(uint8_t*)color inPalette:(CNewPals*)palette isBGR:(BOOL)flag {
    uint8_t r_color, g_color, b_color, a_color;
    if(flag){
        // BGR
        r_color = color[2];
        g_color = color[1];
        b_color = color[0];
    }
    else {
        // RGB
        r_color = color[0];
        g_color = color[1];
        b_color = color[2];
    }
    a_color = color[3];
    bool isAlphaZero = a_color == 0;
    
    uint8_t r_palette, g_palette, b_palette, a_palette;
    for(int i = 0; i < 256; ++i) {
        r_palette = palette->Red(i);
        g_palette = palette->Green(i);
        b_palette = palette->Blue(i);
        a_palette = palette->Alpha(i);
        
        if(isAlphaZero) {
            if(r_color == r_palette && g_color == g_palette && b_color == b_palette)
                return i;
        } else {
            if(r_color == r_palette && g_color == g_palette && b_color == b_palette && a_color == a_palette)
                return i;
        }
    }
    
//    NSMutableArray *array_palete0 = [[NSMutableArray alloc]init];
//    for (int i = 0; i < 256; i++) {
//        r_palette = palette->Red(i);
//        g_palette = palette->Green(i);
//        b_palette = palette->Blue(i);
//        a_palette = palette->Alpha(i);
//        [array_palete0 addObject:@{ @"r" : @(r_palette), @"g" : @(g_palette), @"b" : @(b_palette), @"a" : @(a_palette)}];
//    }
    
//    if(color[0] != 0 && color[1] != 0 && color[2] != 0) {
//        NSLog(@"Color not found: [%d, %d, %d, %d]", (int)color[0], (int)color[1], (int)color[2], (int)color[3]);
//    }
    
//    return 0;
    int r = arc4random_uniform(256);
    return r;
}

- (void) updateCellByFrame:(UINT) Frame level:(UINT) Level {
    DWORD key = self.c_scene->BlowCell(Frame, Level);
    if (!key) return;
    DWORD cnt = self.c_scene->RefCount(key); // w/o clipboard
    self.c_scene->UpdateCache(Frame, Level);
    if (!cnt)
        return;
    UINT f, l;
    for (UINT f = 0; cnt && (f < self.frames); f++) {
        for (UINT l = 0; cnt && (l < self.levels); l++) {
            if ((f == Frame) && (l == Level))
                continue;
            if (self.c_scene->GetCellKey(f,l) == key) {
                self.c_scene->UpdateCache(f,l);
                cnt--;
            }
        }
    }
}

- (void) keepCellAtFrame:(NSInteger)row level: (NSInteger)column isNew:(BOOL)isNew {
    if (self.layers->Put(isNew)) {
        self.c_scene->BlowCell((UINT)row, (UINT)column);
        [self updateCellByFrame:(UINT)row level:(UINT)column];
    }
    self.c_scene->Modified(true);
}

#pragma mark - Cell / row operations

- (void)shiftCellsBackwardStartingFromRow:(NSInteger)fromRow {
    NSInteger endRow = fromRow;
    
    self.c_scene->ScnChangeFrames((UINT)fromRow + 1, (UINT)endRow);
    self.layers->Put();
    self.c_io->Save();
}

- (void)shiftCellsForwardStartingFromRow:(NSInteger)fromRow {
    NSInteger endRow = fromRow;
    
    self.c_scene->ScnChangeFrames((UINT)fromRow, (UINT)endRow + 1);
    self.layers->Put();
    self.c_io->Save();
}

- (void)truncateToRows:(NSInteger)inNumRows {
    
}

- (void)deleteRow:(NSInteger)row {
    NSInteger endRow = row;
    
    [self deleteSelection:row endRow:endRow];
    
    self.layers->Put();
    self.c_io->Save();
}

- (bool) deleteSelection:(NSInteger)startRow endRow: (NSInteger)endRow {
    UINT Frame, Level;
    for (UINT Level = 0; Level <= self.c_scene->LevelCount() - 1; Level++)
        for (UINT Frame = (UINT)startRow; Frame <= endRow; Frame++) {
            self.c_scene->ChangeCellKey(Frame, Level, true);
        }
    self.c_scene->UpdateLinks(0,0);//pKeys,kCount);
}

- (void) shiftCellsForwardStartingFromColumn:(NSInteger)fromColumn {
    self.c_scene->CheckInfoLevel();
    
//    self.c_scene->InsertLevel((UINT)fromColumn, 1, self.c_scene->GetThumbW());
//    self.c_scene->InsertLevel((UINT)fromColumn, 1, 9999);
    self.c_scene->InsertLevel((UINT)fromColumn, 1, 80);
    
    self.layers->Put();
    self.c_io->Save();
}

- (void) shiftCellsBackwardStartingFromColumn:(NSInteger)fromColumn {
    
}

- (void) truncateToColumns:(NSInteger)inNumColumns {
    
}

- (void) deleteColumn:(NSInteger)column {
    self.c_scene->CheckInfoLevel();
    
    self.c_scene->DeleteLevel((UINT)column, 1);
    
    self.layers->Put();
    self.c_io->Save();
}

#pragma mark - Cut, Copy, Paste Cell

- (void) cutCopyCellAtRow:(NSInteger)row column: (NSInteger)column isCopy:(BOOL)isCopy {
    if (!isCopy) {
        [self keepCellAtFrame:row level:column isNew:true];
    }
    
    self.c_scene->CutCopy((UINT)row, (UINT)column, 1, 1, isCopy);
}

- (void) pasteCellAtRow:(NSInteger)row column: (NSInteger)column {
    UINT * keys = new UINT[1];
    UINT c = 0;

    UINT newkey = self.c_scene->ClipGet(0, 0);
//    if (bBreak)
//    newkey = self.c_scene->DuplicateCell(newkey);
    UINT oldkey = self.c_scene->SwapCellKey((UINT)row, (UINT)column, newkey);
    if (oldkey)
        keys[c++] = oldkey;
    
    self.c_scene->UpdateLinks(keys, c);
    delete [] keys;
    
    self.layers->Put(true);
    self.c_io->Save();
    
//    UINT key = self.c_scene->GetCellKey((UINT)row, (UINT)column);
//    if (self.c_scene->IsLinked(key)) {
//        self.c_scene->DeleteCell((UINT)row, (UINT)column);
//        [self keepCellAtFrame:row level:column isNew:true];
//    }
//
//    self.layers->Put(true);
//    self.c_io->Save();
}

#pragma mark - Sound

- (NSString *)soundPath {
    char nameBuffer[300] = {0};
    self.c_scene->SceneOptionStr(SCOPT_WAVE0, nameBuffer);
    
    return [NSString stringWithUTF8String:nameBuffer];
}

- (NSData *)soundData {
    NSString* soundPath = [self soundPath];
    NSFileManager* fm = [NSFileManager defaultManager];
    if(![fm fileExistsAtPath:soundPath]) {
        NSRange range = [soundPath rangeOfString:@"/" options:NSBackwardsSearch];
        if(range.location == NSNotFound) {
            range = [soundPath rangeOfString:@"\\" options:NSBackwardsSearch];
        }
        if(range.location != NSNotFound) {
            NSString* name = [soundPath substringFromIndex:range.location + 1];
            NSLog(@"Sound file name: %@", name);
            NSString* dgcPath = [self.path stringByDeletingLastPathComponent];
            soundPath = [dgcPath stringByAppendingPathComponent:name];
            if([fm fileExistsAtPath:soundPath]) {
                return [NSData dataWithContentsOfFile:soundPath];
            }
        }
    } else {
        return [NSData dataWithContentsOfFile:soundPath];
    }
    
    return nil;
}

#pragma mark - Palettes

- (FBPalette*)paletteForLevel:(NSInteger)level {
    FBPalette* palette = [[FBPalette alloc] initWithScene:self.c_scene level:level];
    return palette;
}

#pragma mark -

- (void)dealloc {
    delete self.c_io;
    delete self.c_scene;
}

@end

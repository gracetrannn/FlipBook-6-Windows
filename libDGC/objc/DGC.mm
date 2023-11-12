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
        NSString* dgxPath = [inPath stringByReplacingCharactersInRange:NSMakeRange(inPath.length - 1, 1) withString:@"x"];
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
        
        // Initialize IO
        _c_io = new CMyIO();
        _c_io->Close(TRUE);
        // Open file
        result = _c_io->Open(fileName);
        // Create scene
        _c_scene = new CSceneBase(_c_io, std::make_unique<MacResourceLoader>());
        result = _c_scene->Read(FALSE);
        // File is open and ready for usage
        
        if(result != 0) {
            return nil;
        }
        
        self.layers = new CLayers();
        self.layers->Setup(_c_scene, true);
        
    }

    return self;
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

- (BOOL)levelUsed:(NSInteger)level {
    return self.c_scene->LevelFlags((UINT)level) & 0x1;
}

#pragma mark - Cell images

- (UIImage *) getThumbnail {
    UIImage* paintImage = [self getImageOfKind:/*CCell::LAYER_THUMB*/99 row:0 level:0];
    return paintImage;
}

- (UIImage *) overlayImageForFrame:(NSInteger)frame level:(NSInteger)level {
    return [self getLevelImageForFrame:frame level:level];
}

- (UIImage *) backgroundImageForFrame:(NSInteger)frame {
    UIImage* paintImage = [self getImageOfKind:CCell::LAYER_BG row:frame level:0];
    return paintImage;
}

- (UIImage *)imagePaintForFrame:(NSInteger)frame level:(NSInteger)level {
    return [self getLayerImageOfKind:CCell::LAYER_PAINT frame:frame level:level];
}

- (UIImage *)imagePencilForFrame:(NSInteger)frame level:(NSInteger)level {
    return [self getLayerImageOfKind:CCell::LAYER_INK frame:frame level:level];
}

- (UIImage*)getLevelImageForFrame:(NSInteger)frame level:(NSInteger)level {
    DWORD key;
    self.c_scene->GetImageKey(key, (UINT)frame, (UINT)level, 99);
    if(key == 0) {
        return nil;
    }
    
    DWORD w = self.c_scene->Width();
    DWORD h = self.c_scene->Height();
    
    CNewPals* pals = self.c_scene->LevelPalette((UINT)level);
    const int bpp = 4;
    uint8_t* imgBuffer = new uint8_t[bpp*w*h];
    memset(imgBuffer, 0, bpp*w*h);
    self.layers->Select((UINT)frame, (UINT)level);
    self.layers->m_nFlags = 3;
    self.layers->GetImage(imgBuffer, w*bpp);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate((void*)imgBuffer, w, h, 8, w * bpp, colorSpace, kCGImageAlphaPremultipliedLast);
    CGImageRef cgImage = CGBitmapContextCreateImage(ctx);
    UIImage* uiImage = [[UIImage alloc] initWithCGImage:cgImage scale:1 orientation:UIImageOrientationDownMirrored];
    delete [] imgBuffer;
    return uiImage;
}

- (UIImage*)getLayerImageOfKind:(UINT)kind frame:(NSInteger)frame level:(NSInteger)level {
    DWORD key;
    self.c_scene->GetImageKey(key, (UINT)frame, (UINT)level, 99);
    if(key == 0) {
        return nil;
    }
    
    DWORD w = self.c_scene->Width();
    DWORD h = self.c_scene->Height();
    
    CNewPals* pals = self.c_scene->LevelPalette((UINT)level);
    const int bpp = 4;
    uint8_t* imgBuffer = new uint8_t[bpp*w*h];
    memset(imgBuffer, 0, bpp*w*h);
    self.layers->Select((UINT)frame, (UINT)level);
    self.layers->m_nFlags = 3;
    self.layers->GetLayerImage(imgBuffer, kind);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate((void*)imgBuffer, w, h, 8, w * bpp, colorSpace, kCGImageAlphaPremultipliedLast);
    CGImageRef cgImage = CGBitmapContextCreateImage(ctx);
    UIImage* uiImage = [[UIImage alloc] initWithCGImage:cgImage]; //[[UIImage alloc] initWithCGImage:cgImage scale:1 orientation:UIImageOrientationDownMirrored];
    delete [] imgBuffer;
    return uiImage;
}

- (UIImage*)getImageOfKind:(UINT)kind row:(NSInteger)row level:(NSInteger)level
{
    int result = 0;
    DWORD key;
    result = _c_scene->GetImageKey(key, (UINT)row, (UINT)level, kind);
    if (key == 0) {
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
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *dataPath = [documentsDirectory stringByAppendingPathComponent:@"bg.dat"];
    NSData* data = [NSData dataWithBytes:pData length:dataLength];
    [data writeToFile:dataPath atomically:YES];
    NSLog(@"Data was written to %@", dataPath);
    
    UIImage* uiImage = [[UIImage alloc] initFromPixelBytes:(pData + offset) width:w height:h bitDepth:d];
    
    delete[] pData;
    return uiImage;
}

#pragma mark - Sound

- (NSData *)soundData {
    char nameBuffer[300] = {0};
    self.c_scene->SceneOptionStr(SCOPT_WAVE0, nameBuffer);
    
    NSString* soundPath = [NSString stringWithUTF8String:nameBuffer];
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

#pragma mark -

- (void)dealloc {
    delete self.c_io;
    delete self.c_scene;
}

@end

#include "MacResourceLoader.h"
#include <memory>

#import "NSBundle+DGC.h"


ResourcePtr MacResourceLoader::LoadImprint() {
    NSURL* url = [[NSBundle dgcBundle] URLForResource:@"thedisc" withExtension:@"XYZ"];
    NSData* data = [NSData dataWithContentsOfURL:url];
    ResourceHeader* pHeader = (ResourceHeader*)[data bytes];
    size_t bufferSize = sizeof(ResourceHeader) + pHeader->outSize;
    auto result = std::make_unique<unsigned char[]>(bufferSize);
    memcpy(result.get(), [data bytes], bufferSize);
    return result;
}

ResourcePtr MacResourceLoader::LoadDefaultPalette() {
    NSURL* url = [[NSBundle dgcBundle] URLForResource:@"INITPAL" withExtension:@"DAT"];
    NSData* data = [NSData dataWithContentsOfURL:url];
    auto result = std::make_unique<unsigned char[]>([data length]);
    memcpy(result.get(), [data bytes], [data length]);
    return result;
}

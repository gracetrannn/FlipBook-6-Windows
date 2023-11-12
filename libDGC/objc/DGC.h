//
//  FBDGCSceneDatabase.h
//  FlipPad
//
//  Created by Alex Vihlayew on 11/2/20.
//  Copyright © 2020 Alex. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface DGC: NSObject

- (id) initWithPath:(NSString *)inPath;

- (NSInteger) frames;
- (NSInteger) levels;
- (NSInteger) frameRate;
- (CGSize) frameSize;

- (UIImage *) getThumbnail;
- (UIImage *) overlayImageForFrame:(NSInteger)frame level:(NSInteger)level;
- (UIImage *) backgroundImageForFrame:(NSInteger)frame;
- (UIImage *) imagePencilForFrame:(NSInteger)frame level:(NSInteger)level;
- (UIImage *) imagePaintForFrame:(NSInteger)frame level:(NSInteger)level;

- (NSData*)soundData;

- (BOOL)levelUsed:(NSInteger)level;

@end

NS_ASSUME_NONNULL_END

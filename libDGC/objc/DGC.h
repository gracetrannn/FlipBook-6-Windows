//
//  FBDGCSceneDatabase.h
//  FlipPad
//
//  Created by Alex Vihlayew on 11/2/20.
//  Copyright © 2020 Alex. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "FBPalette.h"
#import "FBColor.h"

NS_ASSUME_NONNULL_BEGIN

@interface DGC: NSObject

- (id) initWithPath:(NSString *)inPath;

#pragma mark - Properties

- (NSInteger) levels;
- (NSInteger) frames;
- (NSInteger) frameRate;
- (CGSize) frameSize;
- (CGSize) frameComSize;

#pragma mark - Images

- (UIImage *) getThumbnail;

- (UIImage *)backgroundImageForFrame:(NSInteger)frame;

#pragma mark - Image data

// Get data

- (NSData *)paintImageDataForFrame:(NSInteger)frame level:(NSInteger)level convert: (bool)convert32 depth: (uint32_t*)depth;
- (NSData *)pencilImageDataForFrame:(NSInteger)frame level:(NSInteger)level convert: (bool)convert32 depth: (uint32_t*)depth;

- (NSData *)backgroundImageDataForFrame:(NSInteger)frame convert: (bool)convert32 depth: (uint32_t*)depth;

- (NSData *)compositeImageDataForFrame:(NSInteger)frame;

// Set data

- (void)setPaintImageData:(NSData *)data sizeImage:(CGSize)size frame:(NSInteger)frame level:(NSInteger)level;
- (void)setPencilImageData:(NSData *)data sizeImage:(CGSize)size frame:(NSInteger)frame level:(NSInteger)level;

- (void)setPencilImageData:(NSData *)data size:(CGSize)size depth:(NSInteger)depth frame:(NSInteger)frame level:(NSInteger)level;
- (void)setPaintImageData:(NSData *)data size:(CGSize)size depth:(NSInteger)depth frame:(NSInteger)frame level:(NSInteger)level;

#pragma mark - Cell / row operations

- (void)shiftCellsBackwardStartingFromRow:(NSInteger)fromRow;
- (void)shiftCellsForwardStartingFromRow:(NSInteger)fromRow;
- (void)truncateToRows:(NSInteger)inNumRows; // TODO: Implement
- (void)deleteRow:(NSInteger)row;

- (void) shiftCellsForwardStartingFromColumn:(NSInteger)fromColumn;
- (void) shiftCellsBackwardStartingFromColumn:(NSInteger)fromColumn;
- (void) truncateToColumns:(NSInteger)inNumColumns;
- (void) deleteColumn:(NSInteger)column;

#pragma mark - Cut, Copy, Paste Cell

- (void) keepCellAtFrame:(NSInteger)row level: (NSInteger)column isNew:(BOOL)isNew;

- (void) cutCopyCellAtRow:(NSInteger)row column: (NSInteger)column isCopy:(BOOL)isCopy;

- (void) pasteCellAtRow:(NSInteger)row column: (NSInteger)column;

#pragma mark - Levels

- (NSString *)levelNameAtIndex:(NSInteger)index;
- (void)setLevelName:(NSString*)name atIndex:(NSInteger)index;

- (BOOL)isLevelHiddenAtIndex:(NSInteger)index; // TODO: Implement
- (void)setLevelIsHidden:(BOOL)hidden atIndex:(NSInteger)index; // TODO: Implement

- (BOOL)isLevelLockedAtIndex:(NSInteger)index; // TODO: Implement
- (void)setLevelIsLocked:(BOOL)locked atIndex:(NSInteger)index; // TODO: Implement

- (NSInteger)getLevelWidth:(NSInteger)level twidth:(NSInteger)twidth;

//

- (NSString *)soundPath;

- (NSData*)soundData;
- (void)setSoundData:(NSData *)soundData; // TODO: Implement

- (CGFloat)soundOffset; // TODO: Implement
- (void)setSoundOffset:(CGFloat)offset; // TODO: Implement

- (BOOL)levelUsed:(NSInteger)level;

- (FBPalette*)paletteForLevel:(NSInteger)level;

@end

NS_ASSUME_NONNULL_END

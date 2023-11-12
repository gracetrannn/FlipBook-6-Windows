//
//  FBPalette.h
//  FlipPad
//
//  Created by Selim Mustafaev on 01.04.2021.
//  Copyright © 2021 Alex. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@class FBColor;

@interface FBPalette : NSObject

@property (strong, nonatomic) NSString* name;
@property (strong, nonatomic) NSMutableArray<FBColor*>* colors;

- (instancetype)initWithScene:(void*)cscene level:(NSInteger)level;

- (void)replaceColorAtIndex:(NSUInteger)index withColor:(FBColor*)newColor;

@end

NS_ASSUME_NONNULL_END

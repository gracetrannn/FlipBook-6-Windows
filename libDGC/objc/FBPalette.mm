//
//  FBPalette.m
//  FlipPad
//
//  Created by Selim Mustafaev on 01.04.2021.
//  Copyright © 2021 Alex. All rights reserved.
//

#import "FBPalette.h"
#import "FBColor.h"
#import "DGC.h"

#include "CSceneBase.h"
#include "CNewPals.h"

@interface FBPalette ()

@property (assign, nonatomic) CSceneBase* cscene;
@property (assign, nonatomic) CNewPals* cpals;

@end

@implementation FBPalette

- (instancetype)initWithScene:(void*)cscene level:(NSInteger)level
{
    self = [super init];
    if (self) {
        self.cscene = (CSceneBase*)cscene;
        self.cpals = self.cscene->LevelPalette((UINT)level);
        
        self.name = [NSString stringWithCString:self.cpals->GetPalName() encoding:NSUTF8StringEncoding];
        
        const int colorsCount = 256; // 8 bit palette
        NSMutableArray<FBColor*>* colors = [[NSMutableArray<FBColor*> alloc] initWithCapacity:colorsCount];
        for(int i = 0; i < colorsCount; ++i) {
            uint8_t colorData[4];
            self.cpals->Color(colorData, i);
            FBColor* color = [[FBColor alloc] initWithRed:colorData[0] green:colorData[1] blue:colorData[2] alpha:colorData[3]];
            [colors addObject:color];
        }
        self.colors = colors;
    }
    return self;
}

- (void)replaceColorAtIndex:(NSUInteger)index withColor:(FBColor*)newColor
{
    CGFloat r;
    CGFloat g;
    CGFloat b;
    CGFloat a;
    [[newColor uiColor] getRed:&r green:&g blue:&b alpha:&a];
    self.cpals->Assign(index, (BYTE)(r * 255.0),
                                (BYTE)(g * 255.0),
                                (BYTE)(b * 255.0),
                                (BYTE)(a * 255.0)); // CHECK: a or o ?
    [self.colors replaceObjectAtIndex:index withObject:newColor];
}

- (void)dealloc
{

}

@end

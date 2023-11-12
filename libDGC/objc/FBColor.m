//
//  MyClass.m
//  
//
//  Created by Alex Vihlayew on 10/28/21.
//

#import "FBColor.h"

@implementation FBColor

- (instancetype)initWithRed:(uint8_t)red green:(uint8_t)green blue:(uint8_t)blue alpha:(uint8_t)alpha
{
    self = [super init];
    if (self) {
        _red = red;
        _green = green;
        _blue = blue;
        _alpha = alpha;
    }
    return self;
}

- (instancetype)initWithUIColor:(UIColor*)uiColor
{
    self = [super init];
    if (self) {
        const CGFloat* components = CGColorGetComponents(uiColor.CGColor);
        _red = (uint8_t)(components[0] * 255.0);
        _green = (uint8_t)(components[1] * 255.0);
        _blue = (uint8_t)(components[2] * 255.0);
        _alpha = (uint8_t)(components[3] * 255.0);
    }
    return self;
}

- (UIColor*)uiColor
{
    return [[UIColor alloc] initWithRed:(CGFloat)_red / 255.0
                                  green:(CGFloat)_green / 255.0
                                   blue:(CGFloat)_blue / 255.0
                                  alpha:(CGFloat)_alpha / 255.0];
}

@end

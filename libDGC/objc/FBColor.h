//
//  MyClass.h
//  
//
//  Created by Alex Vihlayew on 10/28/21.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface FBColor : NSObject

@property (assign, nonatomic) uint8_t red;
@property (assign, nonatomic) uint8_t green;
@property (assign, nonatomic) uint8_t blue;
@property (assign, nonatomic) uint8_t alpha;

- (instancetype)initWithRed:(uint8_t)red green:(uint8_t)green blue:(uint8_t)blue alpha:(uint8_t)alpha;

- (instancetype)initWithUIColor:(UIColor*)uiColor;

- (UIColor*)uiColor;

@end

NS_ASSUME_NONNULL_END

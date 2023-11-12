//
//  MyClass.m
//  
//
//  Created by Selim Mustafaev on 16.03.2021.
//

#import "NSBundle+DGC.h"

@implementation NSBundle (DGC)

+ (NSBundle*)dgcBundle {
    NSURL* url = [[NSBundle mainBundle] URLForResource:@"libDGC_ObjcDGC" withExtension:@"bundle"];
    return [NSBundle bundleWithURL:url];
//    NSString* bundlePath = [NSString stringWithFormat:@"%@/libDGC_ObjcDGC.bundle", [[NSBundle mainBundle] bundlePath]];
//    return [NSBundle bundleWithPath:bundlePath];
}

@end

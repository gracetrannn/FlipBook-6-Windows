//
//  UIImage_Extras.m
//  FlipBook
//
//  Created by Manton Reece on 5/16/10.
//  Copyright 2010 DigiCel. All rights reserved.
//

#import "UIImage_Extras.h"
#import <objc/runtime.h>
#include <cmath>

struct FBPixel { uint8_t r, g, b, a; };

static char FB_ASSOCIATED_OBJECT_1_KEY = 0;

@implementation UIImage (Extras)

- (UIImage *) roundedWithInset:(float)inInset cornerRadius:(float)inCornerRadius
{
	return self;
}

- (UIImage *) imageByScalingToSize:(CGSize)inTargetSize
{
    CGSize imageSize = [self size];
    float width = imageSize.width;
    float height = imageSize.height;

    // scaleFactor will be the fraction that we'll
    // use to adjust the size. For example, if we shrink
    // an image by half, scaleFactor will be 0.5. the
    // scaledWidth and scaledHeight will be the original,
    // multiplied by the scaleFactor.
    //
    // IMPORTANT: the "targetHeight" is the size of the space
    // we're drawing into. The "scaledHeight" is the height that
    // the image actually is drawn at, once we take into
    // account the ideal of maintaining proportions

    float scaleFactor = 0.0; 
    float scaledWidth = inTargetSize.width;
    float scaledHeight = inTargetSize.height;

    CGPoint thumbnailPoint = CGPointMake(0,0);

    // since not all images are square, we want to scale
    // proportionately. To do this, we find the longest
    // edge and use that as a guide.

    if ( CGSizeEqualToSize(imageSize, inTargetSize) == NO )
    { 
        // use the longeset edge as a guide. if the
        // image is wider than tall, we'll figure out
        // the scale factor by dividing it by the
        // intended width. Otherwise, we'll use the
        // height.

        float widthFactor = inTargetSize.width / width;
        float heightFactor = inTargetSize.height / height;

        if ( widthFactor < heightFactor )
                scaleFactor = widthFactor;
        else
                scaleFactor = heightFactor;

        // ex: 500 * 0.5 = 250 (newWidth)

        scaledWidth = width * scaleFactor;
        scaledHeight = height * scaleFactor;

        // center the thumbnail in the frame. if
        // wider than tall, we need to adjust the
        // vertical drawing point (y axis)

        if ( widthFactor < heightFactor )
                thumbnailPoint.y = (inTargetSize.height - scaledHeight) * 0.5;

        else if ( widthFactor > heightFactor )
                thumbnailPoint.x = (inTargetSize.width - scaledWidth) * 0.5;
    }


    CGContextRef mainViewContentContext;
    CGColorSpaceRef colorSpace;

    colorSpace = CGColorSpaceCreateDeviceRGB();

    // create a bitmap graphics context the size of the image
    mainViewContentContext = CGBitmapContextCreate (NULL, inTargetSize.width, inTargetSize.height, 8, 0, colorSpace, (CGBitmapInfo)kCGImageAlphaPremultipliedLast);

    // free the rgb colorspace
    CGColorSpaceRelease(colorSpace);    

    if (mainViewContentContext==NULL)
        return NULL;

    //CGContextSetFillColorWithColor(mainViewContentContext, [[UIColor whiteColor] CGColor]);
    //CGContextFillRect(mainViewContentContext, CGRectMake(0, 0, targetSize.width, targetSize.height));

    CGContextDrawImage(mainViewContentContext, CGRectMake(thumbnailPoint.x, thumbnailPoint.y, scaledWidth, scaledHeight), self.CGImage);

    // Create CGImageRef of the main view bitmap content, and then
    // release that bitmap context
    CGImageRef cg_img = CGBitmapContextCreateImage(mainViewContentContext);

	CGContextRelease(mainViewContentContext);

    // convert the finished resized image to a UIImage 
    UIImage* new_img = [UIImage imageWithCGImage:cg_img];
	
	CGImageRelease (cg_img);

    return new_img;
}

- (UIImage *) rf_imageByChangingAlpha:(CGFloat)inAlpha
{
	CGRect r = CGRectMake (0, 0, [self size].width, [self size].height);
    UIGraphicsBeginImageContextWithOptions(r.size, NO, 1);
	CGContextRef context = UIGraphicsGetCurrentContext();
	
	CGContextSaveGState (context);
	CGContextTranslateCTM (context, 0, r.size.height);
	CGContextScaleCTM (context, 1.0, -1.0);
	CGContextSetAlpha (context, inAlpha);
	CGContextDrawImage (context, r, self.CGImage);
	CGContextRestoreGState (context);
	
	UIImage* new_img = UIGraphicsGetImageFromCurrentImageContext();
	UIGraphicsEndImageContext();
	return new_img;
}

- (UIImage *)rf_imageByAddingImage:(UIImage*)image atRect:(CGRect)rect
{
    CGRect first_r = CGRectMake (0, 0, self.size.width, self.size.height);
    
    UIGraphicsBeginImageContextWithOptions(first_r.size, NO, 1);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSaveGState(context);
    
    CGContextSetBlendMode(context, kCGBlendModeNormal);
    CGContextSetInterpolationQuality(context, kCGInterpolationHigh);
    CGContextSetAllowsAntialiasing(context, YES);
    CGContextSetShouldAntialias(context, YES);
    
    [self drawInRect:first_r];
    [image drawInRect:CGRectMake(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height)];
    
    UIImage* img = UIGraphicsGetImageFromCurrentImageContext();
    CGContextRestoreGState(context);
    UIGraphicsEndImageContext();
    
    return img;
}

+ (UIImage *) rf_imageByCompositingImages:(NSArray *)inImages backgroundColor:(UIColor *)inColor
{
	if ([inImages count] == 0) {
		return nil;
	}
	
	CGImageRef first_img = ((UIImage *)[inImages objectAtIndex:0]).CGImage;
	if (first_img == nil) {
		return nil;
	}
	
	CGRect first_r = CGRectMake (0, 0, CGImageGetWidth(first_img), CGImageGetHeight(first_img));

    UIGraphicsBeginImageContextWithOptions(first_r.size, NO, 1);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSaveGState(context);
    
    CGContextSetBlendMode(context, kCGBlendModeNormal);
    CGContextSetInterpolationQuality(context, kCGInterpolationHigh);
    CGContextSetAllowsAntialiasing(context, YES);
    CGContextSetShouldAntialias(context, YES);
    
	if (inColor) {
        UIImage* colorImage = [UIImage rf_imageWithSize:first_r.size fillColor:inColor];
        [colorImage drawInRect:first_r];
	}
    
	for (UIImage* img in inImages) {
		CGFloat use_alpha = [img fb_associatedAlpha];
		if (use_alpha == 0.0) {
            [img drawInRect:first_r];
		} else {
            [img drawInRect:first_r blendMode:kCGBlendModeNormal alpha:use_alpha];
		}
	}
	
    UIImage* img = UIGraphicsGetImageFromCurrentImageContext();
    CGContextRestoreGState(context);
    UIGraphicsEndImageContext();
    
	return img;
}

+ (UIImage *) rf_imageWithCGImage:(CGImageRef)inImage allowReturningNil:(BOOL)inReturnNil
{
	if ((inImage == NULL) && inReturnNil) {
		return nil;
	}
	else {
		return [self imageWithCGImage:inImage];
	}
}

+ (UIImage *) rf_imageWithSize:(CGSize)inSize fillColor:(UIColor *)inColor
{
	CGRect r = CGRectMake (0, 0, inSize.width, inSize.height);
    UIGraphicsBeginImageContextWithOptions(inSize, NO, 1);
    
	if (inColor) {
		[inColor setFill];
		UIRectFill (r);
	}
	
	UIImage* img = UIGraphicsGetImageFromCurrentImageContext();
	UIGraphicsEndImageContext();
	return img;
}

- (void) fb_setAssociatedAlpha:(CGFloat)inAlpha
{
	NSNumber* val = [NSNumber numberWithFloat:inAlpha];
	objc_setAssociatedObject (self, &FB_ASSOCIATED_OBJECT_1_KEY, val, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (CGFloat) fb_associatedAlpha
{
	NSNumber* val = objc_getAssociatedObject (self, &FB_ASSOCIATED_OBJECT_1_KEY);
	return [val floatValue];
}

- (UIImage *)imageByTintColor:(UIColor *)color
{
    UIGraphicsBeginImageContextWithOptions(self.size, NO, self.scale);
    CGRect rect = CGRectMake(0, 0, self.size.width, self.size.height);
    [color set];
    UIRectFill(rect);
    [self drawAtPoint:CGPointMake(0, 0) blendMode:kCGBlendModeDestinationIn alpha:1];
    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return newImage;
}

//public struct PixelData {
//    var a: UInt8
//    var r: UInt8
//    var g: UInt8
//    var b: UInt8
//}
//
//convenience init?(pixels: [PixelData], width: Int, height: Int) {
//    guard width > 0 && height > 0, pixels.count == width * height else { return nil }
//    var data = pixels
//    guard let providerRef = CGDataProvider(data: Data(bytes: &data, count: data.count * MemoryLayout<PixelData>.size) as CFData)
//        else { return nil }
//    guard let cgim = CGImage(
//        width: width,
//        height: height,
//        bitsPerComponent: 8,
//        bitsPerPixel: 32,
//        bytesPerRow: width * MemoryLayout<PixelData>.size,
//        space: CGColorSpaceCreateDeviceRGB(),
//        bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedFirst.rawValue),
//        provider: providerRef,
//        decode: nil,
//        shouldInterpolate: true,
//        intent: .defaultIntent)
//    else { return nil }
//    self.init(cgImage: cgim)
//}

- (instancetype)initFromPixelBytes:(unsigned char *)bytes width:(NSInteger)width height:(NSInteger)height bitDepth:(NSInteger)bitDepth
{
    NSInteger bytesPerPixel = 4;
    NSInteger pixelCount = width * height;
    NSInteger dataCapacity = bytesPerPixel * pixelCount;
    NSInteger originalStride =  4 * ((bitDepth * width + 31) / 32);
    NSInteger stride = width*bytesPerPixel;
    int originalBpp = (int)bitDepth/8;
    
    uint8_t* imgBuffer = new uint8_t[dataCapacity];
    for(NSInteger y = height - 1; y >= 0; --y) {
        for(NSInteger x = 0; x < width; ++x) {
            uint8_t* pOutPixel = imgBuffer + y*stride + x*bytesPerPixel;
            uint8_t* pInPixel = bytes + (height - y - 1)*originalStride + x*originalBpp;
            
            pOutPixel[0] = pInPixel[2];
            pOutPixel[1] = pInPixel[1];
            pOutPixel[2] = pInPixel[0];
            pOutPixel[3] = 255; // Opacity
        }
    }
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(imgBuffer, width, height, 8, width * bytesPerPixel, colorSpace, kCGImageAlphaPremultipliedLast);
    CGImageRef cgImage = CGBitmapContextCreateImage(ctx);
    
    delete[] imgBuffer;
    
    self = [self initWithCGImage:cgImage];
    
    CGImageRelease(cgImage);
    CGContextRelease(ctx);
    
    return self;
}

- (instancetype)initFromRgbaBuffer:(uint8_t*)buffer width:(NSInteger)width height:(NSInteger)height premultiplyAlpha:(BOOL)premultiply flipped:(BOOL)flipped {
    
    if(premultiply) {
        size_t pixels = width*height;
        for(size_t i = 0; i < pixels; ++i) {
            uint8_t* pixel = buffer + i*4;
            CGFloat alpha = pixel[3]/255.0;
            if(alpha > 0) {
                pixel[0] = (uint8_t)std::round(pixel[0]*alpha);
                pixel[1] = (uint8_t)std::round(pixel[1]*alpha);
                pixel[2] = (uint8_t)std::round(pixel[2]*alpha);
            }
        }
    }
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(buffer, width, height, 8, width*4, colorSpace, kCGImageAlphaPremultipliedLast);
    if(flipped) {
        CGContextTranslateCTM(ctx, 0, height);
        CGContextScaleCTM(ctx, 1.0, -1.0);
    }
    CGImageRef cgImage = CGBitmapContextCreateImage(ctx);
    
    self = [self initWithCGImage:cgImage];
    
    CGImageRelease(cgImage);
    CGContextRelease(ctx);
    
    return self;
}

- (void)toPixelBuffer:(uint8_t*)buffer flipped:(BOOL)flipped {
    CGImageRef imageRef = [self CGImage];
    NSUInteger width = CGImageGetWidth(imageRef);
    NSUInteger height = CGImageGetHeight(imageRef);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = 8;
    CGContextRef context = CGBitmapContextCreate(buffer, width, height,
                                                 bitsPerComponent, bytesPerRow, colorSpace,
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    if(flipped) {
        CGContextTranslateCTM(context, 0, height);
        CGContextScaleCTM(context, 1.0, -1.0);
    }
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
    CGContextRelease(context);
    
    for(int i = 0; i < height*width; ++i) {
        uint8_t* pixel = buffer + i*bytesPerPixel;
        CGFloat alpha = pixel[3]/255.0;
        if(alpha > 0) {
            pixel[0] = (uint8_t)std::round(pixel[0]/alpha);
            pixel[1] = (uint8_t)std::round(pixel[1]/alpha);
            pixel[2] = (uint8_t)std::round(pixel[2]/alpha);
        }
    }
}

@end

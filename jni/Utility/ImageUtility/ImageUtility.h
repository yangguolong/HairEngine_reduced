//
//  ImageUtility.h
//  Demo
//
//  Created by yangguolong on 13-7-9.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef ImageUtility_H
#define ImageUtility_H

#include <opencv2/opencv.hpp>

unsigned char * getImageDataAndInfoForImagePath(const char * filePath, int * theWidth, int * theHeight);

unsigned char * getPngImageDataAndInfoForImagePath(const char * filePath, int * theWidth, int * theHeight, int returnFormatType);

unsigned char * getJpgImageDataAndInfoForImagePath(const char * filePath, int * theWidth, int * theHeight, int returnFormatType);

unsigned char * getImageDataAndInfoFromJpgImages(const char * rgbImageFilePath, const char * alphaImageFilePath, int * theWidth, int * theHeight);

bool saveImageDataToFile(const unsigned char * imageData, int width, int height, const char * filePath);

bool saveJpgImageDataToFile(const unsigned char * imageData, int width, int height, const char * filePath);

bool flipImageData(unsigned char * imageData, int width, int height);

bool scaleImageData(unsigned char * sourceData, int sourceWidth, int sourceHeight, unsigned char * targetData, int targetWidth, int targetHeight);

#endif

//
//  ImageUtility.cpp
//  Demo
//
//  Created by yangguolong on 13-7-9.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "ImageUtility.h"
#include <stdio.h>
#include <stdlib.h>

unsigned char * getImageDataAndInfoForImagePath(const char * filePath, int * theWidth, int * theHeight)
{
    int nChannels = 4;
    IplImage * img = cvLoadImage(filePath, CV_LOAD_IMAGE_UNCHANGED);
    if(img == NULL || img->imageData == NULL)
    {
        printf("Failed to get image data and info %s\n", filePath);
        return NULL;
    }
    int width = img->width;
    int height = img->height;
    unsigned char * imageData = new unsigned char[width * height * nChannels];
    if(!imageData)
    {
        printf("Failed to alloc memory for data array when getting image data.\n");
        cvReleaseImage(&img);
        return NULL;
    }
    
    int widthStep = img->widthStep;
    unsigned char * data = (unsigned char *)img->imageData;
    int cvIndex;
    int arrayIndex = 0;
    for(int i=0; i<height; i++)
        for(int j=0; j<width; j++)
        {
            cvIndex = i * widthStep + j * nChannels;
            imageData[arrayIndex] = data[cvIndex+2];
            imageData[arrayIndex+1] = data[cvIndex+1];
            imageData[arrayIndex+2] = data[cvIndex];
            imageData[arrayIndex+3] = data[cvIndex+3];
            
            arrayIndex += nChannels;
        }
    cvReleaseImage(&img);
    
    if(theWidth != NULL)
        (*theWidth) = width;
    if(theHeight != NULL)
        (*theHeight) = height;
    
    return imageData;
}

unsigned char * getPngImageDataAndInfoForImagePath(const char * filePath, int * theWidth, int * theHeight, int returnFormatType)
{
    //returnFormatType  1: RGBA per pixel   2: continue R channnel buffer and then G, B , no alpha data
    unsigned char * imageData = NULL;
    if(returnFormatType == 1)
    {
        imageData = getImageDataAndInfoForImagePath(filePath, theWidth, theHeight);
    }
    else if(returnFormatType == 2)
    {
        int nChannels = 4;
        IplImage * img = cvLoadImage(filePath, CV_LOAD_IMAGE_UNCHANGED);
        if(img == NULL || img->imageData == NULL)
        {
            printf("Failed to get image data and info %s\n", filePath);
            return NULL;
        }
        int width = img->width;
        int height = img->height;
        int totalPixelNum = width * height;
        imageData = new unsigned char[totalPixelNum * 3];  // no alpha channel
        if(!imageData)
        {
            printf("Failed to alloc memory for data array when getting image data.\n");
            cvReleaseImage(&img);
            return NULL;
        }
        
        int widthStep = img->widthStep;
        unsigned char * data = (unsigned char *)img->imageData;
        int cvIndex;
        int arrayIndex;
        
        int greenOffset = totalPixelNum;
        int blueOffset = totalPixelNum + totalPixelNum;
        
        arrayIndex = 0;
        for(int i=0; i<height; i++)
            for(int j=0; j<width; j++)
            {
                cvIndex = i * widthStep + j * nChannels;
                imageData[arrayIndex] = data[cvIndex+2];                //red
                imageData[arrayIndex + greenOffset] = data[cvIndex+1];  //green
                imageData[arrayIndex+ blueOffset] = data[cvIndex];      //blue
                
                arrayIndex++;
            }
        cvReleaseImage(&img);
        
        if(theWidth != NULL)
            (*theWidth) = width;
        if(theHeight != NULL)
            (*theHeight) = height;
    }
    
    return imageData;
}

unsigned char * getJpgImageDataAndInfoForImagePath(const char * filePath, int * theWidth, int * theHeight, int returnFormatType)
{
    //returnFormatType  1: RGB per pixel   2: continue R channnel buffer and then G, B
    int nChannels = 3;
    IplImage * img = cvLoadImage(filePath, CV_LOAD_IMAGE_UNCHANGED);
    if(img == NULL || img->imageData == NULL)
    {
        printf("Failed to get image data and info %s\n", filePath);
        return NULL;
    }
    int width = img->width;
    int height = img->height;
    int totalPixelNum = width * height;
    unsigned char * imageData = new unsigned char[totalPixelNum *nChannels];
    if(!imageData)
    {
        printf("Failed to alloc memory for data array when getting image data.\n");
        cvReleaseImage(&img);
        return NULL;
    }
    
    int widthStep = img->widthStep;
    unsigned char * data = (unsigned char *)img->imageData;
    int cvIndex;
    int arrayIndex;
    
    if(returnFormatType == 1)
    {
        arrayIndex = 0;
        for(int i=0; i<height; i++)
            for(int j=0; j<width; j++)
            {
                cvIndex = i * widthStep + j * nChannels;
                imageData[arrayIndex] = data[cvIndex+2];
                imageData[arrayIndex+1] = data[cvIndex+1];
                imageData[arrayIndex+2] = data[cvIndex];
                
                arrayIndex += nChannels;
            }
    }
    else if(returnFormatType == 2)
    {
        int greenOffset = totalPixelNum;
        int blueOffset = totalPixelNum + totalPixelNum;
        
        arrayIndex = 0;
        for(int i=0; i<height; i++)
            for(int j=0; j<width; j++)
            {
                cvIndex = i * widthStep + j * nChannels;
                imageData[arrayIndex] = data[cvIndex+2];    //red
                imageData[arrayIndex + greenOffset] = data[cvIndex+1];  // green
                imageData[arrayIndex + blueOffset] = data[cvIndex];    //blue
            
                arrayIndex++;
            }
    }
    
    cvReleaseImage(&img);
    
    if(theWidth != NULL)
        (*theWidth) = width;
    if(theHeight != NULL)
        (*theHeight) = height;
    
    return imageData;
}

unsigned char * getImageDataAndInfoFromJpgImages(const char * rgbImageFilePath, const char * alphaImageFilePath, int * theWidth, int * theHeight)
{
    int widthRGB, heightRGB, widthA, heightA;
    
    unsigned char * rgbData = getJpgImageDataAndInfoForImagePath(rgbImageFilePath, &widthRGB, &heightRGB, 1);
    if(rgbData == NULL)
        return NULL;
    
    unsigned char * alphaData = getJpgImageDataAndInfoForImagePath(alphaImageFilePath, &widthA, &heightA, 1);
    if(alphaData == NULL)
    {
        delete [] alphaData;
        return NULL;
    }
    
    if(widthRGB != widthA || heightRGB != heightA)
    {
        delete [] rgbData;
        delete [] alphaData;
        return NULL;
    }
    
    if(theWidth != NULL)
        (*theWidth) = widthRGB;
    if(theHeight != NULL)
        (*theHeight) = heightRGB;
    
    int totalBytes = widthRGB * heightRGB * 4;
    unsigned char * imageData = new unsigned char[totalBytes];
    int pngIndex = 0;
    int jpgIndex = 0;
    for(; pngIndex < totalBytes;)
    {
        imageData[pngIndex] = rgbData[jpgIndex];
        imageData[pngIndex+1] = rgbData[jpgIndex+1];
        imageData[pngIndex+2] = rgbData[jpgIndex+2];
        imageData[pngIndex+3] = alphaData[jpgIndex];
        pngIndex+=4;
        jpgIndex+=3;
    }
    
    delete [] rgbData;
    delete [] alphaData;
    
    return imageData;
}

bool saveImageDataToFile(const unsigned char * imageData, int width, int height, const char * filePath)
{
    int nChannels = 4;
    
    IplImage * img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, nChannels);
    if(!img->imageData)
    {
        printf("Failed to alloc memory for img when creating texture file.\n");
        return false;
    }
    int widthStep = img->widthStep;
    unsigned char * data = (unsigned char *)img->imageData;
    int cvIndex;
    int arrayIndex = 0;
    for(int i=0; i<height; i++)
        for(int j=0; j<width; j++)
        {
            cvIndex = i * widthStep + j * nChannels;
            data[cvIndex] = imageData[arrayIndex+2];
            data[cvIndex+1] = imageData[arrayIndex+1];
            data[cvIndex+2] = imageData[arrayIndex];
            data[cvIndex+3] = imageData[arrayIndex+3];
            
            arrayIndex += nChannels;
        }
    
    int tag = cvSaveImage(filePath, img);
    cvReleaseImage(&img);
    
    if(tag != 1)
    {
        printf("Failed to save img.\n");
        return false;
    }
    return true;
}

bool saveJpgImageDataToFile(const unsigned char * imageData, int width, int height, const char * filePath)
{
    int nChannels = 3;
    
    IplImage * img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, nChannels);
    if(!img->imageData)
    {
        printf("Failed to alloc memory for img when creating texture file.\n");
        return false;
    }
    int widthStep = img->widthStep;
    unsigned char * data = (unsigned char *)img->imageData;
    int cvIndex;
    int arrayIndex = 0;
    for(int i=0; i<height; i++)
        for(int j=0; j<width; j++)
        {
            cvIndex = i * widthStep + j * nChannels;
            data[cvIndex] = imageData[arrayIndex+2];
            data[cvIndex+1] = imageData[arrayIndex+1];
            data[cvIndex+2] = imageData[arrayIndex];
            
            arrayIndex += nChannels;
        }
    
    int tag = cvSaveImage(filePath, img);
    cvReleaseImage(&img);
    
    if(tag != 1)
    {
        printf("Failed to save img.\n");
        return false;
    }
    return true;
}

bool flipImageData(unsigned char * imageData, int width, int height)
{
    int nChannels = 4;
    int totalPixelBytes = width * height * nChannels;
    int widthStep = width * nChannels;
    
    unsigned char * tmpData = new unsigned char[totalPixelBytes];
    if(!tmpData)
    {
        printf("Failed to alloc memory for tmpData when flipping image.\n");
        return false;
    }
    memcpy(tmpData, imageData, totalPixelBytes);
    
    int tmpArrayIndex, arrayIndex;
    for(int i=0; i<height; i++)
    {
        tmpArrayIndex = i * widthStep;
        arrayIndex = (height - 1 - i) * widthStep;
        memcpy(imageData+arrayIndex, tmpData+tmpArrayIndex, widthStep);
    }
    
    delete [] tmpData;
    
    return true;
}

bool scaleImageData(unsigned char * sourceData, int sourceWidth, int sourceHeight, unsigned char * targetData, int targetWidth, int targetHeight)
{
    int nChannels = 4;
    IplImage * srcImg = cvCreateImage(cvSize(sourceWidth, sourceHeight), IPL_DEPTH_8U, nChannels);
    IplImage * targetImg = cvCreateImage(cvSize(targetWidth, targetHeight), IPL_DEPTH_8U, nChannels);
    if(!srcImg->imageData || !targetImg->imageData)
    {
        printf("Failed to create srcImg or targetImg when scaling image.\n");
        return false;
    }
    int widthStep = srcImg->widthStep;
    unsigned char * data = (unsigned char *)srcImg->imageData;
    int imgIndex;
    int arrayIndex = 0;
    for(int i=0; i<sourceHeight; i++)
        for(int j=0; j<sourceWidth; j++)
        {
            imgIndex = i * widthStep + j * nChannels;
            data[imgIndex] = sourceData[arrayIndex+2];
            data[imgIndex+1] = sourceData[arrayIndex+1];
            data[imgIndex+2] = sourceData[arrayIndex];
            data[imgIndex+3] = sourceData[arrayIndex+3];
            
            arrayIndex += nChannels;
        }
    
    cvResize(srcImg, targetImg, CV_INTER_LINEAR);
    
    widthStep = targetImg->widthStep;
    data = (unsigned char *)targetImg->imageData;
    arrayIndex = 0;
    for(int i=0; i<targetHeight; i++)
        for(int j=0; j<targetWidth; j++)
        {
            imgIndex = i * widthStep + j * nChannels;
            targetData[arrayIndex] = data[imgIndex+2];
            targetData[arrayIndex+1] = data[imgIndex+1];
            targetData[arrayIndex+2] = data[imgIndex];
            targetData[arrayIndex+3] = data[imgIndex+3];
            
            arrayIndex += nChannels;
        }
    
    cvReleaseImage(&srcImg);
    cvReleaseImage(&targetImg);
    
    return true;
}
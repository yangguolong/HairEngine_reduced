//
//  FaceDetector.cpp
//  Test
//
//  Created by yangguolong on 13-6-21.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "HeadDetector.h"
#include <stdio.h>
#include <stdlib.h>

static const int iEnd76ToMid76[] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
	27, 68, 28, 69, 29, 70, 30, 71, 31,
	32, 72, 33, 73, 34, 74, 35, 75, 36,
	37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
	53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
    76
};

HeadDetector::HeadDetector(const char * dataFileDir, bool needInit)
{
    m_originalImage = NULL;
    if(dataFileDir)
    {
        int length = strlen(dataFileDir);
        if(length > 0)
        {
            strcpy(m_dataFileDir, dataFileDir);
            if(needInit)
                stasm_init(m_dataFileDir, 0);
        }
        else
            printf("invaild originalImage in head detector HeadDetector\n");
    }
    else
        printf("invaild originalImage in head detector HeadDetector\n");
}

HeadDetector::~HeadDetector()
{
    if(m_features.data)
        m_features.release();
    if(m_originalImage)
        cvReleaseImage(&m_originalImage);
    m_dataFileDir[0] = '\0';
}

void HeadDetector::setOriginalImage(IplImage * originalImage)
{
    if(!originalImage)
    {
        printf("invaild originalImage in head detector setOriginalImage\n");
        return;
    }
    m_originalImage = cvCloneImage(originalImage);
}

cv::Mat& HeadDetector::getResultFeatures()
{
    return m_features;
}

bool HeadDetector::runDetect()
{
    //CFAbsoluteTime startTime, endTime;

    cv::Mat_<unsigned char> imgMat = transformToGrayImage(m_originalImage);
    if (!imgMat.data)
    {
        printf("Cannot load image file\n");
        imgMat.release();
        return false;
    }
    
    int * featureArray = new int[OldLandmarksNum * 2];
    int foundface;
    float landmarks[2 * stasm_NLANDMARKS]; // x,y coords
    //startTime =  CFAbsoluteTimeGetCurrent();
    if(!stasm_search_single(&foundface, landmarks, (const char *)imgMat.data, imgMat.cols, imgMat.rows, m_dataFileDir, m_dataFileDir))
    {
        printf("Error in stasm_search_auto\n");
        return false;
    }
    //endTime = CFAbsoluteTimeGetCurrent();
    //printf("        asm: %lf s\n", endTime - startTime);
    
    if (!foundface)
    {
        printf("No face found.");
        return false;
    }
    else
    {
        stasm_convert_shape(landmarks, OldLandmarksNum);
        for (int i = 0; i < OldLandmarksNum; i++)
        {
            featureArray[(i<<1)]   = cvRound(landmarks[(i<<1)]);
            featureArray[(i<<1)+1] = cvRound(landmarks[(i<<1)+1]);
        }
    }
    
    if(m_features.data)
        m_features.release();
    m_features = cv::Mat::zeros(1, OldLandmarksNum, CV_32FC2);
    for (int featureI = 0; featureI < OldLandmarksNum; featureI++)
        m_features.at<cv::Vec2f>(0, featureI) = cv::Vec2f(
                                                          featureArray[iEnd76ToMid76[featureI] * 2],
                                                          featureArray[iEnd76ToMid76[featureI] * 2 + 1]);
    delete [] featureArray;
    
    return true;
}

cv::Mat_<unsigned char> HeadDetector::transformToGrayImage(IplImage * img)
{
    int width     = img->width;
    int height    = img->height;
    int nchannels = img->nChannels;
    int step      = img->widthStep;
    
    IplImage* img2 = cvCreateImage(cvSize(img->width, img->height),IPL_DEPTH_8U,1);
    
    /* setup the pointer to access image data */
    uchar *data = ( uchar* )img->imageData;
    uchar *data2= ( uchar* )img2->imageData;
    
    /* convert to grayscale manually */
    int i, j, r, g, b, byte;
    for( i = 0 ; i < height ; i++ )
    {
        for( j = 0 ; j < width ; j++ )
        {
            b = data[i*step + j*nchannels + 0];
            g = data[i*step + j*nchannels + 1];
            r = data[i*step + j*nchannels + 2];
        
            byte = (int)(r * 0.21f + g * 0.72f + b * 0.07f);
            
            data2[i*(img2->widthStep)+j*(img2->nChannels)+0] = byte;
        }
    }
    
    cv::Mat tmpImg(img2, true);
    cv::Mat_<unsigned char> imgMat = (cv::Mat_<unsigned char>)tmpImg;
    cvReleaseImage(&img2);
    
    return imgMat;
}
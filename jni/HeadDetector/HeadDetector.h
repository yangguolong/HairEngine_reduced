//
//  FaceDetector.h
//  Test
//
//  Created by yangguolong on 13-6-21.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef HeadDetector_H
#define HeadDetector_H

#include "stasm_lib.h"
#include <opencv2/opencv.hpp>

#define OldLandmarksNum 76

class HeadDetector
{
public:
    HeadDetector(const char * dataFileDir, bool needInit);
    ~HeadDetector();
    
    void setOriginalImage(IplImage * originalImage);
    bool runDetect();
    cv::Mat& getResultFeatures();
    
private:
    cv::Mat_<unsigned char> transformToGrayImage(IplImage * inputImage);
    
    char        m_dataFileDir[500];
    IplImage *  m_originalImage;
    cv::Mat     m_features;
};

#endif

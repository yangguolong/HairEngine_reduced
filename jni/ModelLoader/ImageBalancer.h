//
//  ImageBalancer.h
//  Demo
//
//  Created by yangguolong on 13-8-9.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef ImageBalancer_H
#define ImageBalancer_H

#include "opencv2/opencv.hpp"

class ImageBalancer
{
public:
    ImageBalancer();
    ~ImageBalancer();
    
    void initBalancer(const unsigned char * srcImageData, int width, int height);
    void runBalance(const unsigned char * hairFaceImageData, int faceImageWidth, int faceImageHeight, unsigned char * hairTextureData, int hairTextureWidth, int hairTextureHeight);
    void runBalance(const float faceImageLuminance, unsigned char * hairTextureData, int hairTextureWidth, int hairTextureHeight);
    
private:
    float luminanceMean(cv::Mat& image);
    void  balanceHairTexture(const float luminanceDisp, unsigned char * textureData, int width, int height);
    cv::Vec4f rgb2XYZ(cv::Vec4f rgb);
    
    float m_LuminanceSrc;
};

#endif

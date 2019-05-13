//
//  ImageBalancer.cpp
//  Demo
//
//  Created by yangguolong on 13-8-9.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "ImageBalancer.h"

#define IBMIN(a,b)  ((a) > (b) ? (b) : (a))
#define IBMAX(a,b)  ((a) < (b) ? (b) : (a))

#define nPixelChannels 4

ImageBalancer::ImageBalancer()
{
    m_LuminanceSrc = 0.0f;
}

ImageBalancer::~ImageBalancer()
{
    m_LuminanceSrc = 0.0f;
}

void ImageBalancer::initBalancer(const unsigned char * srcImageData, int width, int height)
{
    cv::Mat srcImage = cv::Mat::zeros(height, width, CV_32FC4);
    int arrayIndex = 0;
    for (int hI = 0; hI < height; hI++)
        for (int wI = 0; wI < width; wI++)
        {
            srcImage.at<cv::Vec4f>(hI, wI) = cv::Vec4f(srcImageData[arrayIndex] / 255.f, srcImageData[arrayIndex+1] / 255.f, srcImageData[arrayIndex+2] / 255.f, srcImageData[arrayIndex+3] / 255.f);
            
            arrayIndex += nPixelChannels;
        }
    m_LuminanceSrc = luminanceMean(srcImage);
    srcImage.release();
}

void ImageBalancer::runBalance(const unsigned char * hairFaceImageData, int faceImageWidth, int faceImageHeight, unsigned char * hairTextureData, int hairTextureWidth, int hairTextureHeight)
{
    cv::Mat hairFaceImage = cv::Mat::zeros(faceImageHeight, faceImageWidth, CV_32FC4);
    int arrayIndex = 0;
    for (int hI = 0; hI < faceImageHeight; hI++)
        for (int wI = 0; wI < faceImageWidth; wI++)
        {
            hairFaceImage.at<cv::Vec4f>(hI, wI) = cv::Vec4f(hairFaceImageData[arrayIndex] / 255.f, hairFaceImageData[arrayIndex+1] / 255.f, hairFaceImageData[arrayIndex+2] / 255.f, hairFaceImageData[arrayIndex+3] / 255.f);
            
            arrayIndex += nPixelChannels;
        }
    float luminanceDst = luminanceMean(hairFaceImage);
    hairFaceImage.release();
    
    static int num = 1;
    printf("%d %f\n", num++, luminanceDst);
    
    float luminanceDisp = m_LuminanceSrc - luminanceDst;
    balanceHairTexture(luminanceDisp, hairTextureData, hairTextureWidth, hairTextureHeight);
}

void ImageBalancer::runBalance(const float faceImageLuminace, unsigned char * hairTextureData, int hairTextureWidth, int hairTextureHeight)
{
    float luminanceDisp = m_LuminanceSrc - faceImageLuminace;
    balanceHairTexture(luminanceDisp, hairTextureData, hairTextureWidth, hairTextureHeight);
}

float ImageBalancer::luminanceMean(cv::Mat& image)
{
    float weightSum = 0.f;
	float luminanceSum = 0.f;
	for(int hI = 0; hI < image.rows; hI++)
    {
		for(int wI = 0; wI < image.cols; wI++)
        {
            cv::Vec4f color = image.at<cv::Vec4f>(hI, wI);
			if(color[3] > 0.5f)
            {
				//luminanceSum += log(IBMAX(IBMIN(rgb2XYZ(color)[1], 1.f), 3.03e-4f));
                luminanceSum += IBMAX(IBMIN(rgb2XYZ(color)[1], 1.f), 0.0f);
				weightSum += 1.f;
			}
		}
	}
	return luminanceSum / weightSum;
}

void ImageBalancer::balanceHairTexture(const float luminanceDisp, unsigned char * textureData, int width, int height)
{
    int arrayIndex = 0;
    for(int i=0; i<height; i++)
        for(int j=0; j<width; j++)
        {
            //for pixel (j, i)
            if(textureData[arrayIndex + 3] >= 0x00)
            {
                cv::Vec4f color = cv::Vec4f(textureData[arrayIndex] / 255.f, textureData[arrayIndex + 1] / 255.f, textureData[arrayIndex + 2] / 255.f, textureData[arrayIndex + 3] / 255.f);
//                float luminance = rgb2XYZ(color)[1];
//                float tmpluminanceDisp = exp(log(IBMAX(IBMIN(luminance, 1.f), 3.03e-4f)) + luminanceDisp) - luminance;
                float tmpluminanceDisp = luminanceDisp;
                color += cv::Vec4f(tmpluminanceDisp, tmpluminanceDisp, tmpluminanceDisp, 0.f);
                for (int dimI = 0; dimI < 4; dimI++)
                    color[dimI] = IBMAX(IBMIN(color[dimI], 1.f), 0.f);
                
                textureData[arrayIndex] = (unsigned char)(color[0] * 255);
                textureData[arrayIndex + 1] = (unsigned char)(color[1] * 255);
                textureData[arrayIndex + 2] = (unsigned char)(color[2] * 255);
                textureData[arrayIndex + 3] = (unsigned char)(color[3] * 255);
            }
            arrayIndex += nPixelChannels;
        }
}

cv::Vec4f ImageBalancer::rgb2XYZ(cv::Vec4f rgb)
{
    return cv::Vec4f(
                 rgb.dot(cv::Vec4f(0.4124564f, 0.3575761f, 0.1804375f, 0.f)),
                 rgb.dot(cv::Vec4f(0.2126729f, 0.7151522f, 0.0721750f, 0.f)),
                 rgb.dot(cv::Vec4f(0.0193339f, 0.1191920f, 0.9503041f, 0.f)),
                 rgb[3]);
}
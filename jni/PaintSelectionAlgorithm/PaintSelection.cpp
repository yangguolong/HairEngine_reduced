//
//  PaintSelection.cpp
//  MyPainter
//
//  Created by yangguolong on 13-5-9.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "PaintSelection.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <limits>
#include <opencv2/opencv.hpp>
#include "ImageUtility.h"
#include <stack>


#define BytePerPixel 4  //bytePerPixel should be 3 that is RGBA
#define BackgroundSamples 800
#define DilatingWidth 40
#define Lambda 60
#define Epsilon 0.05f
#define AllowedImagePixelNum 30000 //1024 * 1024
#define SelctionRectExpandLength 10
#define DilationSize 10
#define LineThicknessForFore 6
#define LineThicknessForBack 10

#define isImageOverStep(width, height, x, y) ( (x)<0 || (x)>=(width) || (y)<0 || (y)>=(height) )

const float MaxFloat = std::numeric_limits<float>::max();

PaintSelection::PaintSelection()
{
    _Bitmap = NULL;
    _MarkMap = NULL;
    _Grid = NULL;
    _HEnergyArray = NULL;
    _VEnergyArray = NULL;
    _ForeGmm = NULL;
    _BackGmmForFore = NULL;
    _BackGmmForBack = NULL;
    
    _OutMask = NULL;
    
    _SelectRemovedNumForFore = 0;
    _SelectRemovedNumForBack = 0;
}

PaintSelection::~PaintSelection()
{
    releaseMemory();
}

//interface
void PaintSelection::initPaintSelection(unsigned char * srcBitmap,int srcWidth, int srcHeight)
{
    srand(120);
    _SrcWidth = srcWidth;
    _SrcHeight = srcHeight;
    _OutMask = new unsigned char[_SrcWidth * _SrcHeight];
    
    //scale image and evaluate _ScaleFactor
    int srcTotalNum = srcWidth * srcHeight;
    if(srcTotalNum > AllowedImagePixelNum)
    {
        float srcRadio = ((float)srcWidth) / ((float)srcHeight);
        _Width = (int)sqrtf(AllowedImagePixelNum * srcRadio);
        _Height = _Width / srcRadio;
        _PixelNum = _Width * _Height;
        _BytesPerLine = _Width * BytePerPixel;
        _ScaleFactor = ((float)_Width) / ((float)srcWidth);
        
        //gen _Bitmap
        _Bitmap = new unsigned char[_PixelNum * BytePerPixel];
        
        IplImage * srcImage = cvCreateImage(cvSize(srcWidth, srcHeight),IPL_DEPTH_8U,4);
        uchar * data = (uchar *)srcImage->imageData;
       
        int step = srcImage->widthStep;
        int nChannels = srcImage->nChannels;
        int arrayIndex = 0;
        int cvIndex;
        for(int i=0; i<srcHeight; i++)
            for(int j=0; j<srcWidth; j++)
            {
                //for pixel (j, i)
                cvIndex = i * step + j*nChannels;
                data[cvIndex] = srcBitmap[arrayIndex+2];
                data[cvIndex+1] = srcBitmap[arrayIndex+1];
                data[cvIndex+2] = srcBitmap[arrayIndex];
                data[cvIndex+3] = srcBitmap[arrayIndex+3];
                arrayIndex += BytePerPixel;
            }
        
        IplImage * dstImage = cvCreateImage(cvSize(_Width, _Height),IPL_DEPTH_8U,4);
        cvResize(srcImage, dstImage, CV_INTER_LINEAR);
        
        int totalDstBytes = _PixelNum * BytePerPixel;
        data = (uchar *)dstImage->imageData;
        for(int i=0; i<totalDstBytes; i+=BytePerPixel)
        {
            //opencv BGRA to bitmap RGBA
            _Bitmap[i] = data[i+2];
            _Bitmap[i+1] = data[i+1];
            _Bitmap[i+2] = data[i];
            _Bitmap[i+3] = data[i+3];
        }
        
        step = dstImage->widthStep;
        nChannels = dstImage->nChannels;
        data = (uchar *)dstImage->imageData;
        arrayIndex = 0;
        for(int i=0; i<_Height; i++)
            for(int j=0; j<_Width; j++)
            {
                //for pixel (j, i)
                cvIndex = i * step + j*nChannels;
                _Bitmap[arrayIndex] = data[cvIndex+2];
                _Bitmap[arrayIndex+1] = data[cvIndex+1];
                _Bitmap[arrayIndex+2] = data[cvIndex];
                _Bitmap[arrayIndex+3] = data[cvIndex+3];
                arrayIndex += BytePerPixel;
            }
        
        cvReleaseImage(&srcImage);
        cvReleaseImage(&dstImage);
    }
    else
    {
        _Width = srcWidth;
        _Height = srcHeight;
        _PixelNum = _Width * _Height;
        _BytesPerLine = _Width * BytePerPixel;
        _ScaleFactor = 1.0f;
        //gen _Bitmap
        _Bitmap = new unsigned char[_PixelNum * BytePerPixel];
        memcpy(_Bitmap, srcBitmap, _PixelNum * BytePerPixel);
    }
    
    _MarkMap = new MaskDataType[_PixelNum];
    _CompetitionScribbleSeeds.reserve(_PixelNum);
    _CompetitionScribbles.reserve(10);
    _CompetitionScribbleGmms.reserve(0);
    resetPaintSelection();
    
    _HSize = (_Width - 1) * _Height;
    _VSize = _Width * (_Height - 1);
    _HEnergyArray = new float[_HSize];
    _VEnergyArray = new float[_VSize];
    evaluatePriorEnergy();
    
    printf("_PSScale = %f\nSrc (%d, %d)\nDst (%d %d)\n",_ScaleFactor, _SrcWidth, _SrcHeight, _Width, _Height);
}

void PaintSelection::resetPaintSelection()
{
    memset(_MarkMap, Undecided, _PixelNum * sizeof(MaskDataType));
    _SelectRemovedNumForFore = BackgroundSamples;
    _SelectRemovedNumForBack = BackgroundSamples;
    refreshGMMPointsForRelativeBackground(true);
    refreshGMMPointsForRelativeBackground(false);
    
    if(_Grid)  delete _Grid;
    _Grid = NULL;
    
    _CompetitionScribbleSeeds.clear();
    _CompetitionScribbles.clear();
    for(int i=_CompetitionScribbleGmms.size()-1; i>=0; i--)
        delete _CompetitionScribbleGmms[i];
    _CompetitionScribbleGmms.clear();
}

void PaintSelection::releaseMemory()
{
    if(_Bitmap)
    {
        delete [] _Bitmap;
        _Bitmap = NULL;
    }
    if(_MarkMap)
    {
        delete [] _MarkMap;
        _MarkMap = NULL;
    }
    if(_Grid)
    {
        delete _Grid;
        _Grid = NULL;
    }
    if(_HEnergyArray)
    {
        delete [] _HEnergyArray;
        _HEnergyArray = NULL;
    }
    if(_VEnergyArray)
    {
        delete [] _VEnergyArray;
        _VEnergyArray = NULL;
    }
    if(_ForeGmm)
    {
        delete _ForeGmm;
        _ForeGmm = NULL;
    }
    if(_BackGmmForFore)
    {
        delete  _BackGmmForFore;
        _BackGmmForFore = NULL;
    }
    if(_BackGmmForBack)
    {
        delete  _BackGmmForBack;
        _BackGmmForBack = NULL;
    }
    if(_OutMask)
    {
        delete [] _OutMask;
        _OutMask = NULL;
    }
    
    _OutContour.contourNum = 0;
    if(_OutContour.contourPointNumArray)
    {
        delete [] _OutContour.contourPointNumArray;
        _OutContour.contourPointNumArray = NULL;
    }
    if(_OutContour.contourPoints)
    {
        delete [] _OutContour.contourPoints;
        _OutContour.contourPoints = NULL;
    }
    
    _CompetitionScribbles.clear();
    _CompetitionScribbleSeeds.clear();
    for(int i=_CompetitionScribbleGmms.size()-1; i>=0; i--)
        delete _CompetitionScribbleGmms[i];
    _CompetitionScribbleGmms.clear();
}

PSContour * PaintSelection::doPaintSelectionWithContourBack(PSPoint * points, int size, bool isForeground, bool & isMaskChanged)
{
    if(points == NULL || size <=0)
    {
        printf("points array is invalid or has no points (size == %d)\n", size);
        return &_OutContour;
    }
    
    isMaskChanged = true;
    
    // check new points
    bool isNewBackgroundOccupied;
    PSPoint * expandPoints = NULL;
    int expandPointsSize = 0;
    int * seedIndexArray = NULL;
    int seedIndexArraySize = 0;
    
    translatePointsAndRect(points, size, _ScaleFactor);
    expandStroke(points, size, expandPoints, expandPointsSize, isForeground);
    checkNewPoints(expandPoints, expandPointsSize, isForeground, isNewBackgroundOccupied, seedIndexArray, seedIndexArraySize);
    delete [] expandPoints;
    
    //if no new "background" is occupied
    if(!isNewBackgroundOccupied)
    {
        printf("no need to evaluate\n");
        delete [] seedIndexArray;
        isMaskChanged = false;
        return &_OutContour;
    }
    
    //get GMM for foreground
    createGMMForWanted(seedIndexArray, seedIndexArraySize, isForeground);
    
    //GMM for background has been refreshed in the end of last extension
    
    //get GMMs for competition scribbles
    createGMMsForCompetitionScribbles(isForeground);
    
    //create Grid and fill energy
    if(_Grid)   delete _Grid;
    _Grid = new Grid(_Width, _Height);
    fillPriorEnergy();
    fillLikelihoodEnergy(isForeground);
    
    //do graph cut
    _Grid->compute_maxflow();
    
    // write back the result
    updateGraphCutResultForContour(seedIndexArray, seedIndexArraySize, isForeground);
    delete [] seedIndexArray;
    
    return &_OutContour;
}

unsigned char * PaintSelection::getFinalMask()
{
    int arrayIndex;
    
    IplImage * srcImage = cvCreateImage(cvSize(_Width, _Height),IPL_DEPTH_8U,1);
    uchar * data = (uchar *)srcImage->imageData;
    int step = srcImage->widthStep;
    int nChannels = srcImage->nChannels;
    arrayIndex = 0;
    for(int i=0; i<_Height; i++)
        for(int j=0; j<_Width; j++)
        {
            //for pixel (j, i)
            data[i*step + j*nChannels] = ((_MarkMap[arrayIndex++] & Foregroud) ? 0xff: 0x00);
        }
    
    IplImage * dstImage = cvCreateImage(cvSize(_SrcWidth, _SrcHeight),IPL_DEPTH_8U,1);
    cvResize(srcImage, dstImage, CV_INTER_LINEAR);
    
    arrayIndex = 0;
    step = dstImage->widthStep;
    nChannels = dstImage->nChannels;
    data = (uchar *)dstImage->imageData;
    for(int i=0; i<_SrcHeight; i++)
        for(int j=0; j<_SrcWidth; j++)
        {
            //for pixel (j, i)
            _OutMask[arrayIndex++] = ((data[i*step + j*nChannels]) ? 0xff: 0x00);
        }
    
    cvReleaseImage(&srcImage);
    cvReleaseImage(&dstImage);
    
    return _OutMask;
}

unsigned char * PaintSelection::getFinalMaskWithDilation()
{
    //背景为黑色0x00 前景为白色0xff，所以使用dilation膨胀，白增黑减
    IplImage * src = cvCreateImage(cvSize(_Width, _Height), IPL_DEPTH_8U, 1);
    IplImage * dilation = cvCreateImage(cvSize(_Width, _Height), IPL_DEPTH_8U, 1);
    
    if(src->imageData && dilation->imageData)
    {
        //do dilation
        int arrayIndex = 0;
        unsigned char * data = (unsigned char *)src->imageData;
        int step = src->widthStep;
        int nChannels = src->nChannels;
        for(int i=0; i<_Height; i++)
            for(int j=0; j<_Width; j++)
                data[i*step + j*nChannels] = ( (_MarkMap[arrayIndex] & Foregroud)?0xff : 0x00);
        
        int scaledDilationSize = _ScaleFactor * DilationSize;
        if(scaledDilationSize == 0)
            scaledDilationSize = 1;
        
        IplConvKernel * element = cvCreateStructuringElementEx(2*scaledDilationSize+1, 2*scaledDilationSize+1, scaledDilationSize, scaledDilationSize, cv::MORPH_ELLIPSE);
        cvDilate(src, dilation, element);
        cvReleaseStructuringElement(&element);
        
        //resize result mask image
        IplImage * dstImage = cvCreateImage(cvSize(_SrcWidth, _SrcHeight),IPL_DEPTH_8U,1);
        cvResize(dilation, dstImage, CV_INTER_LINEAR);
        
        arrayIndex = 0;
        step = dstImage->widthStep;
        nChannels = dstImage->nChannels;
        data = (uchar *)dstImage->imageData;
        for(int i=0; i<_SrcHeight; i++)
            for(int j=0; j<_SrcWidth; j++)
            {
                //for pixel (j, i)
                _OutMask[arrayIndex++] = ((data[i*step + j*nChannels]) ? 0xff: 0x00);
            }
        cvReleaseImage(&dstImage);
    }
    
    cvReleaseImage(&src);
    cvReleaseImage(&dilation);
    
    return _OutMask;
}

//pixel color fetching operation
PSColor PaintSelection::pixelColorAt(int x, int y)
{
    int anchorPoint = y * _BytesPerLine + x * BytePerPixel;
    return PSColor(_Bitmap[anchorPoint], _Bitmap[anchorPoint + 1], _Bitmap[anchorPoint + 2]);
}

PSColor PaintSelection::pixelColorAtArrayAnchorPoint(int anchorPoint)
{
    return PSColor(_Bitmap[anchorPoint], _Bitmap[anchorPoint + 1], _Bitmap[anchorPoint + 2]);
}

PSColorF PaintSelection::pixelColorFloatAt(int x, int y)
{
    int anchorPoint = y * _BytesPerLine + x * BytePerPixel;
    return PSColorF(_Bitmap[anchorPoint], _Bitmap[anchorPoint + 1], _Bitmap[anchorPoint + 2]);
}

PSColorF PaintSelection::pixelColorFloatAtArrayAnchorPoint(int anchorPoint)
{
    return PSColorF(_Bitmap[anchorPoint], _Bitmap[anchorPoint + 1], _Bitmap[anchorPoint + 2]);
}

//substeps
void PaintSelection::generateBackgoundSample(int num, bool isForeground)
{
    const MaskDataType candidateMask = isForeground? UndecidedAllForForeMask : UndecidedAllForBackMask;
    const MaskDataType propertyMask  = isForeground? BackSelectForFore: BackSelectForBack;
    int index;
    MaskDataType tag;
    
    while(num)
    {
        index = rand() % _PixelNum;
        tag = _MarkMap[index];
        
        if( (tag & candidateMask) && ((~tag) & propertyMask) )
        {
            _MarkMap[index] |= propertyMask;
            num--;
        }
    }
}

void PaintSelection::evaluatePriorEnergy()
{
    //evaluate SQR distance
    _Beta = 0.0f;
    
    float distance;
    PSColor color1, color2;
    float R, G, B;
    int arrayIndex = 0;

    for(int i=0; i<_Height; i++)
        for(int j=1; j<_Width; j++)
        {
            // for link (j - 1, i) --- (j, i)
            color1 = pixelColorAt(j - 1, i);
            color2 = pixelColorAt(j, i);
            R = (color1.r - color2.r) / 255.0f;
            G = (color1.g - color2.g) / 255.0f;
            B = (color1.b - color2.b) / 255.0f;
            distance = SQRLENGTH(R, G, B);
            _Beta += distance;
            //_Beta += sqrtf(distance);
            _HEnergyArray[arrayIndex++] = distance;
        }
    
    arrayIndex = 0;
    for(int i=0;i<_Width;i++)
        for(int j=1;j<_Height;j++)
        {
            // for link (i, j - 1) --- (i, j)
            color1 = pixelColorAt(i, j - 1);
            color2 = pixelColorAt(i, j);
            R = (color1.r - color2.r) / 255.0f;
            G = (color1.g - color2.g) / 255.0f;
            B = (color1.b - color2.b) / 255.0f;
            distance = SQRLENGTH(R, G, B);
            _Beta += distance;
            //_Beta += sqrtf(distance);
            _VEnergyArray[arrayIndex++] = distance;
        }
    
    
    // evaluate Beta
    _Beta /= (_HSize + _VSize);
    _Beta = 1.0f / _Beta;
    
    
    //evaluate neibor energy
    for(int i=0; i<_HSize; i++)
    {
        if(i == (1023 * 767 + 197))
        {
            int a=1;
            a++;
        }
        //distance = sqrtf(_HEnergyArray[i]);
        distance = _HEnergyArray[i];
        _HEnergyArray[i] =  (1.0f / ((_Beta * distance) + Epsilon)) * Lambda;
    }
    
    for(int i=0; i<_VSize; i++)
    {
        //distance = sqrtf(_VEnergyArray[i]);
        distance = _VEnergyArray[i];
        _VEnergyArray[i] =  (1.0f / ((_Beta * distance) + Epsilon)) * Lambda;
    }
}

void PaintSelection::fillPriorEnergy()
{
    float valG;
    int arrayIndex = 0;
    
    for(int i=0; i<_Height; i++)
        for(int j=1; j<_Width; j++)
        {
            // for link (j-1, i) --- (j, i)
            valG = _HEnergyArray[arrayIndex++];
            _Grid->set_neighbor_cap(_Grid->node_id(j, i), -1, 0, valG);
            _Grid->set_neighbor_cap(_Grid->node_id(j-1, i), +1, 0, valG);
        }
    
    arrayIndex = 0;
    for(int i=0;i<_Width;i++)
        for(int j=1;j<_Height;j++)
        {
            // for link (i, j-1) --- (i, j)
             valG = _VEnergyArray[arrayIndex++];
            _Grid->set_neighbor_cap(_Grid->node_id(i, j), 0, -1, valG);
            _Grid->set_neighbor_cap(_Grid->node_id(i, j-1), 0, +1, valG);
        }
}

void PaintSelection::fillLikelihoodEnergy(bool isForeground)
{
    const MaskDataType seedMask = isForeground?ForeSeed:BackSeed;
    Gmm * backGmm = isForeground?_BackGmmForFore:_BackGmmForBack;
    
    MaskDataType tag;
    PSColorF color;
    float energyF, energyB;
    int arrayIndex =0;
    int anchorPoint = 0;

    if(isForeground)
    {
        for(int i=0; i<_Height; i++)
            for(int j=0; j<_Width; j++)
            {
                //for pixel (j, i)
                tag = _MarkMap[arrayIndex];
                if(tag & ForegroundAndSeedMask)
                {
                    energyF = 0.0f;
                    energyB = MaxFloat;
                }
                else if(tag & UndecidedAllForForeMask)
                {
                    if(tag & BackHard)
                    {
                        energyF = MaxFloat;
                        energyB = 0.0f;
                    }
                    else
                    {
                        color = pixelColorFloatAtArrayAnchorPoint(anchorPoint);
                        
                        energyF = _ForeGmm->pdf(color.data);
                        energyB = backGmm->pdf(color.data);
                        
                        if(energyF > 0.0f)
                            energyF = (float)(-log(energyF));
                        else
                            energyF = MaxFloat;
                        
                        if(energyB > 0.0f)
                            energyB = (float)(-log(energyB));
                        else
                            energyB = MaxFloat;
                    }
                }
                else
                {
                    // it will never enter here
                    printf("Opps 1, enter error place.\n");
                    energyF = MaxFloat;
                    energyB = 0.0f;
                }
                
                _Grid->set_terminal_cap(_Grid->node_id(j, i), energyB, energyF); // note : graph cut does Max flow
                
                arrayIndex++;
                anchorPoint += BytePerPixel;
            }
        
        int scribbleNum = _CompetitionScribbles.size();
        int tmpIndex, xPos, yPos;
        for(int i=0; i<scribbleNum; i++)
        {
            std::vector<int> & scribble = _CompetitionScribbles[i];
            int scribblePointNum = scribble.size();
            Gmm * tmpGmm = _CompetitionScribbleGmms[i];
            for(int j=0; j<scribblePointNum; j++)
            {
                tmpIndex = scribble[j];
                //if the hard scribble is not occupied by seedMask
                if(!(_MarkMap[tmpIndex] & seedMask))
                {
                    xPos = tmpIndex % _Width;
                    yPos = tmpIndex / _Width;
                    
                    color = pixelColorFloatAtArrayAnchorPoint(tmpIndex * BytePerPixel);
                    energyF = _ForeGmm->pdf(color.data);
                    energyB = tmpGmm->pdf(color.data);
                    
                    if(energyF > 0.0f)
                        energyF = (float)(-log(energyF));
                    else
                        energyF = MaxFloat;
                    
                    if(energyB > 0.0f)
                        energyB = (float)(-log(energyB));
                    else
                        energyB = MaxFloat;
                    
                    _Grid->set_terminal_cap(_Grid->node_id(xPos, yPos), energyB, energyF); // note : graph cut does Max flow
                }
            }
        }
    }
    else
    {
        for(int i=0; i<_Height; i++)
            for(int j=0; j<_Width; j++)
            {
                //for pixel (j, i)
                tag = _MarkMap[arrayIndex];
                if(tag & BackgroundAndSeedMask)
                {
                    energyF = MaxFloat;
                    energyB = 0.0f;
                }
                else if(tag & UndecidedAllForBackMask)
                {
                    if(tag & ForeHard)
                    {
                        energyF = 0.0f;
                        energyB = MaxFloat;
                    }
                    else
                    {
                        color = pixelColorFloatAtArrayAnchorPoint(anchorPoint);
                        // exchange here
                        energyB = _ForeGmm->pdf(color.data);                    
                        energyF = backGmm->pdf(color.data);
                        
                        if(energyF > 0.0f)
                            energyF = (float)(-log(energyF));
                        else
                            energyF = MaxFloat;
                        
                        if(energyB > 0.0f)
                            energyB = (float)(-log(energyB));
                        else
                            energyB = MaxFloat;
                    }
                }
                else
                {
                    //it will never enter here
                    printf("Opps 2, enter error place.\n");
                    energyF = 0.0f;
                    energyB = MaxFloat;
                }
                
                _Grid->set_terminal_cap(_Grid->node_id(j, i), energyB, energyF); // note : graph cut does Max flow
                
                arrayIndex++;
                anchorPoint += BytePerPixel;
            }
        
        int scribbleNum = _CompetitionScribbles.size();
        int tmpIndex, xPos, yPos;
        for(int i=0; i<scribbleNum; i++)
        {
            std::vector<int> & scribble = _CompetitionScribbles[i];
            int scribblePointNum = scribble.size();
            Gmm * tmpGmm = _CompetitionScribbleGmms[i];
            for(int j=0; j<scribblePointNum; j++)
            {
                tmpIndex = scribble[j];
                //if the hard scribble is not occupied by seedMask
                if(!(_MarkMap[tmpIndex] & seedMask))
                {
                    xPos = tmpIndex % _Width;
                    yPos = tmpIndex / _Width;
                    
                    color = pixelColorFloatAtArrayAnchorPoint(tmpIndex * BytePerPixel);
                    energyB = _ForeGmm->pdf(color.data);
                    energyF = tmpGmm->pdf(color.data);
                    
                    if(energyF > 0.0f)
                        energyF = (float)(-log(energyF));
                    else
                        energyF = MaxFloat;
                    
                    if(energyB > 0.0f)
                        energyB = (float)(-log(energyB));
                    else
                        energyB = MaxFloat;
                    
                    _Grid->set_terminal_cap(_Grid->node_id(xPos, yPos), energyB, energyF); // note : graph cut does Max flow
                }
            }
        }
    }
}

void PaintSelection::translatePointsAndRect(PSPoint *points, int size, float scaleFactor)
{
    for(int i=0; i<size; i++)
    {
        points[i].x *= scaleFactor;
        points[i].y *= scaleFactor;
    }
}

void PaintSelection::expandStroke(PSPoint * points, int size, PSPoint * & expandPoints, int & expandPointsSize, bool isForeground)
{
    float thickness = isForeground?LineThicknessForFore:LineThicknessForBack;
    const float curlineThickness = thickness * _ScaleFactor;
    
    expandPoints = new PSPoint[_Width * _Height];
    
    IplImage * rImg = cvCreateImage(cvSize(_Width, _Height), IPL_DEPTH_8U, 1);
    cvSet(rImg, cvScalar(0));
    
    cvCircle(rImg, cvPoint(points[0].x, points[0].y), (curlineThickness / 2.0f), cvScalar(255), -1, 4);
    
    for(int i=1; i<size; i++)
        cvLine(rImg, cvPoint(points[i-1].x, points[i-1].y), cvPoint(points[i].x, points[i].y), cvScalar(255), curlineThickness, 4);
    
    if(size > 1)
        cvCircle(rImg, cvPoint(points[size-1].x, points[size-1].y), (curlineThickness / 2.0f), cvScalar(255), -1, 4);
    
    int arrayIndex = 0;
    unsigned char * data = (unsigned char *)rImg->imageData;
    int step = rImg->widthStep;
    int nChannels = rImg->nChannels;
    for(int i=0; i<_Height; i++)
        for(int j=0; j<_Width; j++)
        {
            //for pixle(j, i)
            if(data[i*step + j*nChannels] > 0x80)
                expandPoints[arrayIndex++] = PSPoint(j, i);
        }
    cvReleaseImage(&rImg);
    
    expandPointsSize = arrayIndex;
}

void PaintSelection::checkNewPoints(const PSPoint * points, int size, bool isForeground, bool & isNewBackgroundOccupied, int * & seedIndexArray, int & seedIndexArraySize)
{
    isNewBackgroundOccupied = false;    
    const MaskDataType candidateMask = isForeground? UndecidedAllForForeMask : UndecidedAllForBackMask;
    const MaskDataType seedMask = isForeground? ForeSeed : BackSeed;
    const MaskDataType competitionHardMask = isForeground?BackHard:ForeHard;
    
    seedIndexArray = new int[_PixelNum];
    if(!seedIndexArray)
    {
        printf("failed to alloc memory for seedIndexArray\n");
        return;
    }
    seedIndexArraySize = 0;
    
    int arrayIndex;
    MaskDataType tag;
    _CompetitionScribbleSeeds.clear();
    
    for(int i=0; i<size; i++)
    {
        //As cv:findContour doesn't take into account 1-pixel border of input image,
        //so a pixel of the border has to be removed from seedIndexArray,
        //or it may cause a crash that no new contour contains any seed pixel.
        if(points[i].x <=0 || points[i].x >= (_Width - 1) || points[i].y <=0 || points[i].y >= (_Height - 1))
            continue;
        
        arrayIndex = points[i].y * _Width + points[i].x;
        tag = _MarkMap[arrayIndex];
        
        if(tag & candidateMask)
        {
            if(tag & competitionHardMask)
                _CompetitionScribbleSeeds.push_back(arrayIndex);
            
            isNewBackgroundOccupied = true;
            _MarkMap[arrayIndex] = (tag & PropertiesMask) | seedMask;
            seedIndexArray[seedIndexArraySize++] = arrayIndex;
        }
    }
    printf("competitionScribbleSeeds num: %ld\n", _CompetitionScribbleSeeds.size());
    printf("wanted seeds num: %d\n", seedIndexArraySize);
}

void PaintSelection::createGMMForWanted(const int * seedIndexArray, int seedIndexArraySize, bool isForeground)
{
    const MaskDataType candidateMask = isForeground? Foregroud : Background;
    const MaskDataType wantedSeedMask = isForeground? ForeSeed: BackSeed;
    
    PSColorF * wantedSampleColorArray = new PSColorF[_PixelNum];
    int wantedSampleArraySize = 0;
    int arrayIndex;
    
    //get foreground samples
    bool * selectionMaskArray = new bool[_PixelNum]; // mark whether has been selected as wanted point
    memset(selectionMaskArray, 0, sizeof(bool) * _PixelNum);
    
    //采集所有种子点
    for(int i=0; i<seedIndexArraySize; i++)
    {
        arrayIndex = seedIndexArray[i];
        selectionMaskArray[arrayIndex] = true;
        wantedSampleColorArray[wantedSampleArraySize++] = pixelColorFloatAtArrayAnchorPoint(arrayIndex * BytePerPixel);
    }
    
    //采集种子点辐射区域的前景点
    //左 上 右 下
    const int neiborOffsetX[4] = {-1, 0, +1, 0};
    const int neiborOffsetY[4] = {0, -1, 0, +1};
    const int neiborIndexOffset[4] = {-1, -_Width, 1, +_Width};
    int xPos, yPos;
    int nXPos, nYPos, nArrayIndex;
    bool isBoundaryPoint;
    int xMin, yMin, xMax, yMax;
    for(int i=0; i<seedIndexArraySize; i++)
    {
        arrayIndex = seedIndexArray[i];
        xPos = arrayIndex % _Width;
        yPos = arrayIndex / _Width;
        
        //检查是否是边界点
        isBoundaryPoint = false;
        for(int j=0; j<4; j++)
        {
            nXPos = xPos + neiborOffsetX[j];
            nYPos = yPos + neiborOffsetY[j];
            if(!isImageOverStep(_Width, _Height, nXPos, nYPos))
            {
                nArrayIndex = arrayIndex + neiborIndexOffset[j];
                if(_MarkMap[nArrayIndex] & wantedSeedMask)
                    continue;
                else
                {
                    isBoundaryPoint = true;
                    break;
                }
            }
        }
        if(!isBoundaryPoint)
            continue;
        
        //计算边界点辐射的Rect
        xMin = MAX((xPos - SelctionRectExpandLength), 0);
        xMax = MIN((xPos + SelctionRectExpandLength), (_Width - 1));
        yMin = MAX((yPos - SelctionRectExpandLength), 0);
        yMax = MIN((yPos + SelctionRectExpandLength), (_Height - 1));
        
        //采集Rect中未被选择的前景或种子点
        for(int col=yMin; col<=yMax; col++)
            for(int row=xMin; row<xMax; row++)
            {
                // for pixel (row, col)
                nArrayIndex = col * _Width + row;
                if( (_MarkMap[nArrayIndex] & candidateMask) && (!selectionMaskArray[nArrayIndex]) )
                {
                    wantedSampleColorArray[wantedSampleArraySize++] = pixelColorFloatAtArrayAnchorPoint(nArrayIndex * BytePerPixel);
                    selectionMaskArray[nArrayIndex] = true;
                }
            }
    }
    delete [] selectionMaskArray;
    
    // create GMM for foreground samples
    if(_ForeGmm)
        delete _ForeGmm;
    _ForeGmm = new Gmm(4, 3); // attention needed
    _ForeGmm->init((float *)wantedSampleColorArray, wantedSampleArraySize);
    _FIterNum = _ForeGmm->em((float *)wantedSampleColorArray, wantedSampleArraySize);
    
    delete [] wantedSampleColorArray;
}

void PaintSelection::refreshGMMPointsForRelativeBackground(bool isForeground)
{
    const MaskDataType targetPropertyMask  = isForeground? BackSelectForFore: BackSelectForBack;
    Gmm * * backGmmPtr = &(isForeground?_BackGmmForFore:_BackGmmForBack);
    int removedNum = isForeground?_SelectRemovedNumForFore:_SelectRemovedNumForBack;
    
    if((*backGmmPtr) && removedNum == 0)
        return;
    
    PSColorF * backSampleColorArray = new PSColorF[BackgroundSamples];
    int backSampleArraySize = 0;
    int arrayIndex;
    
    // get background samples
    generateBackgoundSample(removedNum, isForeground);
    if(isForeground)
        _SelectRemovedNumForFore = 0;
    else
        _SelectRemovedNumForBack = 0;
    
    for(int i=0; i<_Height; i++)
        for(int j=0; j<_Width; j++)
        {
            // for pixel (j, i)
            arrayIndex = i * _Width + j;
            if(_MarkMap[arrayIndex] & targetPropertyMask)
                backSampleColorArray[backSampleArraySize++] = pixelColorFloatAt(j, i);
        }
    
    assert(backSampleArraySize == BackgroundSamples);
    
    if((*backGmmPtr))
        delete (*backGmmPtr);
    (*backGmmPtr) = new Gmm(4, 3);
    (*backGmmPtr)->init((float *)backSampleColorArray, backSampleArraySize);
    _BIterNum = (*backGmmPtr)->em((float *)backSampleColorArray, backSampleArraySize);
    
    delete [] backSampleColorArray;
}

void PaintSelection::createGMMsForCompetitionScribbles(bool isForeground)
{
    const MaskDataType competitionHardMask = isForeground? BackHard : ForeHard;
    
    const int seedSize = _CompetitionScribbleSeeds.size();
    if(seedSize == 0)
        return;
    
    //find out all the competition stribbles
    const int neiborOffsetX[4] = {-1, 0, +1, 0};
    const int neiborOffsetY[4] = {0, -1, 0, +1};
    const int neiborIndexOffset[4] = {-1, -_Width, 1, +_Width};
    
    _CompetitionScribbles.clear();
    int seedIndex = 0, tmpIndex = 0, nArrayIndex = 0;
    int xPos, yPos, nXPos, nYPos;
    std::stack<int> traceStack;
    std::vector<bool> isVisited(_PixelNum, false);
    for(int i=0; i<seedSize; i++)
    {
        seedIndex = _CompetitionScribbleSeeds[i];
        if(!isVisited[seedIndex])
        {
            traceStack.push(seedIndex);
            
            std::vector<int> newScribbleVec;
            newScribbleVec.reserve((_PixelNum>>2));
            while(!traceStack.empty())
            {
                tmpIndex = traceStack.top();
                if(isVisited[tmpIndex])
                {
                    traceStack.pop();
                    continue;
                }
                newScribbleVec.push_back(tmpIndex);
                isVisited[tmpIndex] = true;
                traceStack.pop();
                
                xPos = tmpIndex % _Width;
                yPos = tmpIndex / _Width;
                for(int j=0; j<4; j++)
                {
                    nXPos = xPos + neiborOffsetX[j];
                    nYPos = yPos + neiborOffsetY[j];
                    if(!isImageOverStep(_Width, _Height, nXPos, nYPos))
                    {
                        nArrayIndex = tmpIndex + neiborIndexOffset[j];
                        if((_MarkMap[nArrayIndex] & competitionHardMask) && (!isVisited[nArrayIndex]) )
                            traceStack.push(nArrayIndex);
                    }
                }
            }
            _CompetitionScribbles.push_back(newScribbleVec);
        }
    }
//    printf("competition scribble num: %ld\n", _CompetitionScribbles.size());
//    for(int i=0 ;i<_CompetitionScribbles.size(); i++)
//    {
//        printf("scribble %d: %ld\n", i, _CompetitionScribbles[i].size());
//    }
    
    //create GMMs for each competition scribble
    int scribbleNum = _CompetitionScribbles.size();
    for(int i=_CompetitionScribbleGmms.size()-1; i>=0; i--)
        delete _CompetitionScribbleGmms[i];
    _CompetitionScribbleGmms.clear();
    _CompetitionScribbleGmms.resize(scribbleNum);
    
    PSColorF * wantedSampleColorArray = new PSColorF[_PixelNum];
    int wantedSampleArraySize = 0;
    for(int i=0; i<scribbleNum; i++)
    {
        std::vector<int> & tmpScribble = _CompetitionScribbles[i];
        wantedSampleArraySize = tmpScribble.size();
        for(int j=0; j<wantedSampleArraySize; j++)
            wantedSampleColorArray[j] = pixelColorFloatAtArrayAnchorPoint(tmpScribble[j] * BytePerPixel);
        
        _CompetitionScribbleGmms[i] = new Gmm(4, 3);
        _CompetitionScribbleGmms[i]->init((float *)wantedSampleColorArray, wantedSampleArraySize);
        _CompetitionScribbleGmms[i]->em((float *)wantedSampleColorArray, wantedSampleArraySize);
    }
    delete [] wantedSampleColorArray;
}

void PaintSelection::updateGraphCutResultForContour(int * seedIndexArray, int seedIndexArraySize, bool isForeground)
{
    const MaskDataType mask = isForeground? Foregroud: Background;
    const MaskDataType seedMask = isForeground? ForeSeed: BackSeed;
    const MaskDataType useAsBackPropertyMask  = isForeground? BackSelectForFore: BackSelectForBack;
    const MaskDataType hardScribblePropertyNeedRemoveMask  = isForeground? BackHard: ForeHard;
    const MaskDataType hardScribblePropertyNeedAssignMask  = isForeground? ForeHard: BackHard;
    const int segmentVal = isForeground? 0: 1;
    
    //get newly expanded area
    int arrayIndex = 0;
    cv::Mat cvMat(_Height, _Width, CV_8UC1);
    for(int i=0;i<_Height;i++)
        for(int j=0;j<_Width;j++)
        {
            // for pixel (j, i)
            // a new point belonging to wanted ground
            if( (_Grid->get_segment(_Grid->node_id(j, i)) == segmentVal) && (!(_MarkMap[arrayIndex] & mask)) )
            {
                cvMat.at<bool>(i, j) = true;
            }
            else
                cvMat.at<bool>(i, j) = false;
            
            arrayIndex++;
        }
    
    //evaluate contours for new area
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(cvMat, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    cvMat.release();
    
    //find out a contour that contains user specicfied points
    int selectedContourIndex = -1;
    int contourSize = contours.size();
    int xPos, yPos;
    for(int i=0; i<seedIndexArraySize; i++)
    {
        arrayIndex = seedIndexArray[i];
        xPos = arrayIndex % _Width;
        yPos = arrayIndex / _Width;
        cv::Point cvPoint = cv::Point(xPos , yPos);
        for(int j=0; j<contourSize; j++)
            if( (cv::pointPolygonTest(contours[j], cvPoint, false)) >= 0 )
            {
                selectedContourIndex = j;
                break;
            }
        
        if(selectedContourIndex >= 0)
            break;
    }
    assert(selectedContourIndex >= 0);
    
    
    //add newly expanded area to _MarkMap
    //Meanwhile, update the _BackSelectRemovedNum;
    if(isForeground)
        assert(_SelectRemovedNumForFore == 0);
    else
        assert(_SelectRemovedNumForBack == 0);
    
    cv::Mat cvMatRst(_Height, _Width, CV_8UC1, cv::Scalar(0));
    cv::Scalar color = cv::Scalar(255, 255, 255, 255);
    cv::drawContours(cvMatRst, contours, selectedContourIndex, color, CV_FILLED);
    
    cv::Rect contourRect = cv::boundingRect(contours[selectedContourIndex]);
    int rightBound = contourRect.x + contourRect.width;
    int bottomBound = contourRect.y + contourRect.height;
    rightBound = MIN(rightBound, _Width);
    bottomBound = MIN(bottomBound, _Height);
    
    MaskDataType tag;
    int removedNum = 0;
    for(int i=contourRect.y; i<=bottomBound; i++)
        for(int j=contourRect.x; j<=rightBound; j++)
        {
            //for pixel (j, i)
            if(cvMatRst.at<bool>(i, j))
            {
                arrayIndex = i * _Width + j;
                tag = _MarkMap[arrayIndex];
                
                //count pixels that is backForFore and remove the property
                if(tag & useAsBackPropertyMask)
                {
                    removedNum++;
                    tag = tag & (~useAsBackPropertyMask);
                }
                //if it is a oppsite hard scribble, remove the property
                if(tag & hardScribblePropertyNeedRemoveMask)
                    tag = tag & (~hardScribblePropertyNeedRemoveMask);
                //if it is a seed pixel, then make it the hard scribble
                if(tag & seedMask)
                    tag = tag | hardScribblePropertyNeedAssignMask;
                
                //reserve the properties and make the pixel a foreground/background one
                _MarkMap[arrayIndex] = (tag & PropertiesMask) | mask;
            }
        }
    
    //As we only treat one adjancent contour as newly expended area, some seeds may not be in the contour
    int leftSeedsCount = 0;
    for(int i=0; i<seedIndexArraySize; i++)
    {
        arrayIndex = seedIndexArray[i];
        tag = _MarkMap[arrayIndex];
        if(tag & seedMask)
        {
            if(tag & useAsBackPropertyMask)
            {
                removedNum++;
                tag = tag & (~useAsBackPropertyMask);
            }
            if(tag & hardScribblePropertyNeedRemoveMask)
                tag = tag & (~hardScribblePropertyNeedRemoveMask);
            
            tag = tag | hardScribblePropertyNeedAssignMask;
            
            _MarkMap[arrayIndex] = (tag & PropertiesMask) | mask;
            leftSeedsCount++;
        }
    }
    if(leftSeedsCount > 0)
        printf("Seeds not in newly expended area: %d.\n", leftSeedsCount);
    
    //refresh background gmm
    if(isForeground)
        _SelectRemovedNumForFore += removedNum;
    else
        _SelectRemovedNumForBack += removedNum;
    refreshGMMPointsForRelativeBackground(isForeground);
    
    cvMatRst.release();
    contours.clear();
    
    
    //map updated area to source image 
    IplImage * srcImage = cvCreateImage(cvSize(_Width, _Height),IPL_DEPTH_8U,1);
    uchar * data = (uchar *)srcImage->imageData;
    int step = srcImage->widthStep;
    int nChannels = srcImage->nChannels;
    arrayIndex = 0;
    for(int i=0; i<_Height; i++)
        for(int j=0; j<_Width; j++)
        {
            //for pixel (j, i)
            data[i*step + j*nChannels] = ((_MarkMap[arrayIndex++] & Foregroud) ? 0xff: 0x00);
        }
    
    IplImage * dstImage = cvCreateImage(cvSize(_SrcWidth, _SrcHeight),IPL_DEPTH_8U,1);
    cvResize(srcImage, dstImage, CV_INTER_LINEAR);
    
    //get contours of areas in source image
    cv::Mat cvMatRstContour(_SrcHeight, _SrcWidth, CV_8UC1);
    arrayIndex = 0;
    step = dstImage->widthStep;
    nChannels = dstImage->nChannels;
    data = (uchar *)dstImage->imageData;
    for(int i=0; i<_SrcHeight; i++)
        for(int j=0; j<_SrcWidth; j++)
        {
            //for pixel (j, i)
            cvMatRstContour.at<bool>(i, j) = ((data[i*step + j*nChannels]) ? true: false);
        }
    cvReleaseImage(&srcImage);
    cvReleaseImage(&dstImage);
    
    std::vector<std::vector<cv::Point> > rstContours;
    cv::findContours(cvMatRstContour, rstContours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    cvMatRstContour.release();
    
    //update _OutContour
    if(_OutContour.contourPointNumArray)
    {
        delete [] _OutContour.contourPointNumArray;
        _OutContour.contourPointNumArray = NULL;
    }
    if(_OutContour.contourPoints)
    {
        delete [] _OutContour.contourPoints;
        _OutContour.contourPoints = NULL;
    }
    
    _OutContour.contourNum = rstContours.size();
    _OutContour.totalPointsNum = 0;
    _OutContour.contourPointNumArray = new int[_OutContour.contourNum];
    for(int i=0; i<_OutContour.contourNum; i++)
    {
        _OutContour.contourPointNumArray[i] = rstContours[i].size();
        _OutContour.totalPointsNum += _OutContour.contourPointNumArray[i];
    }
    _OutContour.contourPoints = new PSPointF[_OutContour.totalPointsNum];
    arrayIndex = 0;
    for(int i=0; i<_OutContour.contourNum; i++)
    {
        int num = _OutContour.contourPointNumArray[i];
        std::vector<cv::Point> & tmpVec = rstContours[i];
        for(int j=0; j<num; j++)
            _OutContour.contourPoints[arrayIndex++] = PSPointF(tmpVec[j].x, tmpVec[j].y);
    }
    
    rstContours.clear();
}

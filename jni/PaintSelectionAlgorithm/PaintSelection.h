//
//  PaintSelection.h
//  MyPainter
//
//  Created by yangguolong on 13-5-9.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef PAINTSELECTION_H
#define PAINTSELECTION_H


#include "PSDefinition.h"
#include "GridGraph_2D_4C.h"
#include "fgmm++.hpp"
#include <vector>

//分类
#define Foregroud       0x01  // 前景
#define ForeSeed        0x02  // 前景种子
#define Background      0x04  // 用户选定背景
#define BackSeed        0x08  // 背景种子
#define Undecided       0x10  // 待定背景

//属性
#define BackSelectForFore  0x20     // 适用前景的“待定背景”
#define BackSelectForBack  0x40     // 适用背景的“待定背景”
#define ForeHard           0x80     //硬前景（前景笔刷）
#define BackHard           0x100    //硬背景（背景笔刷）

#define ForegroundAndSeedMask       0x03    //已有的前景和种子点集合
#define BackgroundAndSeedMask       0x0c    //已有背景和种子点集合
#define UndecidedAllForForeMask     0x1c    //对 前景 而言的 “待定背景” 的所有情况
#define UndecidedAllForBackMask     0x13    //对 背景 而言的 “待定背景” 的所有情况
#define PropertiesMask              0x1e0    //所有属性的掩码

typedef int MaskDataType;

typedef GridGraph_2D_4C<float, float, float> Grid;


class PaintSelection
{
public:
    PaintSelection();
    ~PaintSelection();
    void initPaintSelection(unsigned char * srcBitmap,int srcWidth, int srcHeight);
    void resetPaintSelection();
    void releaseMemory();
    //return array : false for fore ground; true for background
    PSContour * doPaintSelectionWithContourBack(PSPoint * points, int size, bool isForeground, bool & isMaskChanged);
    unsigned char * getFinalMask();
    unsigned char * getFinalMaskWithDilation(); // with dilation
    
private:
    //pixel color fetching operation
    PSColor pixelColorAt(int x, int y);
    PSColor pixelColorAtArrayAnchorPoint(int anchorPoint);
    PSColorF pixelColorFloatAt(int x, int y);
    PSColorF pixelColorFloatAtArrayAnchorPoint(int anchorPoint);
    
    //substeps
    void translatePointsAndRect(PSPoint * points, int size, float scaleFactor);
    void expandStroke(PSPoint * points, int size, PSPoint * & expandPoints, int & expandPointsSize, bool isForeground);
    void checkNewPoints(const PSPoint * points, int size, bool isForeground, bool & isNewBackgroundOccupied, int * & seedIndexArray, int & seedIndexArraySize);
    void createGMMForWanted(const int * seedIndexArray, int seedIndexArraySize, bool isForeground);
    void refreshGMMPointsForRelativeBackground(bool isForeground);
    void createGMMsForCompetitionScribbles(bool isForeground);
    void generateBackgoundSample(int num, bool isForeground);
    void evaluatePriorEnergy();
    void fillPriorEnergy();
    void fillLikelihoodEnergy(bool isForeground);
    void updateGraphCutResultForContour(int * seedIndexArray, int seedIndexArraySize, bool isForeground);
    
    unsigned char * _Bitmap;
    int _Width;
    int _Height;
    int _PixelNum;
    int _BytesPerLine;
    MaskDataType * _MarkMap;
    Grid * _Grid;
    Gmm * _ForeGmm;
    Gmm * _BackGmmForFore;
    Gmm * _BackGmmForBack;
    int _SelectRemovedNumForFore;
    int _SelectRemovedNumForBack;
    
    std::vector<int> _CompetitionScribbleSeeds;
    std::vector<std::vector<int> > _CompetitionScribbles;
    std::vector<Gmm *> _CompetitionScribbleGmms;
    
    float _Beta;
    float * _HEnergyArray;
    float * _VEnergyArray;
    int _HSize;
    int _VSize;
    
    float _ScaleFactor; // dstImage / srcImage
    int _SrcWidth;
    int _SrcHeight;
    unsigned char * _OutMask;
    PSContour _OutContour;
    
    int _FIterNum;
    int _BIterNum;
};

#endif

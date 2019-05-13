//
//  HairEngine.h
//  Demo
//
//  Created by yangguolong on 13-7-27.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef HairEngine_H
#define HairEngine_H

#include <opencv2/opencv.hpp>
#include <stdlib.h>

#define UseRegTest
//add this macro to export data
//#define ExportData

class PaintSelection;
class HeadDetector;
#ifdef UseRegTest
class RegTest;
#endif
class HeadGenerator;
class TexGenerator;
class IRenderer;

union HEPointF
{
    HEPointF(){};
    HEPointF(float theX, float theY)
    {
        x = theX;
        y = theY;
    }
    float data[2];
    struct {float x,y;};
};

struct HEContour
{
	int contourNum = 0;
    int * contourPointNumArray = nullptr;
    int totalPointsNum = 0;
    HEPointF * contourPoints = nullptr;
};

class HairEngine
{
public:
    HairEngine();
    ~HairEngine();
    
    bool                initEngine          (const char * dataDir, bool useRegressor);
    bool                closeEngine         ();
    
    bool                setImage            (const unsigned char * bitmapData, int width, int height, const char * workDir);
    //unsigned char *     addStroke         (int type, int size, int * pointArray);
    HEContour *         addStroke           (int type, int size, int * pointArray);
    bool                clearStroke         ();
    bool                finishStroke        (float pmaxDiff = 500.0);

    //added things
    bool				autoGenerateHead(float pmaxDiff = 500.0);
    bool				detectFace();
    bool				generateHead();
    bool				generateTexture(float pmaxDiff = 500.0);

    bool 				changeExpression(int expressionID);
    bool 				changeModifier(int modifierID);
    bool				resetExpression();

    bool 				changeHairColor(float r, float g, float b, float a);

    //end of added things

    bool                finishStrokeStep1   (); //检测特征点，生成头部geometry
    bool                finishStrokeStep2   (float pmaxDiff = 500.0); //生成texture

    bool                initViewer          (int width, int height);
    bool                closeViewer         ();
    bool                resizeViewer        (int width, int height);
    bool                setHairDir          (const char * hairDir);
    bool                setTransform        (float xDegree, float yDegree, float scale);
    bool                resetTransform      ();
    bool                adjustHairPosition  (float disX, float disY, float disZ);
    bool                adjustHairScale  	(float scale);
    bool                resetHairPosition   ();
    bool                enableShadow        (bool enable);
    bool                enableExpression    (bool enable);

    bool                render              ();
    
    bool				setDermabrasionDegree(int degree);

    cv::Mat &           getFeatureMat();
    
private:
    IplImage *          convertImageData2IplImage(const unsigned char * srcBitmap, int srcWidth, int srcHeight);
    void                generateSelectionImage();
    void                evaluateFaceImageFromInputImage();
    void                evaluatePartialFaceImageFromInputImage();
    void 				beautifyImage(const unsigned char* bitmapData, unsigned char* dstBitmapData, int width, int height);
    
    PaintSelection *    _PaintSelection;
    HeadDetector *      _HeadDetector;
#ifdef UseRegTest
    RegTest      *      _RegTest;
#endif
    HeadGenerator *     _HeadGenerator;
    TexGenerator *      _TexGenerator;
    IRenderer *         _Renderer;
    
    int					_dermabrasionDegree; //磨皮程度

    char                _DataDir[500];
    char                _WorkDir[500];
    unsigned char *     _InputBitmapData;
    int                 _InputBitmapWidth;
    int                 _InputBitmapHeight;
    unsigned char *     _SelectionImageData;
    unsigned char *     _InputFaceImageData;
    unsigned char *     _inputPartialFaceImageData; //除去眼睛 鼻子 眉毛 嘴唇的面部区域

    bool                _NeedGenerateHeadModel;
    bool                _NeedGenerateTexture;
    bool				_NeedDetectFace;
    unsigned int        _VaildHairStrokeNum;
    
    HEContour           _HEContour;
    
#ifdef UseRegTest
    bool                _UseRegressor;
#endif

};

#endif /* defined(__Demo__HairEngine__) */

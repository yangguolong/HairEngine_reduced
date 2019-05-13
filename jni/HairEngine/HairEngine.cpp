//
//  HairEngine.cpp
//  Demo
//
//  Created by yangguolong on 13-7-27.
//  Copyright (c) 2013骞� yangguolong. All rights reserved.
//

#include "HairEngine.h"
#include "PaintSelection.h"
#include "HeadDetector.h"
#ifdef UseRegTest
#include "RegTest.h"
#endif
#include "HeadGenerator.h"
#include "TexGenerator.h"
#include "Renderer.h"
#include "Exporter.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <vector>
#include "ImageUtility.h"
#include "ShaderUtility.h"

#include <android/log.h>
#define LOG_TAG "HairEngine"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define nPixelChannel 4
#define DilationSize 10
#define FaceDilationSize 5
#define FeatureNumber 76
#define PartialFeatureNumber 26

int PartialFaceFeaturePoints[PartialFeatureNumber] = {0, 27, 47, 56, 67, 66, 65, 64, 63, 62, 51, 36, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

HairEngine::HairEngine()
{
    _PaintSelection = NULL;
    _HeadDetector = NULL;
#ifdef UseRegTest
    _RegTest = NULL;
#endif
    _HeadGenerator = NULL;
    _TexGenerator = NULL;
    _Renderer = NULL;
    _InputBitmapData = NULL;
    _SelectionImageData = NULL;
    _InputFaceImageData = NULL;
    _inputPartialFaceImageData = NULL;
    
    _dermabrasionDegree = 15;

    _NeedGenerateHeadModel = true;
    _NeedGenerateTexture = true;
    _NeedDetectFace = true;
    _VaildHairStrokeNum = 0;
    
#ifdef UseRegTest
    _UseRegressor = false;
#endif
}

HairEngine::~HairEngine()
{
    closeEngine();
}

bool HairEngine::initEngine(const char * dataDir, bool useRegressor)
{
    if(dataDir == NULL || strlen(dataDir) == 0)
    {
        LOGI("HairEngine: invalid dataDir in initEngine\n");
        return false;
    }
    strcpy(_DataDir, dataDir);
    
#ifdef UseRegTest
    _UseRegressor = useRegressor;
#endif
    
    char path[500];
    sprintf(path, "%s/HeadDetectorData", _DataDir);
#ifdef UseRegTest
    if(!useRegressor)
    {
#endif
        LOGI("Using ASM.\n");
        if(!_HeadDetector)
            _HeadDetector = new HeadDetector(path, true);
#ifdef UseRegTest
    }
    else
    {
        LOGI("Using Regressor.\n");
        if(_RegTest)
            delete _RegTest;
        _RegTest = new RegTest(path);
        _RegTest->loadRegressor();
        _RegTest->setTestInitNum(21);
    }
#endif
    
    return true;
}

bool HairEngine::closeEngine()
{
    if(_PaintSelection)
    {
        _PaintSelection->releaseMemory();
        delete _PaintSelection;
        _PaintSelection = NULL;
    }
    
    if(_HeadDetector)
    {
        delete _HeadDetector;
        _HeadDetector = NULL;
    }
    
#ifdef UseRegTest
    if(_RegTest)
    {
        delete _RegTest;
        _RegTest = NULL;
    }
#endif
    
    if(_HeadGenerator)
    {
        _HeadGenerator->reset();
        delete _HeadGenerator;
        _HeadGenerator = NULL;
    }
    
    if(_TexGenerator)
    {
        delete _TexGenerator;
        _TexGenerator = NULL;
    }
    
    if(_Renderer)
    {
        _Renderer->releaseMemory();
        delete _Renderer;
        _Renderer = NULL;
    }
    
    if(_InputBitmapData)
    {
        delete [] _InputBitmapData;
        _InputBitmapData = NULL;
    }
    
    if(_SelectionImageData)
    {
        delete [] _SelectionImageData;
        _SelectionImageData = NULL;
    }
    if(_InputFaceImageData)
    {
        delete [] _InputFaceImageData;
        _InputFaceImageData = NULL;
    }
    if(_inputPartialFaceImageData)
    {
        delete [] _inputPartialFaceImageData;
        _inputPartialFaceImageData = NULL;
    }
    
    return true;
}

void HairEngine::beautifyImage(const unsigned char* bitmapData, unsigned char* dstBitmapData, int width, int height)
{
	//涓嶄綔澶勭悊
	memcpy(dstBitmapData, bitmapData, width * height * nPixelChannel);
	return;

    //浣跨敤opencv瀵硅緭鍏ュ浘鐗囪繘琛岀（鐨鐞�
    LOGI("HairEngine: process image with dermabrasion begin\n");
    //cv::Mat srcMat = cv::Mat(height, width, CV_8UC1, (void*)bitmapData);
    IplImage* iplImg = convertImageData2IplImage(bitmapData, width, height);
    cv::Mat srcMat(iplImg,true);
    cvReleaseImage(&iplImg);
    char path[500];
    cv::imwrite(path, srcMat);
    LOGI("HairEngine: save testmat to:%s\n", path);

    cv::Mat dstMat = srcMat.clone();
//crash?    cv::bilateralFilter(srcMat,dstMat,_dermabrasionDegree,_dermabrasionDegree*2.0,_dermabrasionDegree/2.0);
    char path2[500];
    sprintf(path2, "%stestmat2.png", _WorkDir);
    cv::imwrite(path2, dstMat);
    LOGI("HairEngine: save testmat2 to:%s\n", path);

//    unsigned char* dermabrassionBitmapData = dstMat.data;
    IplImage dstImage = dstMat;
    assert(dstImage.nChannels == 4);
    unsigned char* dermabrassionBitmapData = (unsigned char*)dstImage.imageData;
    memcpy(dstBitmapData, dermabrassionBitmapData, width * height * nPixelChannel);
    LOGI("HairEngine: process image with dermabrasion end\n");

}

bool HairEngine::setImage(const unsigned char * bitmapData, int width, int height, const char * workDir)
{
    if(bitmapData == NULL || width <=0 || height <=0 || workDir == NULL || strlen(workDir) ==0)
    {
        LOGI("HairEngine: invalid input data in setImage\n");
        return false;
    }
    
    strcpy(_WorkDir, workDir);
    
    if(_PaintSelection)
    {
        _PaintSelection->releaseMemory();
        delete _PaintSelection;
        _PaintSelection = NULL;
    }
    if(_InputBitmapData)
    {
        delete [] _InputBitmapData;
        _InputBitmapData = NULL;
    }
    if(_SelectionImageData)
    {
        delete [] _SelectionImageData;
        _SelectionImageData = NULL;
    }
    if(_InputFaceImageData)
    {
        delete [] _InputFaceImageData;
        _InputFaceImageData = NULL;
    }
    if(_inputPartialFaceImageData)
    {
        delete [] _inputPartialFaceImageData;
        _inputPartialFaceImageData = NULL;
    }
    
    _InputBitmapData = new unsigned char[width * height * nPixelChannel];
    if(!_InputBitmapData)
    {
        LOGI("HairEngine: failed to alloc memory for _InputBitmapData in setImage\n");
        return false;
    }
    _SelectionImageData = new unsigned char[width * height * nPixelChannel];
    if(!_SelectionImageData)
    {
        LOGI("HairEngine: failed to alloc memory for _SelectionImageData in setImage\n");
        delete [] _InputBitmapData;
        _InputBitmapData = NULL;
        return false;
    }
    _InputFaceImageData = new unsigned char[width * height * nPixelChannel];
    if(!_InputFaceImageData)
    {
        LOGI("HairEngine: failed to alloc memory for _InputFaceImageData in setImage\n");
        delete [] _InputBitmapData;
        _InputBitmapData = NULL;
        delete [] _SelectionImageData;
        _SelectionImageData = NULL;
        return false;
    }
    _inputPartialFaceImageData = new unsigned char[width * height * nPixelChannel];
    if(!_inputPartialFaceImageData)
    {
        LOGI("HairEngine: failed to alloc memory for _inputPartialFaceImageData in setImage\n");
        delete [] _InputBitmapData;
        _InputBitmapData = NULL;
        delete [] _SelectionImageData;
        _SelectionImageData = NULL;
        delete [] _InputFaceImageData;
        _InputFaceImageData = NULL;
        return false;
    }
    
    beautifyImage(bitmapData,_InputBitmapData,width,height);
    _InputBitmapWidth = width;
    _InputBitmapHeight = height;
    
    _PaintSelection = new PaintSelection();
    _PaintSelection->initPaintSelection(_InputBitmapData, _InputBitmapWidth, _InputBitmapHeight);
    
    _NeedGenerateHeadModel = true;
    _NeedGenerateTexture = true;
    _NeedDetectFace  = true;
    _VaildHairStrokeNum = 0;
    
    return true;
}

HEContour * HairEngine::addStroke(int type, int size, int * pointArray)
{
    if(!_PaintSelection)
    {
        LOGI("HairEngine: invalid PS instance in addStroke\n");
        return NULL;
    }
    
    bool isMaskChanged;
    //unsigned char * mask = _PaintSelection->doPaintSelectionWithMaskBack((PSPoint *)pointArray, size, (type == 1)? true: false, isMaskChanged);
    PSContour * rstContour = _PaintSelection->doPaintSelectionWithContourBack((PSPoint *)pointArray, size, (type == 1)? true: false, isMaskChanged);
    _NeedGenerateTexture = isMaskChanged;
    //杩欎竴绗旇澶村彂閫夊彇鏈夊彉鍔�
    if(isMaskChanged && type == 1)
        _VaildHairStrokeNum++;
    
    if(!rstContour)
        return NULL;
    else
    {
        _HEContour.contourNum = rstContour->contourNum;
        _HEContour.totalPointsNum = rstContour->totalPointsNum;
        _HEContour.contourPointNumArray = rstContour->contourPointNumArray;
        _HEContour.contourPoints = (HEPointF *)rstContour->contourPoints;
    }
    
    return &_HEContour;
}

bool HairEngine::clearStroke()
{
    if(!_PaintSelection)
    {
        LOGI("HairEngine: invalid PS instance in clearStroke\n");
        return false;
    }
    
    if(_VaildHairStrokeNum > 0)
    {
        _NeedGenerateTexture = true;
        _VaildHairStrokeNum = 0;
    }
    _PaintSelection->resetPaintSelection();
    
    return true;
}

bool HairEngine::finishStroke(float pmaxDiff)
{
    LOGI("Enter FinishStroke");
    
    LOGI("****************\n renderer: %s \n Vendor: %s\n Version: %s\n SHADING_LANGUAGE_VERSION:%s \n EXTENSIONS: %s\n*******************\n", glGetString(GL_RENDERER), glGetString(GL_VENDOR), glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION), glGetString(GL_EXTENSIONS));
    
    clock_t startTime, endTime, subStartTime, subEndTime;
    
    if( (
#ifdef UseRegTest
         !_UseRegressor &&
#endif
         !_HeadDetector)
#ifdef UseRegTest
       || (_UseRegressor && !_RegTest)
#endif 
       )
    {
        LOGI("HairEngine: invalid _HeadDetector or _RegTest in finishStroke\n");
        return false;
    }
    if(!_InputBitmapData)
    {
        LOGI("HairEngine: invalid _InputBitmapData instance in finishStroke\n");
        return false;
    }
    
    if(_NeedGenerateHeadModel)
    {
        LOGI("Enter NeedGenerateHeadModel");
        
        startTime = subStartTime = clock();
        
        //Head Detector
        cv::Mat featureMat;
#ifdef UseRegTest
        if(!_UseRegressor)
        {
#endif      
            LOGI("Flag 1");
            
            IplImage * iplImg = convertImageData2IplImage(_InputBitmapData, _InputBitmapWidth, _InputBitmapHeight);
            _HeadDetector->setOriginalImage(iplImg);
            cvReleaseImage(&iplImg);
            if(!_HeadDetector->runDetect())
                return false;
            featureMat = _HeadDetector->getResultFeatures();
            
            LOGI("Flag 2");
#ifdef UseRegTest
        }
        else
        {
            LOGI("Flag 3");
            
            int num = _InputBitmapWidth * _InputBitmapHeight;
            unsigned char * tmpImageData4 = new unsigned char[num * 4];
            memcpy(tmpImageData4, _InputBitmapData, num * 4);
            flipImageData(tmpImageData4, _InputBitmapWidth, _InputBitmapHeight);
            unsigned char * tmpImageData3 = new unsigned char[num * 3];
            int index4 = 0;
            int index3 = 0;
            for(int i=0; i<num; i++)
            {
                index3 += 3;
                index4 += 4;
                tmpImageData3[index3] = tmpImageData4[index4];
                tmpImageData3[index3 + 1] = tmpImageData4[index4 + 1];
                tmpImageData3[index3 + 2] = tmpImageData4[index4 + 2];
            }

            _RegTest->setImgData(tmpImageData3, _InputBitmapWidth, _InputBitmapHeight);
            if(!_RegTest->testFirstRegress())
                return false;
            featureMat = _RegTest->getLandmarks();
            
            delete [] tmpImageData4;
            delete [] tmpImageData3;
            
            LOGI("Flag 4");
        }
#endif
        
        subEndTime = clock();
        LOGI("    head Det: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
    
        subStartTime = clock();
        //Head Generator
        char featureLocationFile[500];
        char trainResultFile[500];
        char auxiliaryDataFile[500];
        char faceTexAndTopoFile[500];
        char foreheadVertexIndexFile[500];
        char resultBoundary[500];
        char resultTrans[500];
        char resultHead[500];
        sprintf(featureLocationFile, "%s/HeadGenData/featureLocation.txt", _DataDir);
        sprintf(trainResultFile, "%s/HeadGenData/trainResult.dat", _DataDir);
        sprintf(auxiliaryDataFile, "%s/HeadGenData/auxiliaryData.txt", _DataDir);
        sprintf(faceTexAndTopoFile, "%s/HeadGenData/faceTexAndTopo.txt", _DataDir);
        sprintf(foreheadVertexIndexFile, "%s/HeadGenData/foreheadVertexIndex.txt", _DataDir);
        sprintf(resultBoundary, "%s/boundary.txt", _WorkDir);
        sprintf(resultTrans, "%s/trans.txt", _WorkDir);
        sprintf(resultHead, "%s/head.obj", _WorkDir);
        
        HeadGenerator * headGen = new HeadGenerator(featureLocationFile, trainResultFile, auxiliaryDataFile, foreheadVertexIndexFile);
        headGen->reset();
        headGen->setFeatures(&featureMat, _InputBitmapWidth, _InputBitmapHeight);
        headGen->runHeadGenerate();
        headGen->save(faceTexAndTopoFile,resultBoundary, resultTrans, resultHead);
        delete headGen;
        
        subEndTime = clock();
        LOGI("    head Gen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        subStartTime = clock();
        evaluateFaceImageFromInputImage();
        evaluatePartialFaceImageFromInputImage();
        subEndTime = clock();
        LOGI("    face & partial image eva: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);

        //create face image
//        char faceImageFile[500];
//        char partialFaceImageFile[500];
//        sprintf(faceImageFile, "%s/faceImage.png", _WorkDir);
//        sprintf(partialFaceImageFile, "%s/partialFaceImage.png", _WorkDir);
//        saveImageDataToFile(_InputFaceImageData, _InputBitmapWidth, _InputBitmapHeight, faceImageFile);
//        saveImageDataToFile(_inputPartialFaceImageData, _InputBitmapWidth, _InputBitmapHeight, partialFaceImageFile);
        
        _NeedGenerateHeadModel = false;
        
        endTime = clock();
        LOGI("Data Gen: %f\n", ((float)(endTime - startTime)) / CLOCKS_PER_SEC);
    }
    
    if(_NeedGenerateTexture)
    {
        LOGI("Enter NeedGenerateTexture");
        
        startTime = clock();
        
        // Gen Selection Image
        subStartTime = clock();
        generateSelectionImage();
        subEndTime = clock();
        LOGI("    selection Img gen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        // Gen Texture
        char resultBoundary[500];
        char resultTrans[500];
        char resultHead[500];
        char texInitDataDir[500];
        char faceTexOutPath[500];
        char bgTexOutPath[500];
        char VFMapFilePath[500];
        sprintf(texInitDataDir, "%s/TexGeneratorData", _DataDir);
        sprintf(faceTexOutPath, "%s/headTex.png", _WorkDir);
        sprintf(bgTexOutPath, "%s/backgroundTex.png", _WorkDir);
        sprintf(resultBoundary, "%s/boundary.txt", _WorkDir);
        sprintf(resultTrans, "%s/trans.txt", _WorkDir);
        sprintf(resultHead, "%s/head.obj", _WorkDir);
        sprintf(VFMapFilePath, "%s/TexGeneratorData/VFMap.dat", _DataDir);
        
        //char selectionAreaImageFile[500];
        //sprintf(selectionAreaImageFile, "%s/selectionArea.png", _WorkDir);
        //saveImageDataToFile(_SelectionImageData, _InputBitmapWidth, _InputBitmapHeight, selectionAreaImageFile);
        //_SelectionImageData = getImageDataAndInfoForImagePath(selectionAreaImageFile, NULL, NULL);
        
        if(_TexGenerator)
        {
            delete _TexGenerator;
            _TexGenerator = NULL;
        }
        
        subStartTime = clock();
        _TexGenerator = new TexGenerator(texInitDataDir);
        subEndTime = clock();
        LOGI("    init TexGen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        subStartTime = clock();
        _TexGenerator->generateFaceTextureFromImageWithGradient(_SelectionImageData, _inputPartialFaceImageData, _InputBitmapWidth, _InputBitmapHeight, resultHead, resultTrans, VFMapFilePath, faceTexOutPath, pmaxDiff);
        subEndTime = clock();
        LOGI("    face tex gen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        subStartTime = clock();
        _TexGenerator->generateBackgroundTextureFromImage(_SelectionImageData, _InputBitmapWidth, _InputBitmapHeight, bgTexOutPath);
        subEndTime = clock();
        LOGI("    bg tex gen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        _NeedGenerateTexture = false;
        
        endTime = clock();
        LOGI("Texture Gen: %f\n", ((float)(endTime - startTime)) / CLOCKS_PER_SEC);
        
//        int headTexWidth, headTexHeight;
//        const unsigned char * headTexData = _TexGenerator->getHeadTextureInfo(headTexWidth, headTexHeight);
//        char selectionAreaImageFile[500];
//        sprintf(selectionAreaImageFile, "%s/faceTexture.png", _WorkDir);
//        saveImageDataToFile(headTexData, headTexWidth, headTexHeight, selectionAreaImageFile);
//        LOGI("face texture saved.\n");
//        
//        int bgTexWidth, bgTexHeight;
//        const unsigned char * bgTexData = _TexGenerator->getBackgroundTextureInfo(bgTexWidth, bgTexHeight);
//        char bgImageFile[500];
//        sprintf(bgImageFile, "%s/bgTexture.png", _WorkDir);
//        saveImageDataToFile(bgTexData, bgTexWidth, bgTexHeight, bgImageFile);
//        LOGI("bg texture saved.\n");
    }
    
    return true;
}

bool HairEngine::initViewer(int width, int height)
{
	GLCheck(HairEngine::initViewer);
    if(!_TexGenerator)
    {
        LOGI("HairEngine: invalid _TexGenerator instance in initViewer\n");
        return false;
    }
    
   int headTexWidth, headTexHeight, bgTexWidth, bgTexHeight;
   const unsigned char * headTexData = _TexGenerator->getHeadTextureInfo(headTexWidth, headTexHeight);
   const unsigned char * bgTexData = _TexGenerator->getBackgroundTextureInfo(bgTexWidth, bgTexHeight);
    
//    char buffer[500];
//    sprintf(buffer,"%s/HairEngineInitData/TexGeneratorData/Laplacian_512.png", _DataDir);
//    headTexData = getImageDataAndInfoForImagePath(buffer, &headTexWidth, &headTexHeight);

    if(!headTexData || !bgTexData)
    {
        LOGE("HairEngine: invalid headTexData or bgTexData in initViewer\n");
        return false;
    }

    if(_Renderer)
    {
        _Renderer->releaseMemory();
        delete _Renderer;
        _Renderer = NULL;
    }
    
    char rendererInitDataDir[500];
    sprintf(rendererInitDataDir, "%s/RendererData", _DataDir);
    
#ifndef ExportData
    _Renderer = new Renderer(_WorkDir, rendererInitDataDir, rendererInitDataDir);
#else
	_Renderer = new Exporter(std::string(_WorkDir));
#endif
    _Renderer->setHeadTextureData(headTexData, headTexWidth, headTexHeight);
    _Renderer->setBackgroundTexutreData(bgTexData, bgTexWidth, bgTexHeight);
    _Renderer->setFaceImageData(_InputFaceImageData, _InputBitmapWidth, _InputBitmapHeight);
    _Renderer->initRenderer(width, height);
    
    return true;
}

bool HairEngine::closeViewer()
{
    if(_Renderer)
    {
        _Renderer->releaseMemory();
        delete _Renderer;
        _Renderer = NULL;
    }
    return true;
}

bool HairEngine::resizeViewer(int width, int height)
{
    if(!_Renderer)
    {
        LOGI("HairEngine: invalid _Renderer in resizeViewer\n");
        return false;
    }
    
    _Renderer->resizeViewport(width, height);
    
    return true;
}

bool HairEngine::setHairDir(const char * hairDir)
{
    if(!_Renderer)
    {
        LOGI("HairEngine: invalid _Renderer in setHairDir\n");
        return false;
    }
    
    _Renderer->changeHairStyle(hairDir);

    //recover hair color
    _Renderer->changeHairColor(0.0, 0.0, 0.0, 0.0);

    //testing code
    //enableExpression(true);
    //_Renderer->changeModifier(1);
    //_Renderer->changeExpression(0);
    //test
//    _Renderer->changeHairColor(0.8, 0.5, 0.5, 0.2);
//    srand(time(0));
//    _Renderer->changeHairColor((float)rand()/RAND_MAX, (float)rand()/RAND_MAX, (float)rand()/RAND_MAX, 0.2);
    
    return true;
}

bool HairEngine::setTransform(float xDegree, float yDegree, float scale)
{
    if(!_Renderer)
    {
        LOGI("HairEngine: invalid _Renderer in setTransform\n");
        return false;
    }
    
    _Renderer->setRotation(xDegree, yDegree);
    _Renderer->setScale(scale);
    _Renderer->updateMatrixes();
    
    return true;
}

bool HairEngine::resetTransform()
{
    float degree = 0.0f;
    float scale = 1.0f;
    
    if(!_Renderer)
    {
        LOGI("HairEngine: invalid _Renderer in resetTransform\n");
        return false;
    }
    
    _Renderer->setRotation(degree, degree);
    _Renderer->setScale(scale);
    _Renderer->updateMatrixes();
    
    return true;
}

bool HairEngine::adjustHairPosition(float disX, float disY, float disZ)
{
    if(!_Renderer)
    {
        LOGI("HairEngine: invalid _Renderer in adjustHairPosition\n");
        return false;
    }
    
    _Renderer->setHairPositionAdjustVector(disX, disY, disZ);
    _Renderer->updateMatrixes();
    
    return true;
}

bool HairEngine::adjustHairScale(float scale)
{
	if(!_Renderer)
	{
		LOGI("HairEngine: invalid _Renderer in adjustHairScale\n");
		return false;
	}

	_Renderer->setHairScale(scale);
	_Renderer->updateMatrixes();

	return true;
}

bool HairEngine::resetHairPosition()
{
    if(!_Renderer)
    {
        LOGI("HairEngine: invalid _Renderer in resetHairPosition\n");
        return false;
    }
    
    _Renderer->setHairPositionAdjustVector(0.0f, 0.0f, 0.0f);
    _Renderer->setHairScale(1.0f);
    _Renderer->updateMatrixes();
    
    return true;
}

bool HairEngine::enableShadow(bool enable)
{
    return true;
}

bool HairEngine::enableExpression(bool enable)
{
    if(!_Renderer)
    {
        LOGI("HairEngine: invalid _Renderer in enableExpression\n");
        return false;
    }
    
    _Renderer->enableExpression(enable);
    
    return true;
}

bool HairEngine::render()
{
    if(!_Renderer)
    {
        LOGI("HairEngine: invalid _Renderer\n");
        return false;
    }
    
    _Renderer->render();
    
    return true;
}

bool HairEngine::setDermabrasionDegree(int degree)
{
	_dermabrasionDegree = degree;
}

bool HairEngine::autoGenerateHead(float pmaxDiff)
{
	if(_NeedDetectFace)
		if(!detectFace())
		{
			LOGI("HairEngine: face detect failed\n");
			return false;
		}
	if(_NeedGenerateHeadModel)
		if(!generateHead())
		{
			LOGI("HairEngine: head generate failed\n");
			return false;
		}

	if(_NeedGenerateTexture)
		if(!generateTexture())
		{
			LOGI("HairEngine: texture generate failed\n");
			return false;
		}

	LOGI("HairEngine: auto generate finished and successes\n");

	return true;
}

bool HairEngine::detectFace()
{
	if( (
	#ifdef UseRegTest
	          !_UseRegressor &&
	#endif
	         !_HeadDetector)
	#ifdef UseRegTest
	       || (_UseRegressor && !_RegTest)
	#endif
	       )
	    {
	        LOGI("HairEngine: invalid _HeadDetector or _RegTest in finishStroke\n");
	        return false;
	    }
	    if(!_InputBitmapData)
	    {
	        LOGI("HairEngine: invalid _InputBitmapData instance in finishStrokeStep1\n");
	        return false;
	    }

	//Head Detector
	cv::Mat featureMat;
#ifdef UseRegTest
	if(!_UseRegressor)
	{
#endif
		IplImage * iplImg = convertImageData2IplImage(_InputBitmapData, _InputBitmapWidth, _InputBitmapHeight);
		_HeadDetector->setOriginalImage(iplImg);
		cvReleaseImage(&iplImg);
		if(!_HeadDetector->runDetect())
			return false;
		featureMat = _HeadDetector->getResultFeatures();

#ifdef UseRegTest
	}
	else
	{
		int num = _InputBitmapWidth * _InputBitmapHeight;
		unsigned char * tmpImageData4 = new unsigned char[num * 4];
		memcpy(tmpImageData4, _InputBitmapData, num * 4);
		flipImageData(tmpImageData4, _InputBitmapWidth, _InputBitmapHeight);
		unsigned char * tmpImageData3 = new unsigned char[num * 3];
		int index4 = 0;
		int index3 = 0;
		for(int i=0; i<num; i++)
		{
			tmpImageData3[index3] = tmpImageData4[index4];
			tmpImageData3[index3 + 1] = tmpImageData4[index4 + 1];
			tmpImageData3[index3 + 2] = tmpImageData4[index4 + 2];

			index3 += 3;
			index4 += 4;
		}

		_RegTest->setImgData(tmpImageData3, _InputBitmapWidth, _InputBitmapHeight);
		if(!_RegTest->testFirstRegress())
			return false;
		featureMat = _RegTest->getLandmarks();

		delete [] tmpImageData4;
		delete [] tmpImageData3;
	}
#endif

	_NeedDetectFace = false;

	return true;
}

bool HairEngine::generateHead()
{
	char featureLocationFile[500];
	char trainResultFile[500];
	char auxiliaryDataFile[500];
	char faceTexAndTopoFile[500];
	char foreheadVertexIndexFile[500];
	char resultBoundary[500];
	char resultTrans[500];
	char resultHead[500];
	sprintf(featureLocationFile, "%s/HeadGenData/featureLocation.txt", _DataDir);
	sprintf(trainResultFile, "%s/HeadGenData/trainResult.dat", _DataDir);
	sprintf(auxiliaryDataFile, "%s/HeadGenData/auxiliaryData.txt", _DataDir);
	sprintf(faceTexAndTopoFile, "%s/HeadGenData/faceTexAndTopo.txt", _DataDir);
	sprintf(foreheadVertexIndexFile, "%s/HeadGenData/foreheadVertexIndex.txt", _DataDir);
	sprintf(resultBoundary, "%s/boundary.txt", _WorkDir);
	sprintf(resultTrans, "%s/trans.txt", _WorkDir);
	sprintf(resultHead, "%s/head.obj", _WorkDir);

	cv::Mat featureMat = getFeatureMat();

	HeadGenerator * headGen = new HeadGenerator(featureLocationFile, trainResultFile, auxiliaryDataFile, foreheadVertexIndexFile);
	headGen->reset();
	headGen->setFeatures(&featureMat, _InputBitmapWidth, _InputBitmapHeight);
	headGen->runHeadGenerate();
	headGen->save(faceTexAndTopoFile,resultBoundary, resultTrans, resultHead);
	delete headGen;

	evaluateFaceImageFromInputImage();
	evaluatePartialFaceImageFromInputImage();

	_NeedGenerateHeadModel = false;

	return true;
}

bool HairEngine::generateTexture(float pmaxDiff)
{
	// Gen Selection Image
	generateSelectionImage();

	// Gen Texture
	char resultBoundary[500];
	char resultTrans[500];
	char resultHead[500];
	char texInitDataDir[500];
	char faceTexOutPath[500];
	char bgTexOutPath[500];
	char VFMapFilePath[500];
	sprintf(texInitDataDir, "%s/TexGeneratorData", _DataDir);
	sprintf(faceTexOutPath, "%s/headTex.png", _WorkDir);
	sprintf(bgTexOutPath, "%s/backgroundTex.png", _WorkDir);
	sprintf(resultBoundary, "%s/boundary.txt", _WorkDir);
	sprintf(resultTrans, "%s/trans.txt", _WorkDir);
	sprintf(resultHead, "%s/head.obj", _WorkDir);
	sprintf(VFMapFilePath, "%s/TexGeneratorData/VFMap.dat", _DataDir);

	if(_TexGenerator)
	{
		delete _TexGenerator;
		_TexGenerator = NULL;
	}

	_TexGenerator = new TexGenerator(texInitDataDir);

	_TexGenerator->generateFaceTextureFromImageWithGradient(_InputFaceImageData, _inputPartialFaceImageData, _InputBitmapWidth, _InputBitmapHeight, resultHead, resultTrans, VFMapFilePath, faceTexOutPath, pmaxDiff);
	//_TexGenerator->generateFaceTextureFromImageWithGradient(_SelectionImageData, _inputPartialFaceImageData, _InputBitmapWidth, _InputBitmapHeight, resultHead, resultTrans, VFMapFilePath, faceTexOutPath, pmaxDiff);
	//_TexGenerator->generateFaceTextureFromImage(_SelectionImageData,  _InputBitmapWidth, _InputBitmapHeight, resultHead, resultTrans, VFMapFilePath, faceTexOutPath);

	_TexGenerator->generateBackgroundTextureFromImage(_SelectionImageData, _InputBitmapWidth, _InputBitmapHeight, bgTexOutPath);

	_NeedGenerateTexture = false;

	return true;
}

bool HairEngine::finishStrokeStep1()
{
    clock_t startTime, endTime, subStartTime, subEndTime;
    
    if( (
#ifdef UseRegTest        
          !_UseRegressor &&
#endif
         !_HeadDetector)
#ifdef UseRegTest
       || (_UseRegressor && !_RegTest)
#endif
       )
    {
        LOGI("HairEngine: invalid _HeadDetector or _RegTest in finishStroke\n");
        return false;
    }
    if(!_InputBitmapData)
    {
        LOGI("HairEngine: invalid _InputBitmapData instance in finishStrokeStep1\n");
        return false;
    }
    
    if(_NeedGenerateHeadModel)
    {
        startTime = subStartTime = clock();
        
        //Head Detector
        cv::Mat featureMat;
#ifdef UseRegTest
        if(!_UseRegressor)
        {
#endif
            IplImage * iplImg = convertImageData2IplImage(_InputBitmapData, _InputBitmapWidth, _InputBitmapHeight);
            _HeadDetector->setOriginalImage(iplImg);
            cvReleaseImage(&iplImg);
            if(!_HeadDetector->runDetect())
                return false;
            featureMat = _HeadDetector->getResultFeatures();
#ifdef UseRegTest
        }
        else
        {
            int num = _InputBitmapWidth * _InputBitmapHeight;
            unsigned char * tmpImageData4 = new unsigned char[num * 4];
            memcpy(tmpImageData4, _InputBitmapData, num * 4);
            flipImageData(tmpImageData4, _InputBitmapWidth, _InputBitmapHeight);
            unsigned char * tmpImageData3 = new unsigned char[num * 3];
            int index4 = 0;
            int index3 = 0;
            for(int i=0; i<num; i++)
            {
                tmpImageData3[index3] = tmpImageData4[index4];
                tmpImageData3[index3 + 1] = tmpImageData4[index4 + 1];
                tmpImageData3[index3 + 2] = tmpImageData4[index4 + 2];
                
                index3 += 3;
                index4 += 4;
            }
            
            _RegTest->setImgData(tmpImageData3, _InputBitmapWidth, _InputBitmapHeight);
            if(!_RegTest->testFirstRegress())
                return false;
            featureMat = _RegTest->getLandmarks();
            
            delete [] tmpImageData4;
            delete [] tmpImageData3;
        }
#endif
        subEndTime = clock();
        LOGI("    head Det: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        subStartTime = clock();
        //Head Generator
        char featureLocationFile[500];
        char trainResultFile[500];
        char auxiliaryDataFile[500];
        char faceTexAndTopoFile[500];
        char foreheadVertexIndexFile[500];
        char resultBoundary[500];
        char resultTrans[500];
        char resultHead[500];
        sprintf(featureLocationFile, "%s/HeadGenData/featureLocation.txt", _DataDir);
        sprintf(trainResultFile, "%s/HeadGenData/trainResult.dat", _DataDir);
        sprintf(auxiliaryDataFile, "%s/HeadGenData/auxiliaryData.txt", _DataDir);
        sprintf(faceTexAndTopoFile, "%s/HeadGenData/faceTexAndTopo.txt", _DataDir);
        sprintf(foreheadVertexIndexFile, "%s/HeadGenData/foreheadVertexIndex.txt", _DataDir);
        sprintf(resultBoundary, "%s/boundary.txt", _WorkDir);
        sprintf(resultTrans, "%s/trans.txt", _WorkDir);
        sprintf(resultHead, "%s/head.obj", _WorkDir);
        
        HeadGenerator * headGen = new HeadGenerator(featureLocationFile, trainResultFile, auxiliaryDataFile, foreheadVertexIndexFile);
        headGen->reset();
        headGen->setFeatures(&featureMat, _InputBitmapWidth, _InputBitmapHeight);
        headGen->runHeadGenerate();
        headGen->save(faceTexAndTopoFile,resultBoundary, resultTrans, resultHead);
        delete headGen;
        
        subEndTime = clock();
        LOGI("    head Gen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        subStartTime = clock();
        evaluateFaceImageFromInputImage();
        evaluatePartialFaceImageFromInputImage();
        subEndTime = clock();
        LOGI("    face & partial image eva: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        _NeedGenerateHeadModel = false;
        
        endTime = clock();
        LOGI("Data Gen: %f\n", ((float)(endTime - startTime)) / CLOCKS_PER_SEC);
        
        IplImage * tmpImage = convertImageData2IplImage(_InputBitmapData, _InputBitmapWidth, _InputBitmapHeight);
        for(int i=0; i<OldLandmarksNum; i++)
        {
            cv::Vec2f point = featureMat.at<cv::Vec2f>(0, i);
            cvCircle(tmpImage, cvPoint(point[0], point[1]), 2, cvScalar(0, 0, 255, 255), -1, 4);
        }
        
        char path[500];
        sprintf(path, "%s/faceRectDetect.png", _WorkDir);
        cvSaveImage(path, tmpImage);
    }
    
    return true;
}

bool HairEngine::finishStrokeStep2(float pmaxDiff)
{
    clock_t startTime, endTime, subStartTime, subEndTime;
    
    if(_NeedGenerateHeadModel)
    {
        LOGI("HairEngine: need to generate head data before finishStrokeStep2\n");
        return false;
    }
    
    if(_NeedGenerateTexture)
    {
        startTime = clock();
        
        // Gen Selection Image
        subStartTime = clock();
        generateSelectionImage();
        subEndTime = clock();
        LOGI("    selection Img gen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        // Gen Texture
        char resultBoundary[500];
        char resultTrans[500];
        char resultHead[500];
        char texInitDataDir[500];
        char faceTexOutPath[500];
        char bgTexOutPath[500];
        char VFMapFilePath[500];
        sprintf(texInitDataDir, "%s/TexGeneratorData", _DataDir);
        sprintf(faceTexOutPath, "%s/headTex.png", _WorkDir);
        sprintf(bgTexOutPath, "%s/backgroundTex.png", _WorkDir);
        sprintf(resultBoundary, "%s/boundary.txt", _WorkDir);
        sprintf(resultTrans, "%s/trans.txt", _WorkDir);
        sprintf(resultHead, "%s/head.obj", _WorkDir);
        sprintf(VFMapFilePath, "%s/TexGeneratorData/VFMap.dat", _DataDir);
        
        //char selectionAreaImageFile[500];
        //sprintf(selectionAreaImageFile, "%s/selectionArea.png", _WorkDir);
        //saveImageDataToFile(_SelectionImageData, _InputBitmapWidth, _InputBitmapHeight, selectionAreaImageFile);
        //_SelectionImageData = getImageDataAndInfoForImagePath(selectionAreaImageFile, NULL, NULL);
        
        if(_TexGenerator)
        {
            delete _TexGenerator;
            _TexGenerator = NULL;
        }
        
        subStartTime = clock();
        _TexGenerator = new TexGenerator(texInitDataDir);
        subEndTime = clock();
        LOGI("    init TexGen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        subStartTime = clock();
        _TexGenerator->generateFaceTextureFromImageWithGradient(_SelectionImageData, _inputPartialFaceImageData, _InputBitmapWidth, _InputBitmapHeight, resultHead, resultTrans, VFMapFilePath, faceTexOutPath, pmaxDiff);
        //_TexGenerator->generateFaceTextureFromImage(_SelectionImageData,  _InputBitmapWidth, _InputBitmapHeight, resultHead, resultTrans, VFMapFilePath, faceTexOutPath);

        subEndTime = clock();
        LOGI("    face tex gen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        subStartTime = clock();
        _TexGenerator->generateBackgroundTextureFromImage(_SelectionImageData, _InputBitmapWidth, _InputBitmapHeight, bgTexOutPath);
        subEndTime = clock();
        LOGI("    bg tex gen: %f\n", ((float)(subEndTime - subStartTime)) / CLOCKS_PER_SEC);
        
        _NeedGenerateTexture = false;
        
        endTime = clock();
        LOGI("Texture Gen: %f\n", ((float)(endTime - startTime)) / CLOCKS_PER_SEC);
    }
    
    return true;
}

cv::Mat & HairEngine::getFeatureMat()
{
    cv::Mat features;
#ifdef UseRegTest
    if(!_UseRegressor)
#endif
        return _HeadDetector->getResultFeatures();
#ifdef UseRegTest
    else
        return _RegTest->getLandmarks();
#endif
}

//private methods
IplImage * HairEngine::convertImageData2IplImage(const unsigned char * srcBitmap, int srcWidth, int srcHeight)
{
    IplImage * srcImage = cvCreateImage(cvSize(srcWidth, srcHeight),IPL_DEPTH_8U,4);
    unsigned char * data = (uchar *)srcImage->imageData;
    
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
            arrayIndex += nPixelChannel;
        }
    
    return srcImage;
}

void HairEngine::generateSelectionImage()
{
    LOGI("gen Selection Img 1");
    
    memcpy(_SelectionImageData, _InputBitmapData, _InputBitmapWidth * _InputBitmapHeight * nPixelChannel);
    
    LOGI("gen Selection Img 2");
    
    if(!_PaintSelection)
        return;
    
    unsigned char * maskArrayForHair = _PaintSelection->getFinalMask();
    
    if(maskArrayForHair)
    {
        LOGI("gen Selection Img 3");
        
        //鑳屾櫙涓洪粦鑹�0x00 鍓嶆櫙涓虹櫧鑹�0xff锛屾墍浠ヤ娇鐢╠ilation鑶ㄨ儉锛岀櫧澧為粦鍑�
        IplImage * src = cvCreateImage(cvSize(_InputBitmapWidth, _InputBitmapHeight), IPL_DEPTH_8U, 1);
        LOGI("gen Selection Img 4");
        IplImage * dilation = cvCreateImage(cvSize(_InputBitmapWidth, _InputBitmapHeight), IPL_DEPTH_8U, 1);
        LOGI("gen Selection Img 5");
        if(src->imageData && dilation->imageData)
        {
            LOGI("gen Selection Img 6");
            int arrayIndex;
            unsigned char * srcData = (unsigned char *)src->imageData;
            int step = src->widthStep;
            int nChannels = src->nChannels;
            arrayIndex = 0;
            for(int i=0; i<_InputBitmapHeight; i++)
                for(int j=0; j<_InputBitmapWidth; j++)
                    srcData[i*step + j*nChannels] = maskArrayForHair[arrayIndex++];
            LOGI("gen Selection Img 7");
            
            IplConvKernel * element = cvCreateStructuringElementEx(2*DilationSize+1, 2*DilationSize+1, DilationSize, DilationSize, cv::MORPH_RECT);
            LOGI("gen Selection Img 8");
            cvDilate(src, dilation, element);
            LOGI("gen Selection Img 9");
            cvReleaseStructuringElement(&element);
            LOGI("gen Selection Img 10");
            
            cv::Mat features;
            
            LOGI("gen Selection Img 11");
#ifdef UseRegTest
            if(!_UseRegressor)
#endif
                features = _HeadDetector->getResultFeatures();
#ifdef UseRegTest
            else
                features = _RegTest->getLandmarks();
#endif
            LOGI("gen Selection Img 12");
            
            std::vector< std::vector<cv::Point> > points(1);
            std::vector< std::vector<cv::Point> > hull(1);
            LOGI("gen Selection Img 13");
            points[0].resize(FeatureNumber);
            for (int featureI = 0; featureI < FeatureNumber; featureI++)
                points[0][featureI] = cv::Point(features.at<cv::Vec2f>(0, featureI)[0], features.at<cv::Vec2f>(0, featureI)[1]);
            cv::convexHull(points[0], hull[0]);
            cv::Mat faceMask = cv::Mat::zeros(_InputBitmapHeight, _InputBitmapWidth, CV_8UC1);
            cv::drawContours(faceMask, hull, 0, cv::Scalar(255), CV_FILLED);
            cv::Mat faceElement = cv::getStructuringElement(cv::MORPH_RECT,
                                                        cv::Size(2*FaceDilationSize+1, 2*FaceDilationSize+1),
                                                        cv::Point(FaceDilationSize, FaceDilationSize));
            cv::Mat dialateFaceMask = cv::Mat::zeros(_InputBitmapHeight, _InputBitmapWidth, CV_8UC1);
            cv::dilate(faceMask, dialateFaceMask, faceElement);
            
            LOGI("gen Selection Img 14");
            
            step = dilation->widthStep;
            nChannels = dilation->nChannels;
            unsigned char * srcDilationData = (unsigned char *)dilation->imageData;
            arrayIndex = 3;
            for(int i=0;i<_InputBitmapHeight;i++)
                for(int j=0;j<_InputBitmapWidth;j++)
                {
                    //for pixel (j, i)
                    int iplImageIndex = i*step + j*nChannels;
                    if(srcData[iplImageIndex] || (srcDilationData[iplImageIndex] && dialateFaceMask.at<uchar>(i, j) == 0x00))
                    {
                        //when it belongs to foreground
                        _SelectionImageData[arrayIndex] = 0x00;
                    }
                    
                    arrayIndex +=4;
                }
        LOGI("gen Selection Img 15");
            
//            unsigned char * tmpImage = new unsigned char[_InputBitmapWidth * _InputBitmapHeight * nPixelChannel];
//            memcpy(tmpImage, _InputBitmapData, _InputBitmapWidth * _InputBitmapHeight * nPixelChannel);
//            arrayIndex = 3;
//            for(int i=0;i<_InputBitmapHeight;i++)
//                for(int j=0;j<_InputBitmapWidth;j++)
//                {
//                    //for pixel (j, i)
//                    int iplImageIndex = i*step + j*nChannels;
//                    if(srcData[iplImageIndex])
//                    {
//                        //when it belongs to foreground
//                        tmpImage[arrayIndex] = 0x00;
//                    }
//                    arrayIndex +=4;
//                }
//            char srcSelectionFilePath[500];
//            sprintf(srcSelectionFilePath, "%s/srcSelection.png", _WorkDir);
//            saveImageDataToFile(tmpImage, _InputBitmapWidth, _InputBitmapHeight, srcSelectionFilePath);
//            
//            memcpy(tmpImage, _InputBitmapData, _InputBitmapWidth * _InputBitmapHeight * nPixelChannel);
//            arrayIndex = 0;
//            for(int i=0;i<_InputBitmapHeight;i++)
//                for(int j=0;j<_InputBitmapWidth;j++)
//                {
//                    //for pixel (j, i)
//                    int iplImageIndex = i*step + j*nChannels;
//                    if(srcDilationData[iplImageIndex])
//                    {
//                        //when it belongs to foreground
//                        if(dialateFaceMask.at<uchar>(i, j) == 0xff)
//                        {
//                            tmpImage[arrayIndex] = (unsigned char)(255);
//                            tmpImage[arrayIndex+1] = (unsigned char)(255);
//                            tmpImage[arrayIndex+2] = (unsigned char)(255);
//                        }
//                        else
//                        {
//                            tmpImage[arrayIndex] = (unsigned char)(_SelectionImageData[arrayIndex] * 0 + 255 * 0.5f);
//                            tmpImage[arrayIndex+1] = (unsigned char)(_SelectionImageData[arrayIndex+1] * 0.5f);
//                            tmpImage[arrayIndex+2] = (unsigned char)(_SelectionImageData[arrayIndex+2] * 0.5f);
//                        }
//                    }
//                    else if(faceMask.at<uchar>(i, j) == 0xff)
//                    {
//                        tmpImage[arrayIndex] = (unsigned char)(_SelectionImageData[arrayIndex] * 0.5);
//                        tmpImage[arrayIndex+1] = (unsigned char)(_SelectionImageData[arrayIndex+1] * 0.5 + 255 * 0.5f);
//                        tmpImage[arrayIndex+2] = (unsigned char)(_SelectionImageData[arrayIndex+2] * 0.5);
//                    }
//                    else if(dialateFaceMask.at<uchar>(i, j) == 0xff)
//                    {
//                        tmpImage[arrayIndex] = (unsigned char)(_SelectionImageData[arrayIndex] * 0.5);
//                        tmpImage[arrayIndex+1] = (unsigned char)(_SelectionImageData[arrayIndex+1] * 0.5);
//                        tmpImage[arrayIndex+2] = (unsigned char)(_SelectionImageData[arrayIndex+2] * 0.5 + 255 * 0.5f);
//                    }
//                    
//                    arrayIndex +=4;
//                }
//            char diaSelectionFilePath[500];
//            sprintf(diaSelectionFilePath, "%s/diaSelection.png", _WorkDir);
//            saveImageDataToFile(tmpImage, _InputBitmapWidth, _InputBitmapHeight, diaSelectionFilePath);
//            
//            delete [] tmpImage;
            
            cvReleaseImage(&src);
            cvReleaseImage(&dilation);
            faceMask.release();
            dialateFaceMask.release();
            faceElement.release();
            LOGI("gen Selection Img 16");
        }
    }
    
    //TODO selection image
    char rstSelectionFilePath[500];
    sprintf(rstSelectionFilePath, "%s/rstSelection.png", _WorkDir);
    saveImageDataToFile(_SelectionImageData, _InputBitmapWidth, _InputBitmapHeight, rstSelectionFilePath);
}

void HairEngine::evaluateFaceImageFromInputImage()
{
    memcpy(_InputFaceImageData, _InputBitmapData, _InputBitmapWidth * _InputBitmapHeight * nPixelChannel);
    
    cv::Mat features;
#ifdef UseRegTest
    if(!_UseRegressor)
#endif
        features = _HeadDetector->getResultFeatures();
#ifdef UseRegTest
    else
        features = _RegTest->getLandmarks();
#endif
    
    std::vector< std::vector<cv::Point> > points(1);
	std::vector< std::vector<cv::Point> > hull(1);
    
    points[0].resize(FeatureNumber);
    for (int featureI = 0; featureI < FeatureNumber; featureI++)
		points[0][featureI] = cv::Point(features.at<cv::Vec2f>(0, featureI)[0], features.at<cv::Vec2f>(0, featureI)[1]);
	cv::convexHull(points[0], hull[0]);
    cv::Mat faceMask = cv::Mat::zeros(_InputBitmapHeight, _InputBitmapWidth, CV_8UC1);
    cv::drawContours(faceMask, hull, 0, cv::Scalar(255), CV_FILLED);
    int arrayIndex = 3;
    for (int hI = 0; hI < _InputBitmapHeight; hI++)
    {
		for (int wI = 0; wI < _InputBitmapWidth; wI++)
        {
			if (faceMask.at<uchar>(hI, wI) < 0xff)
				_InputFaceImageData[arrayIndex] = 0x00;
            arrayIndex += nPixelChannel;
		}
	}
    
    //reverse face
    //memcpy(_InputBitmapData, _InputFaceImageData , _InputBitmapWidth * _InputBitmapHeight * nPixelChannel);

    char tmpPath[500];
    sprintf(tmpPath, "%s/face.png", _WorkDir);
    saveImageDataToFile(_InputFaceImageData, _InputBitmapWidth, _InputBitmapHeight, tmpPath);
}

void HairEngine::evaluatePartialFaceImageFromInputImage()
{
    memcpy(_inputPartialFaceImageData, _InputBitmapData, _InputBitmapWidth * _InputBitmapHeight * nPixelChannel);
    
    cv::Mat features;
#ifdef UseRegTest
    if(!_UseRegressor)
#endif
        features = _HeadDetector->getResultFeatures();
#ifdef UseRegTest
    else
        features = _RegTest->getLandmarks();
#endif
    
    std::vector< std::vector<cv::Point> > points(1);

    points[0].resize(PartialFeatureNumber);
    for (int featureI = 0; featureI < PartialFeatureNumber; featureI++)
    {
        int tmpIndex = PartialFaceFeaturePoints[featureI];
		points[0][featureI] = cv::Point(features.at<cv::Vec2f>(0, tmpIndex)[0], features.at<cv::Vec2f>(0, tmpIndex)[1]);
    }
    
    cv::Mat faceMask = cv::Mat::zeros(_InputBitmapHeight, _InputBitmapWidth, CV_8UC1);
    cv::drawContours(faceMask, points, 0, cv::Scalar(255), CV_FILLED);
    int arrayIndex = 3;
    for (int hI = 0; hI < _InputBitmapHeight; hI++)
    {
		for (int wI = 0; wI < _InputBitmapWidth; wI++)
        {
			if (faceMask.at<uchar>(hI, wI) < 0xff)
				_inputPartialFaceImageData[arrayIndex] = 0x00;
            arrayIndex += nPixelChannel;
		}
	}

    /*
    char tmpPath[500];
    sprintf(tmpPath, "%s/particalface.png", _WorkDir);
    saveImageDataToFile(_inputPartialFaceImageData, _InputBitmapWidth, _InputBitmapHeight, tmpPath);
    */
}

bool HairEngine::changeExpression(int expressionID)
{
	if(!_Renderer)
	{
		LOGI("HairEngine: invalid _Renderer in change expression\n");
		return false;
	}

	//if(expressionID < 0 || expressionID > 1)
	//{
	//	LOGI("HairEngine: invalid expression ID in change expression, 0, 1 supported \n");
	//	return false;
	//}

	_Renderer->changeModifier(-1); //停止小表情
	_Renderer->changeExpression(expressionID);
	return true;
}

bool HairEngine::changeModifier(int modifierID)
{
	if(!_Renderer)
	{
		LOGI("HairEngine: invalid _Renderer in change modifier\n");
		return false;
	}

	_Renderer->changeExpression(-1); //停止大表情
	_Renderer->changeModifier(modifierID);
	return true;
}

bool HairEngine::resetExpression()
{
	if(!_Renderer)
	{
		LOGI("HairEngine: invalid _Renderer in reset expression\n");
		return false;
	}

	_Renderer->resetExpression();
	return true;
}

bool HairEngine::changeHairColor(float r, float g, float b, float a)
{
	if(!_Renderer)
	{
		LOGI("HairEngine: invalid _Renderer in change hair color\n");
		return false;
	}

	_Renderer->changeHairColor(r,g,b,a);
	return true;
}

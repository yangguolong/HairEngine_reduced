//
//  Renderer.cpp
//  Demo
//
//  Created by yangguolong on 13-7-15.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "Renderer.h"
#include "ImageUtility.h"
#include "ShaderUtility.h"
#include "HeadDeformer.h"
#include "AnimDeformer.h"

#include <android/log.h>
#define LOG_TAG "Renderer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,##__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,##__VA_ARGS__)

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define SelectedHead 4

#define PI 3.1415926
#define ROTATIONX 12
#define ROTATIONY 15
#define SCALEMIN  1.0f
#define SCALEMAX  1.3f

#define ShaderSourceFileSize 1024*50  // 50k

enum
{
    QUAD_UNIFORM_MODELVIEWPROJECTION_MATRIX,
    QUAD_UNIFORM_SAMPLER,
    QUAD_NUM_UNIFORMS
};
GLuint uniforms4Quad[QUAD_NUM_UNIFORMS];

enum
{
    BG_UNIFORM_MODELVIEWPROJECTION_MATRIX,
    BG_UNIFORM_SAMPLER,
    BG_NUM_UNIFORMS
};
GLuint uniforms4BG[BG_NUM_UNIFORMS];

enum
{
    HEAD_UNIFORM_MODELVIEWPROJECTION_MATRIX,
    HEAD_UNIFORM_SAMPLER,
    HEAD_NUM_UNIFORMS
};
GLuint uniforms4Head[HEAD_NUM_UNIFORMS];

enum
{
    SURFACE_UNIFORM_MODELVIEWPROJECTION_MATRIX,
    SURFACE_UNIFORM_IMAGEWIDTH,
    SURFACE_UNIFORM_IMAGEHEIGHT,
    SURFACE_UNIFORM_SAMPLER,
    SUFRACE_UNIFORM_BLEND_COLOR,
    SURFACE_NUM_UNIFORMS
};
GLuint uniforms4Surface[SURFACE_NUM_UNIFORMS];

typedef struct
{
    float position[3];
    float texCoord[2];
    
} Vertex;

Vertex myQuad[6] = {
    {-1.0f, -1.0f, -1.0f,    0.0f, 0.0f},
    {1.0f, -1.0f, -1.0f,     1.0f, 0.0f},
    {1.0f, 1.0f, -1.0f,      1.0f, 1.0f},
    
    {1.0f, 1.0f, -1.0f,      1.0f, 1.0f},
    {-1.0f, 1.0f, -1.0f,     0.0f, 1.0f},
    {-1.0f, -1.0f, -1.0f,    0.0f, 0.0f}
};

Renderer::Renderer(const char * headDataDir, const char * shaderFileDir, const char * expressionFileDir)
{
    int size = strlen(headDataDir);
    _HeadDataDir = new char[size + 3];
    strcpy(_HeadDataDir, headDataDir);
    
    size = strlen(shaderFileDir);
    _ShaderFileDir = new char[size + 3];
    strcpy(_ShaderFileDir, shaderFileDir);
    
    size = strlen(expressionFileDir);
    _ExpressionFileDir = new char[size + 3];
    strcpy(_ExpressionFileDir, expressionFileDir);
    
    _HeadTextureData = NULL;
    _BGTextureData = NULL;
    _FaceImageData = NULL;
    _ScreenShotData = NULL;
    
    glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
    glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC) eglGetProcAddress("glBindVertexArrayOES");
    glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC) eglGetProcAddress("glDeleteVertexArraysOES");
}

Renderer::~Renderer()
{
    tearDownGL();
    
    if(_HeadDataDir)
    {
        delete [] _HeadDataDir;
        _HeadDataDir = NULL;
    }
    if(_ShaderFileDir)
    {
        delete [] _ShaderFileDir;
        _ShaderFileDir = NULL;
    }
    if(_ExpressionFileDir)
    {
        delete [] _ExpressionFileDir;
        _ExpressionFileDir = NULL;
    }
    
    if(_HeadTextureData)
    {
        delete [] _HeadTextureData;
        _HeadTextureData = NULL;
    }
    if(_BGTextureData)
    {
        delete [] _BGTextureData;
        _BGTextureData = NULL;
    }
    if(_FaceImageData)
    {
        delete [] _FaceImageData;
        _FaceImageData = NULL;
    }
    
    if(_ScreenShotData)
    {
        delete [] _ScreenShotData;
        _ScreenShotData = NULL;
    }
}

// Interface
void Renderer::setHeadTextureData(const unsigned char * inputImageData, int width, int height)
{
    if(_HeadTextureData)
    {
        delete [] _HeadTextureData;
        _HeadTextureData = NULL;
    }
    
    _HeadTextureData = new unsigned char[width*height*4];
    memcpy(_HeadTextureData, inputImageData, width*height*4);
    _HeadTextureWidth = width;
    _HeadTextureHeight = height;
}

void Renderer::setBackgroundTexutreData(const unsigned char * inputImageData, int width, int height)
{
    if(_BGTextureData)
    {
        delete [] _BGTextureData;
        _BGTextureData = NULL;
    }
    
    _BGTextureData = new unsigned char[width*height*4];
    memcpy(_BGTextureData, inputImageData, width*height*4);
    _BGTextureWidth = width;
    _BGTextureHeight = height;
}

void Renderer::setFaceImageData(const unsigned char * inputImageData, int width, int height)
{
    if(_FaceImageData)
    {
        delete [] _FaceImageData;
        _FaceImageData = NULL;
    }
    
    _FaceImageData = new unsigned char[width * height * 4];
    memcpy(_FaceImageData, inputImageData, width*height*4);
    _FaceImageWidth = width;
    _FaceImageHeight = height;
    
//    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask ,YES);
//    NSString *documentsDirectory = paths[0];
//    NSString * drawingsDirectory = [documentsDirectory stringByAppendingPathComponent:@"work"];
//    
//    const char * preBanImagePath = [[[drawingsDirectory stringByAppendingPathComponent:@"faceInImage"] stringByAppendingPathExtension:@"png"] UTF8String];
//    saveImageDataToFile(_FaceImageData, _FaceImageWidth, _FaceImageHeight, preBanImagePath);
} 

void Renderer::initRenderer(int windowWidth, int windowHeight)
{
	GLCheck(Renderer::initRenderer);
    _WindowWidth = windowWidth;
    _WindowHeight = windowHeight;
    
    initData();
    initImageBalancer();
    setupGL();
    setupModel();
    
//	LOGI("GLExtenions: %s\n", glGetString(GL_EXTENSIONS));
//	LOGI("GLVersion: %s\n", glGetString(GL_VERSION));
//	LOGI("GLVendor: %s\n", glGetString(GL_VENDOR));
//	LOGI("GLRenderer: %s\n", glGetString(GL_RENDERER));
}

void Renderer::setRotation(float & xAngle, float & yAngle)
{
    _RotateX = xAngle;
    _RotateY = yAngle;
    
    if(_RotateX > ROTATIONY)
        _RotateX = ROTATIONY;
    else if(_RotateX < -ROTATIONY)
        _RotateX = - ROTATIONY;
    
    if(_RotateY > ROTATIONX)
        _RotateY = ROTATIONX;
    else if(_RotateY < -ROTATIONX)
        _RotateY = -ROTATIONX;
    
    xAngle = _RotateX;
    yAngle = _RotateY;
}

void Renderer::setScale(float & scale)
{
    if(scale > SCALEMAX)
        scale = SCALEMAX;
    else if(scale < SCALEMIN)
        scale = SCALEMIN;
    
    if(scale != _Scale)
    {
        _Scale = scale;
        updateProjectionMatrix();
    }
}

void Renderer::setHairPositionAdjustVector(float x, float y, float z)
{
    _HairPositionAdjustX = x;
    _HairPositionAdjustY = y;
    _HairPositionAdjustZ = z;
}

void Renderer::setHairScale(float scale)
{
	_HairScale = scale;
}

void Renderer::updateMatrixes()
{
    //Transform matrix
    BoundBox * headBoundBox = _HeadModel->getBoundBox();
    TransformMatrix * transformMatrix = _HeadModel->getTransformMatrix();
    float scaleFactor = (*transformMatrix).scaleFactor;
    float xTrans = (*transformMatrix).positionTrans[0];
    float yTrans = (*transformMatrix).positionTrans[1];
    float zTrans = (*transformMatrix).positionTrans[2];
    float longMoveX = (*headBoundBox).boundCenter[0] * scaleFactor + xTrans;
    float longMoveY = (*headBoundBox).boundCenter[1] * scaleFactor + yTrans;
    float longMoveZ = (*headBoundBox).boundCenter[2] * scaleFactor + zTrans + ( ( (*headBoundBox).boundMax[2] - (*headBoundBox).boundMin[2] ) / 2.0f * 0.4);
    
    glm::mat4x4 baseModelViewMatrix = glm::translate(glm::mat4x4(1.0f), glm::vec3(longMoveX, longMoveY, longMoveZ));
    //baseModelViewMatrix = glm::scale(baseModelViewMatrix, glm::vec3(_Scale));
    baseModelViewMatrix = glm::rotate(baseModelViewMatrix, _RotateX, glm::vec3(0.0f, 1.0f, 0.0f));
    baseModelViewMatrix = glm::rotate(baseModelViewMatrix, _RotateY, glm::vec3(1.0f, 0.0f, 0.0f));
    baseModelViewMatrix = glm::translate(baseModelViewMatrix, glm::vec3(-longMoveX, -longMoveY, -longMoveZ));
    
    _Head_ModelViewMatrix4 = glm::translate(baseModelViewMatrix, glm::vec3(xTrans, yTrans, zTrans));
    _Head_ModelViewMatrix4 = glm::scale(_Head_ModelViewMatrix4, glm::vec3(scaleFactor));
    _Head_ModelViewProjectionMatrix4 = _ProjectionMatrix * _Head_ModelViewMatrix4;
    
    //LOGI("update matrix %f, %f, %f\n", _HairScale, _HairScaleX, _HairScale + _HairScaleX);

    glm::mat4x4 hairAdjustMatrix = glm::translate(glm::mat4x4(1.0f), glm::vec3(longMoveX, longMoveY, longMoveZ));
    hairAdjustMatrix = glm::translate(hairAdjustMatrix, glm::vec3(_HairPositionAdjustX, _HairPositionAdjustY, _HairPositionAdjustZ));

    //TODO change hair scale
    hairAdjustMatrix = glm::scale(hairAdjustMatrix, glm::vec3(_HairScale+_HairScaleX, _HairScale, _HairScale));
    hairAdjustMatrix = glm::translate(hairAdjustMatrix, glm::vec3(-longMoveX, -longMoveY, -longMoveZ));
    _Hair_ModelViewProjectionMatrix4 = _ProjectionMatrix * baseModelViewMatrix * hairAdjustMatrix * _HairTransferMatrix;
    
    _HairAdjustMatrix = hairAdjustMatrix;
    
    _Back_ModelViewProjectionMatrix4 = _ProjectionMatrix;
    
    if(_LastScale != _Scale || _LastRotateX != _RotateX || _LastRotateY != _RotateY)
    {
        _LastRotateX = _RotateX;
        _LastRotateY = _RotateY;
        _LastScale = _Scale;
        
        updateBackModel();
    }
    
    //TODO select expression
    if(_EnableExpression)
        updateHeadModel();
}

void Renderer::render()
{
    glClearColor(111.0f / 255.0f, 113.0f / 255.0f, 121.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawHairless(_Back_ModelViewProjectionMatrix4, _Head_ModelViewProjectionMatrix4);

	if (strlen(_CurrentHairStyleDir) > 0) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		drawSurfeceWithTriangleStrip(_Hair_ModelViewProjectionMatrix4);
		glDisable(GL_BLEND);
	}
}

void Renderer::releaseMemory()
{
    tearDownGL();
}

void Renderer::changeHairStyle(const char * hairStyleDir)
{
    if(strlen(hairStyleDir) > 0 && strcmp(hairStyleDir, _CurrentHairStyleDir) != 0)
    {
        strcpy(_CurrentHairStyleDir, hairStyleDir);
        
        loadHairModel();
        //setHeadAOData();
        evaluateHairTransferMatrix();
        updateMatrixes();
    }

}

void Renderer::enableExpression(bool isEnable)
{
    _EnableExpression = isEnable;

    if(isEnable == false)
    {
    	_HeadDeformer->resetDeformer();
    	updateHeadModel();
    }
}

void Renderer::changeExpression(int expressionID)
{
	_HeadDeformer->setExpressionID(expressionID);
}

void Renderer::changeModifier(int modifierID)
{	_HeadDeformer->setModifierID(modifierID);

}

void Renderer::resetExpression()
{
	_HeadDeformer->resetDeformer();
}

void Renderer::resizeViewport(int windowWidth, int windowHeight)
{
    _WindowWidth = windowWidth;
    _WindowHeight = windowHeight;
    
    updateProjectionMatrix();
    updateMatrixes();
}

unsigned char * Renderer::takeScreenshot(int &width, int &height)
{
    if(_ScreenShotData == NULL)
        _ScreenShotData = new unsigned char[_BGTextureWidth * _BGTextureHeight * 4];
    
    width = _BGTextureWidth;
    height= _BGTextureHeight;
    
    //evaluate MVP for background head and hair
    glm::mat4x4 head_ModelViewProjectionMatrix4, back_ModelViewProjectionMatrix4, hair_ModelViewProjectionMatrix4;
    
    glm::mat4x4 projectionMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height, -600.0f, 1000.0f);
    
    BoundBox * headBoundBox = _HeadModel->getBoundBox();
    TransformMatrix * transformMatrix = _HeadModel->getTransformMatrix();
    float scaleFactor = (*transformMatrix).scaleFactor;
    float xTrans = (*transformMatrix).positionTrans[0];
    float yTrans = (*transformMatrix).positionTrans[1];
    float zTrans = (*transformMatrix).positionTrans[2];
    float longMoveX = (*headBoundBox).boundCenter[0] * scaleFactor + xTrans;
    float longMoveY = (*headBoundBox).boundCenter[1] * scaleFactor + yTrans;
    float longMoveZ = (*headBoundBox).boundCenter[2] * scaleFactor + zTrans + ( ( (*headBoundBox).boundMax[2] - (*headBoundBox).boundMin[2] ) / 2.0f * 0.4);
    
    glm::mat4x4 baseModelViewMatrix = glm::translate(glm::mat4x4(1.0f), glm::vec3(longMoveX, longMoveY, longMoveZ));
    baseModelViewMatrix = glm::rotate(baseModelViewMatrix, _RotateX, glm::vec3(0.0f, 1.0f, 0.0f));
    baseModelViewMatrix = glm::rotate(baseModelViewMatrix, _RotateY, glm::vec3(1.0f, 0.0f, 0.0f));
    baseModelViewMatrix = glm::translate(baseModelViewMatrix, glm::vec3(-longMoveX, -longMoveY, -longMoveZ));
    
    glm::mat4x4 head_ModelViewMatrix4 = glm::translate(baseModelViewMatrix, glm::vec3(xTrans, yTrans, zTrans));
    head_ModelViewMatrix4 = glm::scale(head_ModelViewMatrix4, glm::vec3(scaleFactor));
    head_ModelViewProjectionMatrix4 = projectionMatrix * head_ModelViewMatrix4;
    
    glm::mat4x4 hairAdjustMatrix = glm::translate(glm::mat4x4(1.0f), glm::vec3(longMoveX, longMoveY, longMoveZ));
    hairAdjustMatrix = glm::translate(hairAdjustMatrix, glm::vec3(_HairPositionAdjustX, _HairPositionAdjustY, _HairPositionAdjustZ));

    //TODO change haiscaleX
    hairAdjustMatrix = glm::scale(hairAdjustMatrix, glm::vec3(_HairScale+_HairScaleX, _HairScale, _HairScale));
    hairAdjustMatrix = glm::translate(hairAdjustMatrix, glm::vec3(-longMoveX, -longMoveY, -longMoveZ));
    hair_ModelViewProjectionMatrix4 = projectionMatrix * baseModelViewMatrix * hairAdjustMatrix * _HairTransferMatrix;
    
    back_ModelViewProjectionMatrix4 = projectionMatrix;
    
    //set up framebuffer
    GLuint frameBuffer;
    GLuint targetTexture;
    GLuint depthBuffer;
    
    glGenTextures(1, &targetTexture);
    glBindTexture(GL_TEXTURE_2D , targetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    //render to texture
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);
    if(!checkCurrenFrameBufferStatus())
    {
        width = height = 0;
        return NULL;
    }
    
    //draw picture on framebuffer
    glViewport(0, 0, width, height);
    glClearColor(111.0f / 255.0f, 113.0f / 255.0f, 121.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawHairless(back_ModelViewProjectionMatrix4, head_ModelViewProjectionMatrix4);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawSurfeceWithTriangleStrip(hair_ModelViewProjectionMatrix4);
    glDisable(GL_BLEND);
    
    //read out pixels
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, _ScreenShotData);
    flipImageData(_ScreenShotData, width, height);
    
    //set alpha channel to 0xff
    int totalBytes = width * height * 4;
    for(int i=3; i<totalBytes; i+=4)
        _ScreenShotData[i] = 0xff;
    
    //clear
    resetAllBinds();
    glDeleteTextures(1, &targetTexture);
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteFramebuffers(1, &frameBuffer);
    
    return _ScreenShotData;
}

void printMatrix(const glm::mat4x4 & mat)
{
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
            LOGI("%f ", mat[j][i]);
        LOGI("\n");
    }
    LOGI("**************************\n");
}

/*
void Renderer::evaluateNewSurfaceTransData()
{
    glm::mat4x4 rstMat;
    
    rstMat = glm::inverse(_HairForwardMatrix) * _HairAdjustMatrix * _HairTransferMatrix;
    rstMat = glm::inverse(rstMat); // rstMat = R * S * T
    
    TransformMatrix * matrix = _SurfaceModel->getTransformMatrix();
    float * rotation = (*matrix).rotation;
//    float * position = (*matrix).positionTrans;
//    float scale = (*matrix).scaleFactor;
    
    float tmpMat[16];
    tmpMat[0] = rotation[0];    tmpMat[1] = rotation[1];    tmpMat[2] = rotation[2];    tmpMat[3] = 0.0f;
    tmpMat[4] = rotation[3];    tmpMat[5] = rotation[4];    tmpMat[6] = rotation[5];    tmpMat[7] = 0.0f;
    tmpMat[8] = rotation[6];    tmpMat[9] = rotation[7];    tmpMat[10] = rotation[8];   tmpMat[11] = 0.0f;
    tmpMat[12] = 0.0f;          tmpMat[13] = 0.0f;          tmpMat[14] = 0.0f;          tmpMat[15] = 1.0f;
    
    glm::mat4x4 rotateMatrixInv = glm::make_mat4x4(tmpMat);
    
//    glm::mat4x4 backMatrix = glm::translate(glm::mat4x4(1.0f), glm::vec3(position[0], position[1], position[2]));
//    backMatrix = glm::scale(backMatrix, glm::vec3(scale));
//    backMatrix = backMatrix * glm::inverse(glm::make_mat4x4(tmpMat));
//    
//    LOGE("rst matrix:\n");
//    printMatrix(rstMat);
//    LOGE("back matrix:\n");
//    printMatrix(backMatrix);
    
    glm::mat4x4 STMat = rstMat * rotateMatrixInv;
    LOGI("ST matrix:\n");
    printMatrix(STMat);
    
    float newScale = (STMat[0][0] + STMat[1][1] + STMat[2][2]) / 3.0f;
    float newX = STMat[3][0];
    float newY = STMat[3][1];
    float newZ = STMat[3][2];
    LOGI("\n(s, x, y ,z) = (%f %f %f %f)\n\n", newScale, newX, newY, newZ);
    checkSurfaceTransEvaluation(newScale, newX, newY, newZ);
    
    char transFilePath[500];
    sprintf(transFilePath, "%s/trans.txt", _CurrentHairStyleDir);
    FILE * transFile = fopen(transFilePath, "w");
    int arrayIndex = 0;
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
            fprintf(transFile, "%f ", rotation[arrayIndex++]);
        fprintf(transFile, "\n");
    }
    fprintf(transFile, "%f %f %f\n", newX, newY, newZ);
    fprintf(transFile, "%f\n", newScale);
    fclose(transFile);
}
*/

void Renderer::checkSurfaceTransEvaluation(float newS ,float newX, float newY, float newZ)
{
    TransformMatrix * matrix = _SurfaceModel->getTransformMatrix();
    float * rotation = (*matrix).rotation;
    
    float tmpMat[16];
    tmpMat[0] = rotation[0];    tmpMat[1] = rotation[1];    tmpMat[2] = rotation[2];    tmpMat[3] = 0.0f;
    tmpMat[4] = rotation[3];    tmpMat[5] = rotation[4];    tmpMat[6] = rotation[5];    tmpMat[7] = 0.0f;
    tmpMat[8] = rotation[6];    tmpMat[9] = rotation[7];    tmpMat[10] = rotation[8];   tmpMat[11] = 0.0f;
    tmpMat[12] = 0.0f;          tmpMat[13] = 0.0f;          tmpMat[14] = 0.0f;          tmpMat[15] = 1.0f;
    glm::mat4x4 backMatrix = glm::make_mat4x4(tmpMat);
    
    newS = 1.0 / newS;
    backMatrix = glm::scale(backMatrix, glm::vec3(newS));
    backMatrix = glm::translate(backMatrix, glm::vec3(-newX, -newY, -newZ));
    
    matrix = _HeadModel->getTransformMatrix();
    rotation = (*matrix).rotation;
    float * position = (*matrix).positionTrans;
    float scale = (*matrix).scaleFactor;
    
    glm::mat4x4 forwardMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position[0], position[1], position[2]));
    forwardMatrix = glm::scale(forwardMatrix, glm::vec3(scale));
    
    tmpMat[0] = rotation[0];    tmpMat[1] = rotation[1];    tmpMat[2] = rotation[2];    tmpMat[3] = 0.0f;
    tmpMat[4] = rotation[3];    tmpMat[5] = rotation[4];    tmpMat[6] = rotation[5];    tmpMat[7] = 0.0f;
    tmpMat[8] = rotation[6];    tmpMat[9] = rotation[7];    tmpMat[10] = rotation[8];   tmpMat[11] = 0.0f;
    tmpMat[12] = 0.0f;          tmpMat[13] = 0.0f;          tmpMat[14] = 0.0f;          tmpMat[15] = 1.0f;
    glm::mat4x4 rotationMatrix = glm::make_mat4x4(tmpMat);
    
    rotationMatrix = glm::inverse(rotationMatrix);
    
    forwardMatrix = forwardMatrix * rotationMatrix;
    glm::mat4x4 newHairTransferMatrix = forwardMatrix * backMatrix;
    
    glm::mat4x4 cmpMat = _HairAdjustMatrix * _HairTransferMatrix;

	LOGI("\n~~~~~~~~~~~~~~~~~~~\n");
	LOGI("new Trans Mat:\n");
    printMatrix(newHairTransferMatrix);
	LOGI("cmp mat:\n");
    printMatrix(cmpMat);
    
    glm::mat4x4 checkMat = newHairTransferMatrix - cmpMat;
	LOGI("check:\n");
    printMatrix(checkMat);
	LOGI("~~~~~~~~~~~~~~~~~~~\n\n");
}

// Data maintenance
void Renderer::initData()
{
    _LastRotateX = _RotateX = 0.0f;
    _LastRotateY = _RotateY = 0.0f;
    _LastScale = _Scale = 1.0f;
    
    _HairPositionAdjustX = 0.0f;
    _HairPositionAdjustY = 0.0f;
    _HairPositionAdjustZ = 0.0f;
    _HairScale = 1.0f;
    _HairScaleX = 0.0f;

    _HeadVertexArray = 0;
    _HeadPositionBuffer = 0;
    _HeadUVBuffer = 0;
    _HeadNormalBuffer = 0;
    _HeadTexture = 0;
    
    _HairGTTexture = 0;
    _HairGTVertexArray = 0;
    _HairGTVertexBuffer = 0;
    
    _BackQuadVertexArray = 0;
    _BackQuadPositionBuffer = 0;
    _BackQuadUVBuffer = 0;
    _BackQuadIndexBuffer = 0;
    _BackTexture = 0;
    
    _HeadModel = NULL;
    _BackModel = NULL;
    _SurfaceModel = NULL;
    _HeadDeformer = NULL;
    _ImageBalancer = NULL;
    
    _QuadVeretexArray = 0;
    _QuadVertexBuffer = 0;
    _QuadTexture = 0;
    
    _HairImageHeight = 1;
    _HairImageWidth = 1;
    
    _EnableExpression = false;
    _AlreadlySelectHairStyle = false;

    _CurrentHairStyleDir[0] = '\0'; //init _CurrentHairStyleDir lenght to 0

    _BlendColor = glm::vec4(0.0, 0.0, 0.0, 0.0);

    updateProjectionMatrix();
}

void Renderer::setupGL()
{
	GLCheck(Renderer::setupGL);
    loadShaders();
    
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.5f);
}

void Renderer::tearDownGL()
{
	// hack by weijin
	//if (_program4BG) {
	//	glDeleteProgram(_program4BG);
	//	_program4BG = 0;
	//}
	//if (_program4Head) {
	//	glDeleteProgram(_program4Head);
	//	_program4Head = 0;
	//}
	//if (_program4Surface) {
	//	glDeleteProgram(_program4Surface);
	//	_program4Surface = 0;
	//}
    
    if(_HeadModel)
    {
        _HeadModel->resetModel();
        delete _HeadModel;
        _HeadModel = NULL;
    }
    if(_SurfaceModel)
    {
        _SurfaceModel->resetModel();
        delete _SurfaceModel;
        _SurfaceModel = NULL;
    }
    if(_BackModel)
    {
        _BackModel->resetModel();
        delete _BackModel;
        _BackModel = NULL;
    }
    if(_HeadDeformer)
    {
        _HeadDeformer->clearExpressionData();
        delete _HeadDeformer;
        _HeadDeformer = NULL;
    }
    if(_ImageBalancer)
    {
        delete _ImageBalancer;
        _ImageBalancer = NULL;
    }
    
    clearGLData4Head();
    clearGLData4Hair();
    clearGLData4Quad();
    clearGLData4BackQuad();
}

void Renderer::updateProjectionMatrix()
{
    float rstScreenWidth, rstScreenHeight;
    float widthScreen = _WindowWidth;
    float heightScreen = _WindowHeight;
    float widthPhoto = _BGTextureWidth;
    float heightPhoto = _BGTextureHeight;
    float radioPhoto = widthPhoto / heightPhoto;
    float radioScreen = widthScreen / heightScreen;
    if(radioPhoto >= radioScreen)
    {
        rstScreenWidth =  widthPhoto;
        rstScreenHeight = rstScreenWidth / radioScreen;
    }
    else
    {
        rstScreenHeight = heightPhoto;
        rstScreenWidth = rstScreenHeight * radioScreen;
    }
    
    rstScreenHeight /= _Scale;
    rstScreenWidth /= _Scale;
    
    float disW = (rstScreenWidth - widthPhoto ) / 2.0f;
    float disH = (rstScreenHeight - heightPhoto) / 2.0f;
    _ProjectionMatrix = glm::ortho(-disW, widthPhoto + disW, -disH, heightPhoto + disH, -600.0f, 1000.0f);
}

void Renderer::initImageBalancer()
{
    if(!_FaceImageData)
    {
        LOGE("invalid _FaceImageData.\n");
        return;
    }
    
    if(_ImageBalancer)
    {
        delete _ImageBalancer;
        _ImageBalancer = NULL;
    }
    
    _ImageBalancer = new ImageBalancer();
    _ImageBalancer->initBalancer(_FaceImageData, _FaceImageWidth, _FaceImageHeight);
    delete [] _FaceImageData;
    _FaceImageData = NULL;
}

void Renderer::setupModel()
{
    // the head model need to be loaded before hair model.
    loadHeadModel();

    loadQuadModel();

    updateMatrixes();
    
    loadBackQuadModel();
}

void Renderer::loadHeadModel()
{
    char headModelFile[500];
    char headTransformFile[500];
    sprintf(headModelFile, "%s/head.obj", _HeadDataDir);
    sprintf(headTransformFile, "%s/trans.txt", _HeadDataDir);
    
    if(_HeadModel)
    {
        _HeadModel->resetModel();
        delete _HeadModel;
        _HeadModel = NULL;
    }
    clearGLData4Head();
    
    _HeadModel = new HeadModel();
    bool isSuccess = _HeadModel->loadModelFromFile(headModelFile, headTransformFile);
    if(!isSuccess)
    {
        LOGE("Failed to load head model");
        return;
    }
    
    glGenBuffers(1, &_HeadPositionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _HeadPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, _HeadModel->getVertexNum() * sizeof(float) * 3, _HeadModel->getPositionArray(), GL_DYNAMIC_DRAW);
    
    glGenBuffers(1, &_HeadUVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _HeadUVBuffer);
    glBufferData(GL_ARRAY_BUFFER, _HeadModel->getVertexNum() * sizeof(float) * 2, _HeadModel->getUVDataArray(), GL_STATIC_DRAW);
    _HeadModel->clearUVDataArray();
    
    glGenVertexArraysOES(1, &_HeadVertexArray);
    glBindVertexArrayOES(_HeadVertexArray);
    
    glEnableVertexAttribArray(HVertexAttribPosition);
    glBindBuffer(GL_ARRAY_BUFFER, _HeadPositionBuffer);
    glVertexAttribPointer(HVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, BUFFER_OFFSET(0));
    
    glEnableVertexAttribArray(HVertexAttribTexCoord0);
    glBindBuffer(GL_ARRAY_BUFFER, _HeadUVBuffer);
    glVertexAttribPointer(HVertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, BUFFER_OFFSET(0));

    loadTextureFromImageData(_HeadTextureData, _HeadTextureWidth, _HeadTextureHeight, &_HeadTexture);
    delete [] _HeadTextureData;
    _HeadTextureData = NULL;
    
    //Expression
    char expressionFile[500];
    char modifierFile[500];
    sprintf(expressionFile, "%s/expressions.dat", _ExpressionFileDir);
    sprintf(modifierFile, "%s/modifiers.dat", _ExpressionFileDir);
    
#ifdef AnimateExpression
	_HeadDeformer = new AnimDeformer();
#else
    _HeadDeformer = new HeadDeformer();
#endif
	LOGE("Expression file %s", expressionFile);
	LOGE("Modifier file %s", modifierFile);
    _HeadDeformer->loadExpressionData(expressionFile, modifierFile, headTransformFile);
}

void Renderer::loadHairModel()
{
    clock_t startTime, endTime, totalStartTime, totalEndTime;
    
    totalStartTime = startTime = clock();
    
//    char surfaceModelFile[500];
    char hairTransformFile[500];
    char additionInfoFile[500];
    char hairTexRGBFile[500];
    char hairTexAlphaFile[500];
    char hairDepthFile[500];
    char hairDepthInfoFile[500];
    
//    sprintf(surfaceModelFile, "%s/surface.SUR", _CurrentHairStyleDir);
    sprintf(hairTransformFile, "%s/trans.txt", _CurrentHairStyleDir);
    sprintf(hairTexRGBFile, "%s/hairTexRGB.jpg", _CurrentHairStyleDir);
    sprintf(hairTexAlphaFile, "%s/hairTexA.jpg", _CurrentHairStyleDir);
    sprintf(additionInfoFile, "%s/addition.txt", _CurrentHairStyleDir);
    sprintf(hairDepthFile, "%s/depth.png", _CurrentHairStyleDir);
    sprintf(hairDepthInfoFile, "%s/depthInfo.txt", _CurrentHairStyleDir);
    
    if(_SurfaceModel)
    {
        _SurfaceModel->resetModel();
        delete _SurfaceModel;
        _SurfaceModel = NULL;
    }
    clearGLData4Hair();
    
    _SurfaceModel = new SurfaceModel();
    //bool isSuccess = _SurfaceModel->loadModelFromFile(surfaceModelFile, hairTransformFile,NULL);
    startTime = clock();
    bool isSuccess = _SurfaceModel->loadModelFromDepthFile(hairDepthFile, hairDepthInfoFile, hairTransformFile);
    endTime = clock();
	LOGI("load hair model: %f\n", ((float)(endTime - startTime)) / CLOCKS_PER_SEC);
    
    if(!isSuccess)
    {
        LOGE("Failed to load suraface model %s\n", _CurrentHairStyleDir);
    }
    
    _GTHairNum = _SurfaceModel->getSortedVertexNum();
    
    glGenBuffers(1, &_HairGTVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _HairGTVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, _SurfaceModel->getSortedVertexNum() * SurfaceModel::getVertexUnitSize(), _SurfaceModel->getVertexDataArray(), GL_STATIC_DRAW);
    
    _SurfaceModel->clearVertexDataArray();
    
    glGenVertexArraysOES(1, &_HairGTVertexArray);
    glBindVertexArrayOES(_HairGTVertexArray);
    glEnableVertexAttribArray(HVertexAttribPosition);
    glVertexAttribPointer(HVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, SurfaceModel::getVertexUnitSize(), BUFFER_OFFSET(0));
    
    int textureWidth, textureHeight;
    unsigned char * textureData = getImageDataAndInfoFromJpgImages(hairTexRGBFile, hairTexAlphaFile, &textureWidth, &textureHeight);

    float hairFaceLuminace;
    FILE * additionFile = fopen(additionInfoFile, "r");
    if(!additionFile)
    {
        LOGE("Failed to open Addition Info File(%s).\n", additionInfoFile);
        return;
    }
    fscanf(additionFile, "%f", &hairFaceLuminace);
    fclose(additionFile);
    
    endTime = clock();
    
    startTime = clock();
    _ImageBalancer->runBalance(hairFaceLuminace, textureData, textureWidth, textureHeight);
    endTime = clock();
    
    loadTextureFromImageData(textureData, textureWidth, textureHeight, &_HairGTTexture);
    _HairImageWidth = textureWidth;
    _HairImageHeight = textureHeight;
    delete [] textureData;
    
    totalEndTime = clock();
	LOGI("hair change time: %f\n", ((float)(totalEndTime - totalStartTime)) / CLOCKS_PER_SEC);
}


void Renderer::loadQuadModel()
{
    clearGLData4Quad();
    
    glGenBuffers(1, &_QuadVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _QuadVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 6, myQuad, GL_STATIC_DRAW);
    
    glGenVertexArraysOES(1, &_QuadVeretexArray);
    glBindVertexArrayOES(_QuadVeretexArray);
    glEnableVertexAttribArray(HVertexAttribPosition);
    glVertexAttribPointer(HVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(HVertexAttribTexCoord0);
    glVertexAttribPointer(HVertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(float)*3));
    
//    char quadTexFilePath[500];
//    sprintf(quadTexFilePath, "%s/sceneBackground.jpg", _ExpressionFileDir);
//    loadTexture(quadTexFilePath, &_QuadTexture, NULL, NULL);
}

void Renderer::loadBackQuadModel()
{
    char boundaryFile[500];
    sprintf(boundaryFile, "%s/boundary.txt", _HeadDataDir);
    
    clearGLData4BackQuad();
    
    loadTextureFromImageData(_BGTextureData, _BGTextureWidth, _BGTextureHeight, &_BackTexture);
    
    _BackImageWidth = _BGTextureWidth;
    _BackImageHeight = _BGTextureHeight;
    delete [] _BGTextureData;
    _BGTextureData = NULL;
    if(_BackModel)
    {
        _BackModel->resetModel();
        delete _BackModel;
        _BackModel = NULL;
    }
    _BackModel = new BackModel();
    
    bool isSuccess = _BackModel->loadBackModel(_BackImageWidth, _BackImageHeight, boundaryFile, _HeadModel, _Head_ModelViewMatrix4);
    if(!isSuccess)
    {
        LOGE("Failed to load back model.\n");
        return;
    }

    _BackModel->runDeform(_Head_ModelViewMatrix4);

    glGenBuffers(1, &_BackQuadPositionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _BackQuadPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * _BackModel->getVertexNum(), _BackModel->getPositionArray(), GL_DYNAMIC_DRAW);
    
    glGenBuffers(1, &_BackQuadUVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _BackQuadUVBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * _BackModel->getVertexNum(), _BackModel->getUVArray(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &_BackQuadIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _BackQuadIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * _BackModel->getIndexNum(), _BackModel->getIndexArray(), GL_STATIC_DRAW);
    
    glGenVertexArraysOES(1, &_BackQuadVertexArray);
    glBindVertexArrayOES(_BackQuadVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, _BackQuadPositionBuffer);
    glEnableVertexAttribArray(HVertexAttribPosition);
    glVertexAttribPointer(HVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, BUFFER_OFFSET(0));
    glBindBuffer(GL_ARRAY_BUFFER, _BackQuadUVBuffer);
    glEnableVertexAttribArray(HVertexAttribTexCoord0);
    glVertexAttribPointer(HVertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, BUFFER_OFFSET(0));

}

void Renderer::loadBodyQuadModel()
{
    clearGLData4BodyQuad();
    
    char bodyImageFile[500];
    sprintf(bodyImageFile, "%s/body.png", _CurrentHairStyleDir);
    
    loadTexture(bodyImageFile, &_BodyTexture, &_BodyImageWidth, &_BodyImageHeight);
    
    float xLeft = 0.0f;
    float xRight = _BodyImageWidth;
    float yBottom = 0.0f;
    float yUp = _BodyImageHeight;
    BoundBox * boundBox = _HeadModel->getBoundBox();
    TransformMatrix * transformMatrix = _HeadModel->getTransformMatrix();
    float z = (*boundBox).boundCenter[2] * ((*transformMatrix).scaleFactor) + (*transformMatrix).positionTrans[2];
    
    Vertex quad[6]= {
        {xLeft,     yBottom,    z,     0.0f, 0.0f},
        {xRight,    yBottom,    z,     1.0f, 0.0f},
        {xRight,    yUp,        z,     1.0f, 1.0f},
        
        {xRight,    yUp,        z,     1.0f, 1.0f},
        {xLeft,     yUp,        z,     0.0f, 1.0f},
        {xLeft,     yBottom,    z,     0.0f, 0.0f}
    };
    
    glGenBuffers(1, &_BodyQuadVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _BodyQuadVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 6, quad, GL_STATIC_DRAW);
    
    glGenVertexArraysOES(1, &_BodyQuadVertexArray);
    glBindVertexArrayOES(_BodyQuadVertexArray);
    glEnableVertexAttribArray(HVertexAttribPosition);
    glVertexAttribPointer(HVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(HVertexAttribTexCoord0);
    glVertexAttribPointer(HVertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(float)*3));
}

void Renderer::setHeadAOData()
{
    _HeadModel->generateNormalDataArrayFromAO(_SurfaceModel->getAODataArray());
    
//    if(_HeadNormalBuffer)
//        glDeleteBuffers(1, &_HeadNormalBuffer);
//    glGenBuffers(1, &_HeadNormalBuffer);
//    glBindBuffer(GL_ARRAY_BUFFER, _HeadNormalBuffer);
//    glBufferData(GL_ARRAY_BUFFER, _HeadModel->getVertexNum() * sizeof(float) * 3, _HeadModel->getNormalDataArray(), GL_STATIC_DRAW);
//    
//    glBindVertexArrayOES(_HeadVertexArray);
//    glEnableVertexAttribArray(HVertexAttribNormal);
//    glVertexAttribPointer(HVertexAttribNormal, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, BUFFER_OFFSET(0));
    
    _SurfaceModel->clearAODataArray();
    _HeadModel->clearNormalDataArray();
}

void Renderer::evaluateHairTransferMatrix()
{
    TransformMatrix * matrix = _SurfaceModel->getTransformMatrix();
    float * rotation = (*matrix).rotation;
    float * position = (*matrix).positionTrans;
    float scale = (*matrix).scaleFactor;

    float tmpMat[16];
    tmpMat[0] = rotation[0];    tmpMat[1] = rotation[1];    tmpMat[2] = rotation[2];    tmpMat[3] = 0.0f;
    tmpMat[4] = rotation[3];    tmpMat[5] = rotation[4];    tmpMat[6] = rotation[5];    tmpMat[7] = 0.0f;
    tmpMat[8] = rotation[6];    tmpMat[9] = rotation[7];    tmpMat[10] = rotation[8];   tmpMat[11] = 0.0f;
    tmpMat[12] = 0.0f;          tmpMat[13] = 0.0f;          tmpMat[14] = 0.0f;          tmpMat[15] = 1.0f;
    glm::mat4x4 backMatrix = glm::make_mat4x4(tmpMat);
    
    scale = 1.0 / scale;

    backMatrix = glm::scale(backMatrix, glm::vec3(scale));
    backMatrix = glm::translate(backMatrix, glm::vec3(-position[0], -position[1], -position[2]));
    
    matrix = _HeadModel->getTransformMatrix();
    rotation = (*matrix).rotation;
    position = (*matrix).positionTrans;
    scale = (*matrix).scaleFactor;

    glm::mat4x4 forwardMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position[0], position[1], position[2]));
    forwardMatrix = glm::scale(forwardMatrix, glm::vec3(scale));

    tmpMat[0] = rotation[0];    tmpMat[1] = rotation[1];    tmpMat[2] = rotation[2];    tmpMat[3] = 0.0f;
    tmpMat[4] = rotation[3];    tmpMat[5] = rotation[4];    tmpMat[6] = rotation[5];    tmpMat[7] = 0.0f;
    tmpMat[8] = rotation[6];    tmpMat[9] = rotation[7];    tmpMat[10] = rotation[8];   tmpMat[11] = 0.0f;
    tmpMat[12] = 0.0f;          tmpMat[13] = 0.0f;          tmpMat[14] = 0.0f;          tmpMat[15] = 1.0f;
    glm::mat4x4 rotationMatrix = glm::make_mat4x4(tmpMat);
    
    rotationMatrix = glm::inverse(rotationMatrix);
    forwardMatrix = forwardMatrix * rotationMatrix;

    LOGI("%f\n", _SurfaceModel->getDisplacementX());

    if(_SurfaceModel->getDisplacementX() != 0)
    	_HairScaleX = 1.0f - _SurfaceModel->getDisplacementX()/_HeadModel->getDisplacementX();
    else
    	_HairScaleX = 0;

    glm::mat4x4 displacementMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, _HeadModel->getDisplacementY(), _HeadModel->getDisplacementZ()));

    _HairTransferMatrix = forwardMatrix * displacementMatrix * backMatrix;
    //_HairTransferMatrix = forwardMatrix * backMatrix;

    _HairForwardMatrix = forwardMatrix;
}

void Renderer::updateBackModel()
{
    if(_BackModel && _BackQuadPositionBuffer)
    {
        _BackModel->runDeform(_Head_ModelViewMatrix4);
        
        glBindBuffer(GL_ARRAY_BUFFER, _BackQuadPositionBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * _BackModel->getVertexNum(), _BackModel->getPositionArray());
    }
}

void Renderer::updateHeadModel()
{
    vector<OBJVertex> originVec = _HeadModel->getOBJVertexVector();
    vector<OBJVertex> outVec = _HeadModel->getExpressionOBJVertexVector();
    _HeadDeformer->runDeform(originVec, outVec);
    _HeadModel->updataHeadPositionArrayWithOBJVertexVector(outVec);
    
    glBindBuffer(GL_ARRAY_BUFFER, _HeadPositionBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * _HeadModel->getVertexNum(), _HeadModel->getPositionArray());
}

void Renderer::clearGLData4Head()
{
    if(_HeadTexture)
        glDeleteTextures(1, &_HeadTexture);
    if(_HeadVertexArray)
        glDeleteVertexArraysOES(1, &_HeadVertexArray);
    if(_HeadPositionBuffer)
        glDeleteBuffers(1, &_HeadPositionBuffer);
    if(_HeadUVBuffer)
        glDeleteBuffers(1, &_HeadUVBuffer);
    if(_HeadNormalBuffer)
        glDeleteBuffers(1, &_HeadNormalBuffer);
}

void Renderer::clearGLData4Hair()
{
    if(_HairGTTexture)
        glDeleteTextures(1, &_HairGTTexture);
    if(_HairGTVertexArray)
        glDeleteVertexArraysOES(1, &_HairGTVertexArray);
    if(_HairGTVertexBuffer)
        glDeleteBuffers(1, &_HairGTVertexBuffer);
}

void Renderer::clearGLData4Quad()
{
    if(_QuadVeretexArray)
        glDeleteVertexArraysOES(1, &_QuadVeretexArray);
    if(_QuadVertexBuffer)
        glDeleteBuffers(1, &_QuadVertexBuffer);
    if(_QuadTexture)
        glDeleteTextures(1, &_QuadTexture);
}

void Renderer::clearGLData4BackQuad()
{
    if(_BackTexture)
        glDeleteTextures(1, &_BackTexture);
    if(_BackQuadVertexArray)
        glDeleteVertexArraysOES(1, &_BackQuadVertexArray);
    if(_BackQuadPositionBuffer)
        glDeleteBuffers(1, &_BackQuadPositionBuffer);
    if(_BackQuadUVBuffer)
        glDeleteBuffers(1, &_BackQuadUVBuffer);
    if(_BackQuadIndexBuffer)
        glDeleteBuffers(1, &_BackQuadIndexBuffer);
}

void Renderer::clearGLData4BodyQuad()
{
    if(_BodyTexture)
        glDeleteTextures(1, &_BodyTexture);
    if(_BodyQuadVertexArray)
        glDeleteVertexArraysOES(1, &_BodyQuadVertexArray);
    if(_BodyQuadVertexBuffer)
        glDeleteBuffers(1, &_BodyQuadVertexBuffer);
}

// render scence
void Renderer::drawHead(glm::mat4x4 & head_ModelViewProjectionMatrix4)
{
    glBindVertexArrayOES(_HeadVertexArray);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _HeadTexture);
    glUseProgram(_program4Head);
    
    glUniformMatrix4fv(uniforms4Head[HEAD_UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, glm::value_ptr(head_ModelViewProjectionMatrix4));
    glUniform1i(uniforms4Head[HEAD_UNIFORM_SAMPLER], 1);
    
    glDrawArrays(GL_TRIANGLES, 0, _HeadModel->getVertexNum());
}
void Renderer::drawHairless(glm::mat4x4 & back_ModelViewProjectionMatrix4, glm::mat4x4 & head_ModelViewProjectionMatrix4)
{    
    glUseProgram(_program4BG);
    glUniformMatrix4fv(uniforms4BG[BG_UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, glm::value_ptr(back_ModelViewProjectionMatrix4));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _BackTexture);
    glUniform1i(uniforms4BG[HEAD_UNIFORM_SAMPLER], 0);
    
    glBindVertexArrayOES(_BackQuadVertexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _BackQuadIndexBuffer);
    glDrawElements(GL_TRIANGLES, _BackModel->getIndexNum(), GL_UNSIGNED_INT, (void*)0);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(_program4Head);
    glUniformMatrix4fv(uniforms4Head[HEAD_UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, glm::value_ptr(head_ModelViewProjectionMatrix4));
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(uniforms4Head[HEAD_UNIFORM_SAMPLER], 2);
    glBindTexture(GL_TEXTURE_2D, _HeadTexture);

    glBindVertexArrayOES(_HeadVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, _HeadModel->getVertexNum());
}

void Renderer::changeHairColor(float r, float g, float b, float a)
{
	_BlendColor = glm::vec4(r,g,b,a);
}

void Renderer::drawSurfeceWithTriangleStrip(glm::mat4x4 & hair_ModelViewProjectionMatrix4)
{
    glBindVertexArrayOES(_HairGTVertexArray);
    glUseProgram(_program4Surface);
    
    glUniformMatrix4fv(uniforms4Surface[SURFACE_UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, glm::value_ptr(hair_ModelViewProjectionMatrix4));
    glUniform1f(uniforms4Surface[SURFACE_UNIFORM_IMAGEWIDTH], (float)_HairImageWidth);
    glUniform1f(uniforms4Surface[SURFACE_UNIFORM_IMAGEHEIGHT], (float)_HairImageHeight);
    glUniform4fv(uniforms4Surface[SUFRACE_UNIFORM_BLEND_COLOR], 1, glm::value_ptr(_BlendColor));
    glCheck("Renderer", "surface_set_uniform4fv");

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _HairGTTexture);
    glUniform1i(uniforms4Surface[SURFACE_UNIFORM_SAMPLER], 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, _GTHairNum);
}

// OpenGL ES 2 shader compilation
bool Renderer::loadShaders()
{
	GLCheck(Renderer::loadShaders);
    char vShaderPath[500];
    char fShaderPath[500];
    
    sprintf(vShaderPath, "%s/Shader4BG.vsh", _ShaderFileDir);
    sprintf(fShaderPath, "%s/Shader4BG.fsh", _ShaderFileDir);
    if(!loadShader4BG(vShaderPath, fShaderPath))
    {
        LOGE("Failed to load shader for background.\n");
        return false;
    }
    
    sprintf(vShaderPath, "%s/Shader4Head.vsh", _ShaderFileDir);
    sprintf(fShaderPath, "%s/Shader4Head.fsh", _ShaderFileDir);
    if(!loadShader4Head(vShaderPath, fShaderPath))
    {
        LOGE("Failed to load shader for head.\n");
        return false;
    }
    
    sprintf(vShaderPath, "%s/Shader4Surface.vsh", _ShaderFileDir);
    sprintf(fShaderPath, "%s/Shader4Surface.fsh", _ShaderFileDir);
    if(!loadShader4Surface(vShaderPath, fShaderPath))
    {
        LOGE("Failed to load shader for hair.\n");
        return false;
    }
    
    return true;
}

bool Renderer::loadShader4BG(const char * vertexShaderFilePath, const char * fragmentShaderFilePath)
{
	GLCheck(Renderer::loadShader4BG);
    GLuint vertShader, fragShader;
    // Create shader program.
    GLuint _program = glCreateProgram();
    
    // Create and compile vertex shader.
    if(!compileShader(&vertShader, GL_VERTEX_SHADER, vertexShaderFilePath, ShaderSourceFileSize))
    {
        LOGE("Failed to compile vertex shader\n");
        return false;
    }
    
    // Create and compile fragment shader.
    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fragmentShaderFilePath, ShaderSourceFileSize)) {
        LOGE("Failed to compile fragment shader\n");
        return false;
    }
    
    // Attach vertex shader to program.
    glAttachShader(_program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(_program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(_program, HVertexAttribPosition, "position");
    glBindAttribLocation(_program, HVertexAttribTexCoord0, "texCoordIn");
    
    // Link program.
    if (!linkProgram(_program)) {
        LOGE("Failed to link program: %d", _program);
        
        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (_program) {
            glDeleteProgram(_program);
            _program = 0;
        }
        
        return false;
    }
    
    // Get uniform locations.
    uniforms4BG[BG_UNIFORM_MODELVIEWPROJECTION_MATRIX] = glGetUniformLocation(_program, "modelViewProjectionMatrix");
    uniforms4BG[BG_UNIFORM_SAMPLER] = glGetUniformLocation(_program, "sampler");
    
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    _program4BG = _program;
    
    return true;
}

bool Renderer::loadShader4Quad(const char * vertexShaderFilePath, const char * fragmentShaderFilePath)
{
	GLCheck(Renderer::loadShader4Quad);
    GLuint vertShader, fragShader;
    
    // Create shader program.
    GLuint _program = glCreateProgram();
    
    // Create and compile vertex shader.
    if(!compileShader(&vertShader, GL_VERTEX_SHADER, vertexShaderFilePath, ShaderSourceFileSize))
    {
        LOGE("Failed to compile vertex shader\n");
        return false;
    }
    
    // Create and compile fragment shader.
    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fragmentShaderFilePath, ShaderSourceFileSize)) {
        LOGE("Failed to compile fragment shader\n");
        return false;
    }
    
    // Attach vertex shader to program.
    glAttachShader(_program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(_program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(_program, HVertexAttribPosition, "position");
    glBindAttribLocation(_program, HVertexAttribTexCoord0, "texCoordIn");
    
    // Link program.
    if (!linkProgram(_program)) {
        LOGE("Failed to link program: %d", _program);
        
        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (_program) {
            glDeleteProgram(_program);
            _program = 0;
        }
        
        return false;
    }
    
    // Get uniform locations.
    uniforms4Quad[QUAD_UNIFORM_MODELVIEWPROJECTION_MATRIX] = glGetUniformLocation(_program, "modelViewProjectionMatrix");
    uniforms4Quad[QUAD_UNIFORM_SAMPLER] = glGetUniformLocation(_program, "sampler");
    
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    return true;
}

bool Renderer::loadShader4Head(const char * vertexShaderFilePath, const char * fragmentShaderFilePath)
{
	GLCheck(Renderer::loadShader4Head);
    GLuint vertShader, fragShader;
    
    // Create shader program.
    GLuint _program = glCreateProgram();
    
    // Create and compile vertex shader.
    if(!compileShader(&vertShader, GL_VERTEX_SHADER, vertexShaderFilePath, ShaderSourceFileSize))
    {
        LOGE("Failed to compile vertex shader\n");
        return false;
    }
    
    // Create and compile fragment shader.
    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fragmentShaderFilePath, ShaderSourceFileSize)) {
        LOGE("Failed to compile fragment shader\n");
        return false;
    }
    
    // Attach vertex shader to program.
    glAttachShader(_program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(_program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(_program, HVertexAttribPosition, "position");
    glBindAttribLocation(_program, HVertexAttribTexCoord0, "texCoordIn");
    //glBindAttribLocation(_program, HVertexAttribNormal, "normalIn");
    
    // Link program.
    if (!linkProgram(_program)) {
        LOGE("Failed to link program: %d", _program);
        
        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (_program) {
            glDeleteProgram(_program);
            _program = 0;
        }
        
        return false;
    }
    
    // Get uniform locations.
    uniforms4Head[HEAD_UNIFORM_MODELVIEWPROJECTION_MATRIX] = glGetUniformLocation(_program, "modelViewProjectionMatrix");
    uniforms4Head[HEAD_UNIFORM_SAMPLER] = glGetUniformLocation(_program, "sampler");
    
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    _program4Head = _program;
    
    return true;
}

bool Renderer::loadShader4Surface(const char * vertexShaderFilePath, const char * fragmentShaderFilePath)
{
	GLCheck(Renderer::loadShader4Surface);
    GLuint vertShader, fragShader;
    
    // Create shader program.
    GLuint _program = glCreateProgram();
    
    // Create and compile vertex shader.
    if(!compileShader(&vertShader, GL_VERTEX_SHADER, vertexShaderFilePath, ShaderSourceFileSize))
    {
        LOGE("Failed to compile vertex shader\n");
        return false;
    }
    
    // Create and compile fragment shader.
    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fragmentShaderFilePath, ShaderSourceFileSize)) {
        LOGE("Failed to compile fragment shader\n");
        return false;
    }
    
    // Attach vertex shader to program.
    glAttachShader(_program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(_program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(_program, HVertexAttribPosition, "position");
    //glBindAttribLocation(_program, HVertexAttribColor, "color");
    
    // Link program.
    if (!linkProgram(_program)) {
        LOGE("Failed to link program: %d", _program);
        
        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (_program) {
            glDeleteProgram(_program);
            _program = 0;
        }
        
        return false;
    }
    
    // Get uniform locations.
    uniforms4Surface[SURFACE_UNIFORM_MODELVIEWPROJECTION_MATRIX] = glGetUniformLocation(_program, "modelViewProjectionMatrix");
    uniforms4Surface[SURFACE_UNIFORM_IMAGEWIDTH] = glGetUniformLocation(_program, "imageWidth");
    uniforms4Surface[SURFACE_UNIFORM_IMAGEHEIGHT] = glGetUniformLocation(_program, "imageHeight");
    uniforms4Surface[SURFACE_UNIFORM_SAMPLER] = glGetUniformLocation(_program, "sampler");
    uniforms4Surface[SUFRACE_UNIFORM_BLEND_COLOR] = glGetUniformLocation(_program, "blendColor");
    glCheck("Renderer", "surface_get_uniform4fv");

    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    _program4Surface = _program;
    
    return true;
}

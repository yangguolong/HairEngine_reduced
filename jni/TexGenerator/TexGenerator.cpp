//
//  TexGenerator.cpp
//  Demo
//
//  Created by yangguolong on 13-7-9.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "TexGenerator.h"
#include "ImageUtility.h"
#include "ShaderUtility.h"
#include "ModelCommonData.h"
#include "umfpack.h"
#include "matrix_transform.hpp"
#include "type_ptr.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <memory.h>
#include "opencv2/opencv.hpp"

#include <android/log.h>
#define LOG_TAG "TexGenerator"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define TextureWidth 1024
#define TextureHeight 1024
#define PossionTexWidth 256
#define PossionTexHeight 256
#define PixelChannels 4
#define AllowedImagePixelNum 65536 // 60000
#define ShaderSourceFileSize 1024*50  // 50k

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Min(a,b) ((a)<(b)?(a):(b))

enum
{
    FACETEX_UNIFORM_MODELVIEWPROJECTION_MATRIX,
    FACETEX_UNIFORM_SAMPLERFACE,
    FACETEX_NUM_UNIFORMS
};
GLuint uniforms4FaceTex[FACETEX_NUM_UNIFORMS];

enum
{
    FACEIMAGE_UNIFORM_MODELVIEWPROJECTION_MATRIX,
    FACEIMAGE_UNIFORM_SAMPLERFACE,
    FACEIMAGE_NUM_UNIFORMS
};
GLuint uniforms4FaceImage[FACEIMAGE_NUM_UNIFORMS];

enum
{
    FACEGRADIENT_UNIFORM_MODELVIEWPROJECTION_MATRIX,
    FACEGRADIENT_NUM_UNIFORMS
};
GLuint uniforms4FaceGradient[FACEGRADIENT_NUM_UNIFORMS];

// lifeCycle
TexGenerator::TexGenerator(const char * texInitFileDir)
{
    int size = strlen(texInitFileDir);
    _ShaderFileDir = new char[size + 3];
    strcpy(_ShaderFileDir, texInitFileDir);
    
    _Laplacian = NULL;
    _LaplacianImageData = NULL;
    _MaskImageData = NULL;
    
    _program4FaceTex = 0;
    _program4FaceImage = 0;
    _program4FaceGradient = 0;
    
    initLaplacianData();
    setupGL();
    
    _ImageData = NULL;
    _MaskData = NULL;
    
    _HeadTextureData = NULL;
    _BGTextureData = NULL;
    _HeadTextureWidth = 0;
    _HeadTextureHeight = 0;
    _BGTextureWidth = 0;
    _BGTextureHeight = 0;
    
    glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
    glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC) eglGetProcAddress("glBindVertexArrayOES");
    glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC) eglGetProcAddress("glDeleteVertexArraysOES");
}

TexGenerator::~TexGenerator()
{
    if(_Laplacian)
    {
        delete _Laplacian;
        _Laplacian = NULL;
    }
    if(_ShaderFileDir)
    {
        delete [] _ShaderFileDir;
        _ShaderFileDir = NULL;
    }
    if(_ImageData)
    {
        delete [] _ImageData;
        _ImageData = NULL;
    };
    if(_MaskData)
    {
        delete [] _MaskData;
        _MaskData = NULL;
    }
    if(_LaplacianImageData)
    {
        delete [] _LaplacianImageData;
        _LaplacianImageData = NULL;
    }
    if(_MaskImageData)
    {
        delete [] _MaskImageData;
        _MaskImageData = NULL;
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
//  releaseGL();	hack by weijin
}

void TexGenerator::initLaplacianData()
{
    char filePath[500];
    sprintf(filePath, "%s/Laplacian_%d.DFI", _ShaderFileDir, PossionTexWidth);
    _Laplacian = new Laplacian();
    _Laplacian->loadLaplacianFromFile(filePath);
    
//    sprintf(filePath, "%s/Laplacian_%d.png", _ShaderFileDir, PossionTexWidth);
//    _LaplacianImageData = getImageDataAndInfoForImagePath(filePath, NULL, NULL);
    
    sprintf(filePath, "%s/mask_%d.png", _ShaderFileDir, PossionTexWidth);
    _MaskImageData = getImageDataAndInfoForImagePath(filePath, NULL, NULL);
    flipImageData(_MaskImageData, PossionTexWidth, PossionTexHeight);
}

void TexGenerator::setupGL()
{
    glEnable(GL_DEPTH_TEST);

    loadShaders();
}

void TexGenerator::releaseGL()
{
    if (_program4FaceTex) {
		GLCheck(_program4FaceTex);
        glDeleteProgram(_program4FaceTex);
        _program4FaceTex = 0;
    }
    
    if (_program4FaceImage) {
		GLCheck(_program4FaceImage);
        glDeleteProgram(_program4FaceImage);
        _program4FaceImage = 0;
    }
    
    if(_program4FaceGradient) {
		GLCheck(_program4FaceGradient);
        glDeleteProgram(_program4FaceGradient);
        _program4FaceGradient = 0;
    }
}

bool TexGenerator::loadShaders()
{
	GLCheck(TexGenerator::loadShaders);
    char vShaderPath[500];
    char fShaderPath[500];
    
    sprintf(vShaderPath, "%s/Shader4FaceTexture.vsh", _ShaderFileDir);
    sprintf(fShaderPath, "%s/Shader4FaceTexture.fsh", _ShaderFileDir);
    if(!loadShader4FaceTex(vShaderPath, fShaderPath))
    {
        LOGI("Failed to load shader for FaceTex.\n");
        return false;
    }
    
    sprintf(vShaderPath, "%s/Shader4FaceImage.vsh", _ShaderFileDir);
    sprintf(fShaderPath, "%s/Shader4FaceImage.fsh", _ShaderFileDir);
    if(!loadShader4FaceImage(vShaderPath, fShaderPath))
    {
        LOGI("Failed to load shader for FaceImage.\n");
        return false;
    }
    
    sprintf(vShaderPath, "%s/Shader4FaceGradient.vsh", _ShaderFileDir);
    sprintf(fShaderPath, "%s/Shader4FaceGradient.fsh", _ShaderFileDir);
    if(!loadShader4FaceGradient(vShaderPath, fShaderPath))
    {
        LOGI("Failed to load shader for FaceGradient.\n");
        return false;
    }

    return true;
}

bool TexGenerator::loadShader4FaceTex(const char * vertexShaderFilePath, const char * fragmentShaderFilePath)
{
	GLCheck(TexGenerator::loadShader4FaceTex);
    GLuint vertShader, fragShader;
    
    // Create shader program.
    GLuint _program = glCreateProgram();
    
    // Create and compile vertex shader.
    if(!compileShader(&vertShader, GL_VERTEX_SHADER, vertexShaderFilePath, ShaderSourceFileSize))
    {
        LOGI("Failed to compile vertex shader\n");
        return false;
    }
    
    // Create and compile fragment shader.
    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fragmentShaderFilePath, ShaderSourceFileSize)) {
        LOGI("Failed to compile fragment shader\n");
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
        LOGI("Failed to link program: %d", _program);
        
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
    uniforms4FaceTex[FACETEX_UNIFORM_MODELVIEWPROJECTION_MATRIX] = glGetUniformLocation(_program, "modelViewProjectionMatrix");
    uniforms4FaceTex[FACETEX_UNIFORM_SAMPLERFACE] = glGetUniformLocation(_program, "samplerFace");
    //uniforms4FaceTex[FACETEX_UNIFORM_SAMPLERDEPTH] = glGetUniformLocation(_program, "samplerDepth");
    
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    _program4FaceTex = _program;
    
    return true;
}

bool TexGenerator::loadShader4FaceImage(const char * vertexShaderFilePath, const char * fragmentShaderFilePath)
{
	GLCheck(TexGenerator::loadShader4FaceImage);
    GLuint vertShader, fragShader;
    
    // Create shader program.
    GLuint _program = glCreateProgram();
    
    // Create and compile vertex shader.
    if(!compileShader(&vertShader, GL_VERTEX_SHADER, vertexShaderFilePath, ShaderSourceFileSize))
    {
        LOGI("Failed to compile vertex shader\n");
        return false;
    }
    
    // Create and compile fragment shader.
    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fragmentShaderFilePath, ShaderSourceFileSize)) {
        LOGI("Failed to compile fragment shader\n");
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
        LOGI("Failed to link program: %d", _program);
        
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
    uniforms4FaceImage[FACEIMAGE_UNIFORM_MODELVIEWPROJECTION_MATRIX] = glGetUniformLocation(_program, "modelViewProjectionMatrix");
    uniforms4FaceImage[FACEIMAGE_UNIFORM_SAMPLERFACE] = glGetUniformLocation(_program, "samplerFace");
    
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    _program4FaceImage = _program;
    
    return true;
}

bool TexGenerator::loadShader4FaceGradient(const char * vertexShaderFilePath, const char * fragmentShaderFilePath)
{
	GLCheck(TexGenerator::loadShader4FaceGradient);
	GLuint vertShader, fragShader;
	
	// Create shader program.
	GLuint _program = glCreateProgram();
	
	// Create and compile vertex shader.
	if(!compileShader(&vertShader, GL_VERTEX_SHADER, vertexShaderFilePath, ShaderSourceFileSize))
	{
	    LOGI("Failed to compile vertex shader\n");
	    return false;
	}
	
	// Create and compile fragment shader.
	if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fragmentShaderFilePath, ShaderSourceFileSize)) {
	    LOGI("Failed to compile fragment shader\n");
	    return false;
	}
	
	// Attach vertex shader to program.
	glAttachShader(_program, vertShader);
	
	// Attach fragment shader to program.
	glAttachShader(_program, fragShader);
	
	// Bind attribute locations.
	// This needs to be done prior to linking.
	glBindAttribLocation(_program, HVertexAttribPosition, "position");
	glBindAttribLocation(_program, HVertexAttribNormal, "normalIn");
	
	// Link program.
	if (!linkProgram(_program)) {
	    LOGI("Failed to link program: %d", _program);
	    
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
	uniforms4FaceGradient[FACEGRADIENT_UNIFORM_MODELVIEWPROJECTION_MATRIX] = glGetUniformLocation(_program, "modelViewProjectionMatrix");
	
	// Release vertex and fragment shaders.
	if (vertShader) {
	    glDetachShader(_program, vertShader);
	    glDeleteShader(vertShader);
	}
	if (fragShader) {
	    glDetachShader(_program, fragShader);
	    glDeleteShader(fragShader);
	}
	
	_program4FaceGradient = _program;
	
	return true;
}

// interface
void TexGenerator::generateFaceTextureFromImage(const unsigned char *inputImageData,  int imageWidth, int imageHeight, const char *objModelString, const char *transformDataFileString, const char * VFMapFileString, const char *outputImageFilePath)
{
    //CFAbsoluteTime startTime, endTime;
    
    //startTime = CFAbsoluteTimeGetCurrent();
    if( (_HeadTextureWidth * _HeadTextureHeight) != (TextureWidth * TextureHeight * 4) )
    {
        if(_HeadTextureData)
            delete [] _HeadTextureData;
        _HeadTextureData = new unsigned char[TextureWidth * TextureHeight * 4];
        _HeadTextureWidth = TextureWidth;
        _HeadTextureHeight = TextureHeight;
    }
    //create origin texture
    projectImageToTextureByGLForOBJModelPath(objModelString, transformDataFileString, VFMapFileString, inputImageData, imageWidth, imageHeight, _HeadTextureData, TextureWidth, TextureHeight);
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("    render to tex: %lf s.\n", endTime - startTime);
    
    //post processing the texture
    handleGLTextureData(_HeadTextureData, TextureWidth, TextureHeight);
}

void TexGenerator::generateBackgroundTextureFromImage(const unsigned char *inputImageData, int srcWidth, int srcHeight, const char *outputImageFilePath)
{
    //CFAbsoluteTime startTime, endTime;
    
    //startTime = CFAbsoluteTimeGetCurrent();
    if( (_BGTextureWidth * _BGTextureHeight) != (srcWidth * srcHeight * 4) )
    {
        if(_BGTextureData)
            delete [] _BGTextureData;
        _BGTextureData = new unsigned char[srcWidth * srcHeight * 4];
        _BGTextureWidth = srcWidth;
        _BGTextureHeight = srcHeight;
    }
    memcpy(_BGTextureData, inputImageData, srcWidth * srcHeight * 4);
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("    cpy img: %lf s.\n", endTime - startTime);
    
    if(_BGTextureData)
    {
        int srcTotalNum = srcWidth * srcHeight;
        if(srcTotalNum > AllowedImagePixelNum)
        {
            float srcRadio = ((float)srcWidth) / ((float)srcHeight);
            int scaledWidth = (int)sqrtf(AllowedImagePixelNum * srcRadio);
            int scaledHeight = scaledWidth / srcRadio;
            unsigned char * scaledImageData = new unsigned char[scaledWidth * scaledHeight * 4];
            LOGI("scaled bg image size (%d %d)\n", scaledWidth, scaledHeight);
            
            //LOGI("Mode 1:\nsrc (%d %d) total %d\ndst (%d %d) total %d\n", srcWidth, srcHeight, srcWidth * srcHeight, scaledWidth, scaledHeight, scaledWidth * scaledHeight);
            
            //startTime = CFAbsoluteTimeGetCurrent();
            //downsample image
            scaleImageData(_BGTextureData, srcWidth, srcHeight, scaledImageData, scaledWidth, scaledHeight);
            //endTime = CFAbsoluteTimeGetCurrent();
            //LOGI("    down sample: %lf s.\n", endTime - startTime);
            
            //startTime = CFAbsoluteTimeGetCurrent();
            //post processing the texture
            processBackgourndTextureData(scaledImageData, scaledWidth, scaledHeight);
            //processBackgourndTextureDataUsingEigen(scaledImageData, scaledWidth, scaledHeight);
            //processTextureMVC(scaledImageData, scaledWidth, scaledHeight);
            //saveImageDataToFile(scaledImageData, scaledWidth, scaledHeight, outputImageFilePath);
            //endTime = CFAbsoluteTimeGetCurrent();
            //LOGI("    evaluate: %lf s.\n", endTime - startTime);
            
            //startTime = CFAbsoluteTimeGetCurrent();
            //upsample image
            unsigned char * upsampledImageData = new unsigned char[srcWidth * srcHeight * 4];
            scaleImageData(scaledImageData, scaledWidth, scaledHeight, upsampledImageData, srcWidth, srcHeight);
            delete [] scaledImageData;
            //endTime = CFAbsoluteTimeGetCurrent();
            //LOGI("    upsample: %lf s.\n", endTime - startTime);
            
            //startTime = CFAbsoluteTimeGetCurrent();
            //composition
            doCompositionWithUpsampledImage(upsampledImageData, _BGTextureData, srcWidth, srcHeight);
            delete [] upsampledImageData;
            //endTime = CFAbsoluteTimeGetCurrent();
            //LOGI("    composite: %lf s.\n", endTime - startTime);
        }
        else
        {
            LOGI("Mode 2:\n src (%d %d) total %d\n", srcWidth, srcHeight, srcWidth * srcHeight);
            
            //startTime = CFAbsoluteTimeGetCurrent();
            //post processing the texture
            processBackgourndTextureData(_BGTextureData, srcWidth, srcHeight);
            //processTextureMVC(_BGTextureData, srcWidth, srcHeight);
            //endTime = CFAbsoluteTimeGetCurrent();
            //LOGI("    evaluate: %lf s.\n", endTime - startTime);
        }
    }
}

const unsigned char * TexGenerator::getHeadTextureInfo(int & width, int & height)
{
    width = _HeadTextureWidth;
    height = _HeadTextureHeight;
    return _HeadTextureData;
}

const unsigned char * TexGenerator::getBackgroundTextureInfo(int & width, int & height)
{
    width = _BGTextureWidth;
    height = _BGTextureHeight;
    return _BGTextureData;
}

// logic
void TexGenerator::projectImageToTextureByGLForOBJModelPath(const char * objFilePath, const char * transformFilePath, const char * VFMapFileString, const unsigned char * inputImageData, int imageWidth, int imageHeight, unsigned char * rstTextureData, GLint textureWidth, GLint textureHeight)
{
//    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask ,YES);
//    NSString *documentsDirectory = paths[0];
//    NSString * drawingsDirectory = [documentsDirectory stringByAppendingPathComponent:@"work"];
    
//    const char * inputImagePath = [[[drawingsDirectory stringByAppendingPathComponent:@"inImage"] stringByAppendingPathExtension:@"png"] UTF8String];
//    saveImageDataToFile(inputImageData, imageWidth, imageHeight, inputImagePath);
    
    // create model vertex buffer, vertexArray and source texture
    GLuint vertexArray4FaceTex;
    GLuint vertexBuffer4FaceTex;
    GLuint texture4FaceTex;
    
    TexHeadModel * headModel = new TexHeadModel();
    bool isSuccess = headModel->loadModelFromFile(objFilePath, transformFilePath);
    if(!isSuccess)
    {
        LOGI("Failed to load head model.\n");
        return;
    }
    
    glGenBuffers(1, &vertexBuffer4FaceTex);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer4FaceTex);
    glBufferData(GL_ARRAY_BUFFER, headModel->getVertexNum() * TexHeadModel::getVertexUnitSize(), headModel->getVertexDataArray(), GL_STATIC_DRAW);
    
    headModel->clearVertexDataArray();
    
    glGenVertexArraysOES(1, &vertexArray4FaceTex);
    glBindVertexArrayOES(vertexArray4FaceTex);
    
    glEnableVertexAttribArray(HVertexAttribPosition);;
    glVertexAttribPointer(HVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, TexHeadModel::getVertexUnitSize(), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(HVertexAttribTexCoord0);
    glVertexAttribPointer(HVertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, TexHeadModel::getVertexUnitSize(), BUFFER_OFFSET(sizeof(float) * 3));
    
    loadTextureFromImageData(inputImageData, imageWidth, imageHeight, &texture4FaceTex);
    
    //evaluate transformation matrix
    TransformMatrix * transformMatrix = headModel->getTransformMatrix();
    float scaleFactor = (*transformMatrix).scaleFactor;
    float xTrans = (*transformMatrix).positionTrans[0];
    float yTrans = (*transformMatrix).positionTrans[1];
    float zTrans = (*transformMatrix).positionTrans[2];
    
    glm::mat4 head_ModelViewProjectionMatrix4 = glm::ortho(0.0f, (float)imageWidth, 0.0f, (float)imageHeight, 0.1f, 5000.0f);
    head_ModelViewProjectionMatrix4 = glm::translate(head_ModelViewProjectionMatrix4, glm::vec3(xTrans, yTrans, zTrans - 3000.0f));
    head_ModelViewProjectionMatrix4 = glm::scale(head_ModelViewProjectionMatrix4, glm::vec3(scaleFactor));
    
    //create frame buffer and texture to be rendered
    
    GLuint frameBuffer;
    GLuint targetTexture;
    GLuint depthBuffer;
    
    glGenTextures(1, &targetTexture);
    glBindTexture(GL_TEXTURE_2D , targetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth, textureHeight);
    
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    //render to texture
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);
    if(!checkCurrenFrameBufferStatus())
        return;
    
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindVertexArrayOES(vertexArray4FaceTex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture4FaceTex);

    glUseProgram(_program4FaceTex);
    glUniformMatrix4fv(uniforms4FaceTex[FACETEX_UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, glm::value_ptr(head_ModelViewProjectionMatrix4));
    glUniform1i(uniforms4FaceTex[FACETEX_UNIFORM_SAMPLERFACE], 0);
    
    glViewport(0, 0, textureWidth, textureHeight);
    
    glDrawArrays(GL_TRIANGLES, 0, headModel->getVertexNum());
    
    glReadPixels(0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, rstTextureData);
    
    delete headModel;
    
    resetAllBinds();
    glUseProgram(0);
    
    glDeleteVertexArraysOES(1, &vertexArray4FaceTex);
    glDeleteBuffers(1, &vertexBuffer4FaceTex) ;
    glDeleteTextures(1, &texture4FaceTex);
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteTextures(1, &targetTexture);
    glDeleteFramebuffers(1, &frameBuffer);
    
//    const char * oriImagePath = [[[drawingsDirectory stringByAppendingPathComponent:@"oriImage"] stringByAppendingPathExtension:@"png"] UTF8String];
//    saveImageDataToFile(rstTextureData, textureWidth, textureHeight, oriImagePath);
}

void TexGenerator::handleGLTextureData(unsigned char * imageData, int width, int height)
{
//    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask ,YES);
//    NSString *documentsDirectory = paths[0];
//    NSString * drawingsDirectory = [documentsDirectory stringByAppendingPathComponent:@"work"];

    //CFAbsoluteTime startTime, endTime;
    
    //startTime = CFAbsoluteTimeGetCurrent();
    //flip the imageData and get CGImageRef
    flipImageData(imageData, width, height);
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("    flip Image: %lf s.\n", endTime - startTime);
    
//    const char * glImagePath = [[[drawingsDirectory stringByAppendingPathComponent:@"glImage"] stringByAppendingPathExtension:@"png"] UTF8String];
//    saveImageDataToFile(imageData, width, height, glImagePath);
    
    //startTime = CFAbsoluteTimeGetCurrent();
    //generate PossionTexSize * PossionTexSize image
    unsigned char * scaledImageData = new unsigned char[PossionTexWidth * PossionTexHeight * 4];
    scaleImageData(imageData, width, height, scaledImageData, PossionTexWidth, PossionTexHeight);
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("    downsample: %lf s.\n", endTime - startTime);
    
    //startTime = CFAbsoluteTimeGetCurrent();
    //do possion with scaledImageData
    doPossionProcessingWithImage(scaledImageData, _Laplacian, PossionTexWidth, PossionTexHeight);
    //processTextureMVCWithLaplacian(scaledImageData, _LaplacianImageData, PossionTexWidth, PossionTexHeight);
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("    evaluate: %lf s.\n", endTime - startTime);
    
    //startTime = CFAbsoluteTimeGetCurrent();
    //upsample scaledImageData
    unsigned char * upsampledImageData = new unsigned char[width * height * 4];
    scaleImageData(scaledImageData, PossionTexWidth, PossionTexHeight, upsampledImageData, width, height);
    delete [] scaledImageData;
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("    upsample: %lf s.\n", endTime - startTime);

    //startTime = CFAbsoluteTimeGetCurrent();
    //composition
    doCompositionWithUpsampledImage(upsampledImageData, imageData, width, height);
    delete [] upsampledImageData;
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("    composite: %lf s.\n", endTime - startTime);
}

void TexGenerator::doPossionProcessingWithImage(unsigned char * imageData, Laplacian * laplacian, int width, int height)
{
    //CFAbsoluteTime startTime, endTime;
    
    //startTime = CFAbsoluteTimeGetCurrent();
    
    const int size = width * height;
    const int totalImageBytes = size * PixelChannels;
    const int xOffset[4] = {0, -1, +1, 0};
    const int yOffset[4] = {-1, 0, 0, +1};
    int indexOffset[4];
    int anchorPointOffset[4];
    
    indexOffset[0] = -width;
    indexOffset[1] = -1;
    indexOffset[2] = +1;
    indexOffset[3] = +width;
    
    anchorPointOffset[0] = -width * PixelChannels;
    anchorPointOffset[1] = -PixelChannels;
    anchorPointOffset[2] = PixelChannels;
    anchorPointOffset[3] = width * PixelChannels;
    
    
    //count poisson pixel num
    int possionNum = 0;
    for(int i=3; i<totalImageBytes; i+=PixelChannels)
        if(imageData[i] < 0xff)
            possionNum++;
    
    if(possionNum == 0)
    {
        LOGI("No need to perform head texture Poisson processing.\n");
        return;
    }
    
    // create poisson needed pixel maps between poisson index and image index
	int * mapPoisson2Tex = (int *)malloc(possionNum * sizeof(int));
	int * mapTex2Poisson = (int *)malloc(size * sizeof(int));
	int pIndex = 0;
    int anchorPoint = 3;
	for(int i=0; i<size; i++)
    {
		if(imageData[anchorPoint] < 0xff)
		{
			mapTex2Poisson[i]= pIndex;
			mapPoisson2Tex[pIndex++] = i;
        }
        
        anchorPoint += PixelChannels;
    }
    LOGI("        F\n");
    LOGI("        possion num = %d\n", possionNum);
    
    //endTime = CFAbsoluteTimeGetCurrent();
    
    
    //create and init a, x and b
    //startTime = CFAbsoluteTimeGetCurrent();
    
    
    int * Ap = (int *)malloc((possionNum + 1) * sizeof(int));
    int * Ai = (int *)malloc((possionNum * 5) * sizeof(int));
    double * Ax = (double *)malloc((possionNum * 5) * sizeof(double));
    double * bR = (double *)malloc(possionNum * sizeof(double));
    double * bG = (double *)malloc(possionNum * sizeof(double));
    double * bB = (double *)malloc(possionNum * sizeof(double));
    double * xR = (double *)malloc(possionNum * sizeof(double));
    double * xG = (double *)malloc(possionNum * sizeof(double));
    double * xB = (double *)malloc(possionNum * sizeof(double));
    
    const float * channelR = laplacian->getRedChannel();
    const float * channelG = laplacian->getGreenChannel();
    const float * channelB = laplacian->getBlueChannel();
    int selfIndex, selfPosX, selfPosY;
    int neiborCount = 0;
    int neiborNeedPossionCount = 0;
    int noneNeiborSumR, noneNeiborSumG, noneNeiborSumB;
    int neiborPosX, neiborPosY;
    bool isNeiborAvalable[4];
    int neiborAnchorPoint = 0;
    int AIndex = 0;
    anchorPoint =0;
    
    Ap[0] = 0;
    
    for(int i=0; i<possionNum; i++)
    {
        selfIndex = mapPoisson2Tex[i];
        selfPosX = selfIndex % width;
        selfPosY = selfIndex / width;
        anchorPoint = selfIndex * PixelChannels;
        // for point (selfPosX, selfPosY)
        // count neibor
        neiborCount = 0;
        neiborNeedPossionCount = 0;
        noneNeiborSumR = noneNeiborSumG = noneNeiborSumB = 0;
        for(int j=0; j<4; j++)
        {
            neiborPosX = selfPosX + xOffset[j];
            neiborPosY = selfPosY + yOffset[j];
            isNeiborAvalable[j] = false;
            
            if( (neiborPosX >= 0 && neiborPosX < width) && (neiborPosY >= 0 && neiborPosY < height) )
            {
                neiborCount++;
                
                neiborAnchorPoint = anchorPoint + anchorPointOffset[j];
                if(imageData[neiborAnchorPoint + 3] == 0xff)
                {
                    noneNeiborSumR += imageData[neiborAnchorPoint];
                    noneNeiborSumG += imageData[neiborAnchorPoint + 1];
                    noneNeiborSumB += imageData[neiborAnchorPoint + 2];
                }
                else
                {
                    isNeiborAvalable[j] = true;
                    neiborNeedPossionCount++;
                }
            }
        }
        Ap[i + 1] = Ap[i] + neiborNeedPossionCount + 1;
        
        // fill matrix
        for(int j=0; j<2; j++)
        {
            if(isNeiborAvalable[j])
            {
                Ai[AIndex] = mapTex2Poisson[selfIndex + indexOffset[j]];
                Ax[AIndex++] = -1.0;
            }
            
        }
        
        Ai[AIndex] = i;
        Ax[AIndex++] = (double)neiborCount;
        
        for(int j=2; j<4; j++)
        {
            if(isNeiborAvalable[j])
            {
                Ai[AIndex] = mapTex2Poisson[selfIndex + indexOffset[j]];
                Ax[AIndex++] = -1.0;
            }
        }
        
        bR[i] = channelR[selfIndex] + noneNeiborSumR;
        bG[i] = channelG[selfIndex] + noneNeiborSumG;
        bB[i] = channelB[selfIndex] + noneNeiborSumB;
    }
    //endTime = CFAbsoluteTimeGetCurrent();
    
    
    //perform lv factor
    //startTime = CFAbsoluteTimeGetCurrent();
    double Info [UMFPACK_INFO], Control [UMFPACK_CONTROL];
    int status;
    void * Symbolic, *Numeric;
    umfpack_di_defaults (Control) ;
    Control [UMFPACK_PRL] = 6 ;
    Control [UMFPACK_PRL] = 5 ;
    status = umfpack_di_symbolic(possionNum, possionNum, Ap, Ai, Ax, &Symbolic, Control, Info);
    if (status < 0)
    {
        umfpack_di_report_info (Control, Info) ;
        umfpack_di_report_status (Control, status) ;
        LOGI("umfpack_di_symbolic failed\n");
    }
    status = umfpack_di_numeric(Ap, Ai, Ax, Symbolic, &Numeric, Control, Info);
    if (status < 0)
    {
        //umfpack_di_report_info (Control, Info) ;
        //umfpack_di_report_status (Control, status) ;
        //LOGI("umfpack_di_numeric failed\n") ;
    }
    umfpack_di_free_symbolic(&Symbolic);
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        lu factor: %lf s\n", endTime - startTime);
    
    
    //evaluation
    //startTime = CFAbsoluteTimeGetCurrent();
    status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, xR, bR, Numeric, Control, Info);
    if (status < 0)
    {
        LOGI("umfpack_di_solve Red failed\n") ;
    }
    
    status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, xG, bG, Numeric, Control, Info);
    if (status < 0)
    {
        LOGI("umfpack_di_solve Green failed\n") ;
    }
    
    status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, xB, bB, Numeric, Control, Info);
    if (status < 0)
    {
        LOGI("umfpack_di_solve Blue failed\n") ;
    }
    
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        cal channels: %lf\n", endTime - startTime);
    
    umfpack_di_free_numeric(&Numeric);
    
    //write back to imageData
    //startTime = CFAbsoluteTimeGetCurrent();
    
    int R, G, B;
    for(int i=0; i<possionNum; i++)
    {
        R = (int)xR[i];
        G = (int)xG[i];
        B = (int)xB[i];
        
        R = Min(Max(R, 0), 255);
        G = Min(Max(G, 0), 255);
        B = Min(Max(B, 0), 255);
        
        anchorPoint = mapPoisson2Tex[i] * PixelChannels;
        
        imageData[anchorPoint] = R;
        imageData[anchorPoint + 1] = G;
        imageData[anchorPoint + 2] = B;
        imageData[anchorPoint + 3] = 0xff;
    }
    
    //release memory
    free(mapTex2Poisson);
    free(mapPoisson2Tex);
    free(Ap);
    free(Ai);
    free(Ax);
    free(bR);
    free(bG);
    free(bB);
    free(xR);
    free(xG);
    free(xB);
    
    //endTime = CFAbsoluteTimeGetCurrent();
}

void TexGenerator::doCompositionWithUpsampledImage(const unsigned char * upsampledImageData, unsigned char * targetImageData, int width, int height)
{
    const int byteSize = width * height * PixelChannels;
    for(int anchorPoint=0; anchorPoint<byteSize; anchorPoint += PixelChannels)
    {
        if(targetImageData[anchorPoint + 3] < 0xff)
        {
            targetImageData[anchorPoint] = upsampledImageData[anchorPoint];
            targetImageData[anchorPoint + 1] = upsampledImageData[anchorPoint + 1];
            targetImageData[anchorPoint + 2] = upsampledImageData[anchorPoint + 2];
            targetImageData[anchorPoint + 3] = 0xff;
        }
    }
}

void TexGenerator::processBackgourndTextureData(unsigned char * imageData, int width, int height)
{
    //CFAbsoluteTime startTime, endTime;
    
    //startTime = CFAbsoluteTimeGetCurrent();
    
    const int size = width * height;
    const int totalImageBytes = size * PixelChannels;
    const int xOffset[4] = {0, -1, +1, 0};
    const int yOffset[4] = {-1, 0, 0, +1};
    int indexOffset[4];
    int anchorPointOffset[4];
    
    indexOffset[0] = -width;
    indexOffset[1] = -1;
    indexOffset[2] = +1;
    indexOffset[3] = +width;
    
    anchorPointOffset[0] = -width * PixelChannels;
    anchorPointOffset[1] = -PixelChannels;
    anchorPointOffset[2] = PixelChannels;
    anchorPointOffset[3] = width * PixelChannels;
    
    
    //count poisson pixel num
    int possionNum = 0;
    for(int i=3; i<totalImageBytes; i+=PixelChannels)
        if(imageData[i] < 0xff)
            possionNum++;
    
    if(possionNum == 0)
    {
        LOGI("No need to perform background Poisson processing.\n");
        return;
    }
    
    // create poisson needed pixel maps between poisson index and image index
	int * mapPoisson2Tex = (int *)malloc(possionNum *sizeof(int));
	int * mapTex2Poisson = (int *)malloc(size * sizeof(int));
	int pIndex = 0;
    int anchorPoint = 3;
	for(int i=0; i<size; i++)
    {
		if(imageData[anchorPoint] < 0xff)
		{
			mapTex2Poisson[i]= pIndex;
			mapPoisson2Tex[pIndex++] = i;
        }
        
        anchorPoint += PixelChannels;
    }
    //LOGI("        B\n");
    //LOGI("        possion num = %d\n", possionNum);
    
    //endTime = CFAbsoluteTimeGetCurrent();
    
    //create and init a, x and b
    //startTime = CFAbsoluteTimeGetCurrent();
    
    
    int * Ap = (int *)malloc((possionNum + 1) * sizeof(int));
    int * Ai = (int *)malloc((possionNum * 5) * sizeof(int));
    double * Ax = (double *)malloc((possionNum * 5) * sizeof(double));
    double * bR = (double *)malloc(possionNum * sizeof(double));
    double * bG = (double *)malloc(possionNum * sizeof(double));
    double * bB = (double *)malloc(possionNum * sizeof(double));
    double * xR = (double *)malloc(possionNum * sizeof(double));
    double * xG = (double *)malloc(possionNum * sizeof(double));
    double * xB = (double *)malloc(possionNum * sizeof(double));
    
    
    int selfIndex, selfPosX, selfPosY;
    int neiborCount = 0;
    int neiborNeedPossionCount = 0;
    int noneNeiborSumR, noneNeiborSumG, noneNeiborSumB;
    int neiborPosX, neiborPosY;
    bool isNeiborAvalable[4];
    int neiborAnchorPoint = 0;
    int AIndex = 0;
    anchorPoint =0;
    
    Ap[0] = 0;
    
    for(int i=0; i<possionNum; i++)
    {
        selfIndex = mapPoisson2Tex[i];
        selfPosX = selfIndex % width;
        selfPosY = selfIndex / width;
        anchorPoint = selfIndex * PixelChannels;
        // for point (selfPosX, selfPosY)
        
        // count neibor
        neiborCount = 0;
        neiborNeedPossionCount = 0;
        noneNeiborSumR = noneNeiborSumG = noneNeiborSumB = 0;
        for(int j=0; j<4; j++)
        {
            neiborPosX = selfPosX + xOffset[j];
            neiborPosY = selfPosY + yOffset[j];
            isNeiborAvalable[j] = false;
            
            if( (neiborPosX >= 0 && neiborPosX < width) && (neiborPosY >= 0 && neiborPosY < height) )
            {
                neiborCount++;
                
                neiborAnchorPoint = anchorPoint + anchorPointOffset[j];
                if(imageData[neiborAnchorPoint + 3] == 0xff)
                {
                    noneNeiborSumR += imageData[neiborAnchorPoint];
                    noneNeiborSumG += imageData[neiborAnchorPoint + 1];
                    noneNeiborSumB += imageData[neiborAnchorPoint + 2];
                }
                else
                {
                    isNeiborAvalable[j] = true;
                    neiborNeedPossionCount++;
                }
            }
        }
        
        Ap[i + 1] = Ap[i] + neiborNeedPossionCount + 1;
        
        // fill matrix
        for(int j=0; j<2; j++)
        {
            if(isNeiborAvalable[j])
            {
                Ai[AIndex] = mapTex2Poisson[selfIndex + indexOffset[j]];
                Ax[AIndex++] = -1.0;
            }
            
        }
        
        Ai[AIndex] = i;
        Ax[AIndex++] = (double)neiborCount;
        
        for(int j=2; j<4; j++)
        {
            if(isNeiborAvalable[j])
            {
                Ai[AIndex] = mapTex2Poisson[selfIndex + indexOffset[j]];
                Ax[AIndex++] = -1.0;
            }
        }
        
        bR[i] = noneNeiborSumR;
        bG[i] = noneNeiborSumG;
        bB[i] = noneNeiborSumB;
    }
    //endTime = CFAbsoluteTimeGetCurrent();
    
    //startTime = CFAbsoluteTimeGetCurrent();
    
    double Info [UMFPACK_INFO], Control [UMFPACK_CONTROL];
    int status;
    void * Symbolic, *Numeric;
    
    umfpack_di_defaults (Control) ;
    Control [UMFPACK_PRL] = 6 ;
    Control [UMFPACK_PRL] = 5 ;
    
    status = umfpack_di_symbolic(possionNum, possionNum, Ap, Ai, Ax, &Symbolic, Control, Info);
    if (status < 0)
    {
        umfpack_di_report_info (Control, Info) ;
        umfpack_di_report_status (Control, status) ;
        LOGI("umfpack_di_symbolic failed\n");
    }
    
    
    status = umfpack_di_numeric(Ap, Ai, Ax, Symbolic, &Numeric, Control, Info);
    if (status < 0)
    {
        //umfpack_di_report_info (Control, Info) ;
        //umfpack_di_report_status (Control, status) ;
        //LOGI("umfpack_di_numeric failed\n") ;
    }
    umfpack_di_free_symbolic(&Symbolic);
    
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        lu factor: %lf s\n", endTime - startTime);
    
    
    //evaluation
    //startTime = CFAbsoluteTimeGetCurrent();
    status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, xR, bR, Numeric, Control, Info);
    if (status < 0)
    {
        LOGI("umfpack_di_solve Red failed\n") ;
    }
    status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, xG, bG, Numeric, Control, Info);
    if (status < 0)
    {
        LOGI("umfpack_di_solve Green failed\n") ;
    }
    
    status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, xB, bB, Numeric, Control, Info);
    if (status < 0)
    {
        LOGI("umfpack_di_solve Blue failed\n") ;
    }
    
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        cal channels: %lf\n", endTime - startTime);
    
    umfpack_di_free_numeric(&Numeric);
    
    //write back to imageData
    //startTime = CFAbsoluteTimeGetCurrent();
    
    int R, G, B;
    for(int i=0; i<possionNum; i++)
    {
        R = (int)xR[i];
        G = (int)xG[i];
        B = (int)xB[i];
        
        R = Min(Max(R, 0), 255);
        G = Min(Max(G, 0), 255);
        B = Min(Max(B, 0), 255);
        
        anchorPoint = mapPoisson2Tex[i] * PixelChannels;
        
        imageData[anchorPoint] = R;
        imageData[anchorPoint + 1] = G;
        imageData[anchorPoint + 2] = B;
        imageData[anchorPoint + 3] = 0xff;
    }
    
    //release memory
    free(mapTex2Poisson);
    free(mapPoisson2Tex);
    free(Ap);
    free(Ai);
    free(Ax);
    free(bR);
    free(bG);
    free(bB);
    free(xR);
    free(xG);
    free(xB);
    
    //endTime = CFAbsoluteTimeGetCurrent();
}

/*
void TexGenerator::processBackgourndTextureDataUsingEigen(unsigned char * imageData, int width, int height)
{
    //CFAbsoluteTime startTime, endTime;
    
    //startTime = CFAbsoluteTimeGetCurrent();
    
    const int size = width * height;
    const int totalImageBytes = size * PixelChannels;
    const int xOffset[4] = {0, -1, +1, 0};
    const int yOffset[4] = {-1, 0, 0, +1};
    int indexOffset[4];
    int anchorPointOffset[4];
    
    indexOffset[0] = -width;
    indexOffset[1] = -1;
    indexOffset[2] = +1;
    indexOffset[3] = +width;
    
    anchorPointOffset[0] = -width * PixelChannels;
    anchorPointOffset[1] = -PixelChannels;
    anchorPointOffset[2] = PixelChannels;
    anchorPointOffset[3] = width * PixelChannels;
    
    
    //count poisson pixel num
    int possionNum = 0;
    for(int i=3; i<totalImageBytes; i+=PixelChannels)
        if(imageData[i] < 0xff)
            possionNum++;
    
    if(possionNum == 0)
    {
        LOGI("No need to perform background Poisson processing.\n");
        return;
    }
    
    // create poisson needed pixel maps between poisson index and image index
	int * mapPoisson2Tex = (int *)malloc(possionNum *sizeof(int));
	int * mapTex2Poisson = (int *)malloc(size * sizeof(int));
	int pIndex = 0;
    int anchorPoint = 3;
	for(int i=0; i<size; i++)
    {
		if(imageData[anchorPoint] < 0xff)
		{
			mapTex2Poisson[i]= pIndex;
			mapPoisson2Tex[pIndex++] = i;
        }
        
        anchorPoint += PixelChannels;
    }
    LOGI("        B\n");
    LOGI("        possion num = %d\n", possionNum);
    
    //endTime = CFAbsoluteTimeGetCurrent();
    
    //create and init a, x and b
    //startTime = CFAbsoluteTimeGetCurrent();
    
    Eigen::SparseMatrix<float> A(possionNum, possionNum);
    A.reserve(Eigen::VectorXi::Constant(possionNum, 5));
    Eigen::VectorXf bR(possionNum);
    Eigen::VectorXf bG(possionNum);
    Eigen::VectorXf bB(possionNum);
    Eigen::VectorXf xR(possionNum);
    Eigen::VectorXf xG(possionNum);
    Eigen::VectorXf xB(possionNum);
    
    
    int selfIndex, selfPosX, selfPosY;
    int neiborCount = 0;
    int neiborNeedPossionCount = 0;
    int noneNeiborSumR, noneNeiborSumG, noneNeiborSumB;
    int neiborPosX, neiborPosY, neiborIndex;
    bool isNeiborAvalable[4];
    int neiborAnchorPoint = 0;
    anchorPoint =0;

    
    for(int i=0; i<possionNum; i++)
    {
        selfIndex = mapPoisson2Tex[i];
        selfPosX = selfIndex % width;
        selfPosY = selfIndex / width;
        anchorPoint = selfIndex * PixelChannels;
        // for point (selfPosX, selfPosY)
        
        // count neibor
        neiborCount = 0;
        neiborNeedPossionCount = 0;
        noneNeiborSumR = noneNeiborSumG = noneNeiborSumB = 0;
        for(int j=0; j<4; j++)
        {
            neiborPosX = selfPosX + xOffset[j];
            neiborPosY = selfPosY + yOffset[j];
            isNeiborAvalable[j] = false;
            
            if( (neiborPosX >= 0 && neiborPosX < width) && (neiborPosY >= 0 && neiborPosY < height) )
            {
                neiborCount++;
                
                neiborAnchorPoint = anchorPoint + anchorPointOffset[j];
                if(imageData[neiborAnchorPoint + 3] == 0xff)
                {
                    noneNeiborSumR += imageData[neiborAnchorPoint];
                    noneNeiborSumG += imageData[neiborAnchorPoint + 1];
                    noneNeiborSumB += imageData[neiborAnchorPoint + 2];
                }
                else
                {
                    isNeiborAvalable[j] = true;
                    neiborNeedPossionCount++;
                }
            }
        }
        
        // fill matrix
        for(int j=0; j<2; j++)
        {
            if(isNeiborAvalable[j])
            {
                neiborIndex = selfIndex + indexOffset[j];
                A.insert(i, mapTex2Poisson[neiborIndex]) = -1.0f;
            }
            
        }
        
        A.insert(i, i) = (float)neiborCount;
        
        for(int j=2; j<4; j++)
        {
            if(isNeiborAvalable[j])
            {
                neiborIndex = selfIndex + indexOffset[j];
                A.insert(i, mapTex2Poisson[neiborIndex]) = -1.0f;
            }
        }
        
        bR(i) = noneNeiborSumR;
        bG(i) = noneNeiborSumG;
        bB(i) = noneNeiborSumB;
    }
    A.makeCompressed();
    //endTime = CFAbsoluteTimeGetCurrent();
    
    //perform lu factor
    //startTime = CFAbsoluteTimeGetCurrent();
    Eigen::SparseLU< Eigen::SparseMatrix<float> > solver;
    solver.compute(A);
    if(solver.info() != Eigen::Success)
    {
        LOGI("Failed to perform lu factor.\n");
        return;
    }
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        lu factor: %lf s\n", endTime - startTime);
    
    
    //evaluation
    //startTime = CFAbsoluteTimeGetCurrent();
    xR = solver.solve(bR);
    if(solver.info() != Eigen::Success)
    {
        LOGI("Failed to evalute red channel.\n");
        return;
    }
    
    xG = solver.solve(bG);
    if(solver.info() != Eigen::Success)
    {
        LOGI("Failed to evalute red channel.\n");
        return;
    }
    
    xB = solver.solve(bB);
    if(solver.info() != Eigen::Success)
    {
        LOGI("Failed to evalute red channel.\n");
        return;
    }

    
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        cal channels: %lf\n", endTime - startTime);
    
    //write back to imageData
    //startTime = CFAbsoluteTimeGetCurrent();
    
    int R, G, B;
    for(int i=0; i<possionNum; i++)
    {
        R = (int)xR(i);
        G = (int)xG(i);
        B = (int)xB(i);
        
        R = Min(Max(R, 0), 255);
        G = Min(Max(G, 0), 255);
        B = Min(Max(B, 0), 255);
        
        anchorPoint = mapPoisson2Tex[i] * PixelChannels;
        
        imageData[anchorPoint] = R;
        imageData[anchorPoint + 1] = G;
        imageData[anchorPoint + 2] = B;
        imageData[anchorPoint + 3] = 0xff;
    }
    
    //release memory
    free(mapTex2Poisson);
    free(mapPoisson2Tex);
    
    //endTime = CFAbsoluteTimeGetCurrent();
}
*/

//　MVC
struct MVCEdgePoint
{
    int x;
    int y;
    int diff[3]; //RGB diff
};

void TexGenerator::processTextureMVCWithLaplacian(unsigned char * imageData, const unsigned char * laplacianImageData, int width, int height)
{
//    _ImageData = new unsigned char[width * height * 4];
//    memcpy(_ImageData, imageData, width*height*4);
//    _MaskData = new unsigned char[width*height];
//    _ImageWidth = width;
//    _ImageHeight = height;
//
//    int arrayTmpIndex = 3;
//    int maskIndex = 0;
//    for(int i=0; i<height;i++)
//        for(int j=0; j<width;j++)
//        {
//            _MaskData[maskIndex++] = (imageData[arrayTmpIndex] >0x80)?0xff:0x00;
//            arrayTmpIndex += 4;
//        }

    const unsigned char alphaThreshold = 0xf8;
    int arrayIndex;
    
    //get contour
    cv::Mat cvMat(height, width, CV_8UC1);
    arrayIndex = 3;
    for(int i=0; i<height;i++)
        for(int j=0; j<width;j++)
        {
            //for pixel (j, i)
            cvMat.at<bool>(i, j) = ((imageData[arrayIndex] >alphaThreshold)?true:false);
            arrayIndex += 4;
        }
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(cvMat, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cv::Point(0, 0));
    cvMat.release();
    
    //init edge array
    int edgePointNum = 0;
    const int contourNum = contours.size();
    MVCEdgePoint * * edgePointArray = new MVCEdgePoint *[contourNum];
    int * contourPointNumArray = new int[contourNum];
    for(int i=0; i<contourNum; i++)
    {
        contourPointNumArray[i] = contours[i].size();
        edgePointArray[i] = new MVCEdgePoint[contours[i].size()];
        edgePointNum += contourPointNumArray[i];
    }
    LOGI("        edge points: %d\n", edgePointNum);
    
    int pixelIndex;
    for(int i=0; i<contourNum; i++)
    {
        std::vector<cv::Point> & contour = contours[i];
        MVCEdgePoint * edgeArrayPtr = edgePointArray[i];
        
        int contourPointNum = contourPointNumArray[i];
        for(int j=0; j<contourPointNum; j++)
        {
            edgeArrayPtr[j].x = contour[j].x;
            edgeArrayPtr[j].y = contour[j].y;
            pixelIndex = ((edgeArrayPtr[j].y * width + edgeArrayPtr[j].x)<<2);
            
            edgeArrayPtr[j].diff[0] = imageData[pixelIndex] - laplacianImageData[pixelIndex];
            edgeArrayPtr[j].diff[1] = imageData[pixelIndex+1] - laplacianImageData[pixelIndex+1];
            edgeArrayPtr[j].diff[2] = imageData[pixelIndex+2] - laplacianImageData[pixelIndex+2];
        }
    }
    contours.clear();
    
    //evaluate
    double weightSum = 0;
    double diffSum[3];
    float tmpWeight = 0;
    int finalColor[3];
    
    pixelIndex = 0;
    int tmpCount = 0;
    for(int i=0; i<height; i++)
        for(int j=0; j<width; j++)
        {
            //for pixel (j, i)
            if(imageData[pixelIndex + 3] <= alphaThreshold)
            {
                tmpCount++;
                
                //in poisson region
                weightSum = 0.0f;
                diffSum[0] = diffSum[1] = diffSum[2] = 0.0f;
                for(int m=0; m<contourNum; m++)
                {
                    int contourPointNum = contourPointNumArray[m];
                    for(int n=0; n<contourPointNum; n++)
                    {
                        //for edge point edgePointArray[m][n]
                        //OpenCV 目测顺时针给点
                        int rightIndex = (contourPointNum -1 + n) % contourPointNum;
                        int leftIndex = (n + 1) % contourPointNum;
                        tmpWeight = evaluateMVCWeight(glm::vec2(j, i),
                                                      glm::vec2(edgePointArray[m][leftIndex].x, edgePointArray[m][leftIndex].y),
                                                      glm::vec2(edgePointArray[m][n].x, edgePointArray[m][n].y),
                                                      glm::vec2(edgePointArray[m][rightIndex].x, edgePointArray[m][rightIndex].y));
                        
                        weightSum += tmpWeight;
                        diffSum[0] += edgePointArray[m][n].diff[0] * tmpWeight;
                        diffSum[1] += edgePointArray[m][n].diff[1] * tmpWeight;
                        diffSum[2] += edgePointArray[m][n].diff[2] * tmpWeight;
                    }
                }
                
                if(weightSum == 0.0f)
                    weightSum = 1.0f;
                diffSum[0] /= weightSum;
                diffSum[1] /= weightSum;
                diffSum[2] /= weightSum;
                
                finalColor[0] = (int)(laplacianImageData[pixelIndex] + diffSum[0]);
                finalColor[1] = (int)(laplacianImageData[pixelIndex+1] + diffSum[1]);
                finalColor[2] = (int)(laplacianImageData[pixelIndex+2] + diffSum[2]);
                
                imageData[pixelIndex] = Max(Min(finalColor[0], 255), 0);
                imageData[pixelIndex+1] = Max(Min(finalColor[1], 255), 0);
                imageData[pixelIndex+2] = Max(Min(finalColor[2], 255), 0);
                imageData[pixelIndex+3] = 0xff;
            }
            
            pixelIndex += 4;
        }
    
    //release memory
    for(int i=0; i<contourNum; i++)
        delete [] edgePointArray[i];
    delete [] edgePointArray;
    delete [] contourPointNumArray;
    
    LOGI("        process point: %d\n", tmpCount);
}

void TexGenerator::processTextureMVC(unsigned char * imageData, int width, int height)
{
    //CFAbsoluteTime startTime, endTime;
    //startTime = CFAbsoluteTimeGetCurrent();
    
    const unsigned char alphaThreshold = 0xf8;
    const unsigned char srcColor = 0x80;
    int arrayIndex;
    
    //get contour
    cv::Mat cvMat(height, width, CV_8UC1);
    arrayIndex = 3;
    for(int i=0; i<height;i++)
        for(int j=0; j<width;j++)
        {
            //for pixel (j, i)
            cvMat.at<bool>(i, j) = ((imageData[arrayIndex] >alphaThreshold)?true:false);
            arrayIndex += 4;
        }
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(cvMat, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cv::Point(0, 0));
    cvMat.release();
    
    //删除图片边框
    if(contours.size() > 1)
    {
        std::vector< std::vector<cv::Point> >::iterator iter = contours.begin() + contours.size() - 1;
        contours.erase(iter);
    }
    
    //init edge array
    int edgePointNum = 0;
    const int contourNum = contours.size();
    MVCEdgePoint * * edgePointArray = new MVCEdgePoint *[contourNum];
    int * contourPointNumArray = new int[contourNum];
    for(int i=0; i<contourNum; i++)
    {
        contourPointNumArray[i] = contours[i].size();
        edgePointArray[i] = new MVCEdgePoint[contours[i].size()];
        edgePointNum += contourPointNumArray[i];
    }
    LOGI("        edge points: %d\n", edgePointNum);
    
    int pixelIndex;
    for(int i=0; i<contourNum; i++)
    {
        std::vector<cv::Point> & contour = contours[i];
        MVCEdgePoint * edgeArrayPtr = edgePointArray[i];
        
        int contourPointNum = contourPointNumArray[i];
        for(int j=0; j<contourPointNum; j++)
        {
            edgeArrayPtr[j].x = contour[j].x;
            edgeArrayPtr[j].y = contour[j].y;
            pixelIndex = ((edgeArrayPtr[j].y * width + edgeArrayPtr[j].x)<<2);
            
            edgeArrayPtr[j].diff[0] = imageData[pixelIndex] - srcColor;
            edgeArrayPtr[j].diff[1] = imageData[pixelIndex+1] - srcColor;
            edgeArrayPtr[j].diff[2] = imageData[pixelIndex+2] - srcColor;
        }
    }
    contours.clear();
    
    //evaluate
    double weightSum = 0;
    double diffSum[3];
    float tmpWeight = 0;
    int finalColor[3];
    
    pixelIndex = 0;
    int tmpCount = 0;
    for(int i=0; i<height; i++)
        for(int j=0; j<width; j++)
        {
            //for pixel (j, i)
            if(imageData[pixelIndex + 3] <= alphaThreshold)
            {
                tmpCount++;
                
                //in poisson region
                weightSum = 0.0f;
                diffSum[0] = diffSum[1] = diffSum[2] = 0.0f;
                for(int m=0; m<contourNum; m++)
                {
                    int contourPointNum = contourPointNumArray[m];
                    for(int n=0; n<contourPointNum; n++)
                    {
                        //for edge point edgePointArray[m][n]
                        int rightIndex = (contourPointNum -1 + n) % contourPointNum;
                        int leftIndex = (n + 1) % contourPointNum;
                        tmpWeight = evaluateMVCWeight(glm::vec2(j, i),
                                                      glm::vec2(edgePointArray[m][leftIndex].x, edgePointArray[m][leftIndex].y),
                                                      glm::vec2(edgePointArray[m][n].x, edgePointArray[m][n].y),
                                                      glm::vec2(edgePointArray[m][rightIndex].x, edgePointArray[m][rightIndex].y));

                        weightSum += tmpWeight;
                        diffSum[0] += edgePointArray[m][n].diff[0] * tmpWeight;
                        diffSum[1] += edgePointArray[m][n].diff[1] * tmpWeight;
                        diffSum[2] += edgePointArray[m][n].diff[2] * tmpWeight;
                    }
                }
                
                if(weightSum == 0.0f)
                    weightSum = 1.0f;
                diffSum[0] /= weightSum;
                diffSum[1] /= weightSum;
                diffSum[2] /= weightSum;
                
                finalColor[0] = (int)(srcColor + diffSum[0]);
                finalColor[1] = (int)(srcColor + diffSum[1]); 
                finalColor[2] = (int)(srcColor + diffSum[2]);
                
                imageData[pixelIndex] = Max(Min(finalColor[0], 255), 0);
                imageData[pixelIndex+1] = Max(Min(finalColor[1], 255), 0);
                imageData[pixelIndex+2] = Max(Min(finalColor[2], 255), 0);
                imageData[pixelIndex+3] = 0xff;
            }
            
            pixelIndex += 4;
        }
    
    //release memory
    for(int i=0; i<contourNum; i++)
        delete [] edgePointArray[i];
    delete [] edgePointArray;
    delete [] contourPointNumArray;
    
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        process point: %d\nmvc: %lf s\n", tmpCount, endTime - startTime);
}

float TexGenerator::evaluateMVCWeight(glm::vec2 v, glm::vec2 leftPoint, glm::vec2 midPoint, glm::vec2 rightPoint)
{
    glm::vec3 vecMid  = glm::vec3(midPoint.x - v.x, midPoint.y - v.y, 0.0f);
    float midLenght = vecMid.length();
    
    if(midLenght == 0.0f)
    {
        return std::numeric_limits<float>::max();
    }
    
    glm::vec3 vecLeft = glm::vec3(leftPoint.x - v.x, leftPoint.y - v.y, 0.0f);
    glm::vec3 vecRight = glm::vec3(rightPoint.x - v.x, rightPoint.y - v.y, 0.0f);
    
    glm::vec3 nLeft = glm::normalize(vecLeft);
    glm::vec3 nMid = glm::normalize(vecMid);
    glm::vec3 nRight = glm::normalize(vecRight);
    
    float cosLeft = glm::dot(nLeft, nMid);
    float sinLeft = glm::cross(nLeft, nMid).length();
    float cosRight = glm::dot(nMid, nRight);
    float sinRight = glm::cross(nMid, nRight).length();
    
    float tanLeft = (1 - cosLeft) / sinLeft;
    float tanRight = (1 - cosRight) / sinRight;
    
    return ((tanLeft + tanRight) / midLenght);
}

//face texture with gradient and noise
void TexGenerator::generateFaceTextureFromImageWithGradient(const unsigned char * inputImageData, const unsigned char * partialFaceImageData, int imageWidth, int imageHeight, const char * objModelString, const char * transformDataFileString, const char * VFMapFileString, const char * outputImageFilePath, float val)
{
//    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask ,YES);
//    NSString *documentsDirectory = paths[0];
//    NSString * drawingsDirectory = [documentsDirectory stringByAppendingPathComponent:@"work"];
    
    LOGI("image size (%d %d)\n", imageWidth, imageHeight);
    
    clock_t subStartTime, subEndTime;
    
    if( (_HeadTextureWidth * _HeadTextureHeight) != (TextureWidth * TextureHeight * 4) )
    {
        if(_HeadTextureData)
            delete [] _HeadTextureData;
        _HeadTextureData = new unsigned char[TextureWidth * TextureHeight * 4];
        _HeadTextureWidth = TextureWidth;
        _HeadTextureHeight = TextureHeight;
    }
    
    LOGI("Pass 1");
    
    //create headVertexArray
    GLuint headVertexArray = 0;
    GLuint headVertexBuffer = 0;
    glm::mat4 head_MVP;
    int headVertexNum;
    
    subStartTime = clock();
    
    TexHeadModel * headModel = new TexHeadModel();
    bool isSuccess = headModel->loadModelFromFile(objModelString, transformDataFileString);
    if(!isSuccess)
    {
        LOGI("Failed to load head model.\n");
        return;
    }
    
    headVertexNum = headModel->getVertexNum();
    
    LOGI("Pass 2");
    
    //evaluate transformation matrix
    TransformMatrix * transformMatrix = headModel->getTransformMatrix();
    float scaleFactor = (*transformMatrix).scaleFactor;
    float xTrans = (*transformMatrix).positionTrans[0];
    float yTrans = (*transformMatrix).positionTrans[1];
    float zTrans = (*transformMatrix).positionTrans[2];
    head_MVP = glm::ortho(0.0f, (float)imageWidth, 0.0f, (float)imageHeight, 0.1f, 5000.0f);
    head_MVP = glm::translate(head_MVP, glm::vec3(xTrans, yTrans, zTrans - 3000.0f));
    head_MVP = glm::scale(head_MVP, glm::vec3(scaleFactor));
    
    glGenBuffers(1, &headVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, headVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, headModel->getVertexNum() * TexHeadModel::getVertexUnitSize(), headModel->getVertexDataArray(), GL_STATIC_DRAW);
    
    headModel->clearVertexDataArray();
    
    LOGI("Pass 3");
    
    glGenVertexArraysOES(1, &headVertexArray);
    glBindVertexArrayOES(headVertexArray);
    
    glEnableVertexAttribArray(HVertexAttribPosition);;
    glVertexAttribPointer(HVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, TexHeadModel::getVertexUnitSize(), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(HVertexAttribTexCoord0);
    glVertexAttribPointer(HVertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, TexHeadModel::getVertexUnitSize(), BUFFER_OFFSET(sizeof(float) * 3));
    glEnableVertexAttribArray(HVertexAttribNormal);
    glVertexAttribPointer(HVertexAttribNormal, 3, GL_FLOAT, GL_FALSE, TexHeadModel::getVertexUnitSize(), BUFFER_OFFSET(sizeof(float) * (3+2)));
    
    subEndTime = clock();
    LOGI("        load Head Data: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    
    subStartTime = clock();
    //create origin texture (big texture)
    projectImageToTexture(headVertexArray, head_MVP, headVertexNum, inputImageData, imageWidth, imageHeight, _HeadTextureData, TextureWidth, TextureHeight);
    subEndTime = clock();
    LOGI("        ori tex: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));    
    
//    char tmpPath[500];
//    sprintf(tmpPath, "%s/oriProject.png", _ShaderFileDir);
//    flipImageData(_HeadTextureData, TextureWidth, TextureHeight);
//    saveImageDataToFile(_HeadTextureData, TextureWidth, TextureHeight, tmpPath);
//    flipImageData(_HeadTextureData, TextureWidth, TextureHeight);

    //create gradient image (input image size)
//    subStartTime = clock();
//    unsigned char * gradientImg = new unsigned char[imageWidth * imageHeight * 4];
//    createGradientImage(headVertexArray, head_MVP, headVertexNum, gradientImg, imageWidth, imageHeight);
//    flipImageData(gradientImg, imageWidth, imageHeight);    
//    subEndTime = clock();
//    LOGI("        grad Img: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
//
//    subStartTime = clock();
//    //create gradient texture (small texture)
//    unsigned char * gradientTextureData = new unsigned char[PossionTexWidth * PossionTexHeight * 4];
//    projectImageToTexture(headVertexArray, head_MVP, headVertexNum, gradientImg, imageWidth, imageHeight, gradientTextureData, PossionTexWidth, PossionTexHeight);
//    delete [] gradientImg;
//    subEndTime = clock();
//    LOGI("        grad Tex: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));

    subStartTime = clock();
    //load gradient texture data
    char gradientTextureFile[500];
    sprintf(gradientTextureFile, "%s/gradientTex_%d.png", _ShaderFileDir, PossionTexWidth);
    unsigned char * gradientTextureData = getImageDataAndInfoForImagePath(gradientTextureFile, NULL, NULL);
    flipImageData(gradientTextureData, PossionTexWidth, PossionTexHeight);
    subEndTime = clock();
    LOGI("        load grad Tex: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    
    subStartTime = clock();
    char headFile[500];
    sprintf(headFile, "%s/head_%d.png", _ShaderFileDir, PossionTexWidth);
    //sprintf(headFile, "%s/head_avg.png", _ShaderFileDir);

    unsigned char * otherHeadTextureData = getImageDataAndInfoForImagePath(headFile, NULL, NULL);
    flipImageData(otherHeadTextureData, PossionTexWidth, PossionTexHeight);
    subEndTime = clock();
    LOGI("        load head lap: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    
    //do posssion on small texture
    subStartTime = clock();
    unsigned char * scaledImageData = new unsigned char[PossionTexWidth * PossionTexHeight * 4];
    if(!scaledImageData)
    {
        LOGI("failed to alloc memory");
    }
    scaleImageData(_HeadTextureData, TextureWidth, TextureHeight, scaledImageData, PossionTexWidth, PossionTexHeight);
    subEndTime = clock();
    LOGI("        downsample: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    
    subStartTime = clock();
    doPossionProcessingWithGradientTexture(scaledImageData, gradientTextureData, otherHeadTextureData, PossionTexWidth, PossionTexHeight, _Laplacian);
    //doPossionProcessingWithGradientTextureUsingEigen(scaledImageData, gradientTextureData, otherHeadTextureData, PossionTexWidth, PossionTexHeight, _Laplacian);
    subEndTime = clock();
    LOGI("        possion: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    
    subStartTime = clock();
    unsigned char * upsampledImageData = new unsigned char[TextureWidth * TextureHeight * 4];
    if(!upsampledImageData)
    {
        LOGI("failed to alloc memory");
    }
    scaleImageData(scaledImageData, PossionTexWidth, PossionTexHeight, upsampledImageData, TextureWidth, TextureHeight);
    subEndTime = clock();
    delete [] gradientTextureData;
    delete [] scaledImageData;
    delete [] otherHeadTextureData;
    LOGI("        upsample: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));

    subStartTime = clock();
    //composition (fill big origin texture)
    doCompositionWithUpsampledImageAndMaskImage(upsampledImageData, _HeadTextureData, TextureWidth, TextureHeight);
    delete [] upsampledImageData;
    flipImageData(_HeadTextureData, TextureWidth, TextureHeight);
    subEndTime = clock();
    LOGI("        doComposition1: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    
    ///////////////////////////////
    subStartTime = clock();
    //render back to image (input image size)
    unsigned char * noiseImage = new unsigned char[imageWidth * imageHeight * 4];
    if(!noiseImage)
    {
        LOGI("failed to alloc memory");
    }
    projectTextureToImage(headVertexArray, head_MVP, headVertexNum, _HeadTextureData, TextureWidth, TextureHeight, noiseImage, imageWidth, imageHeight);
    flipImageData(noiseImage, imageWidth, imageHeight);
    subEndTime = clock();
    LOGI("        rend back: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    
    subStartTime = clock();
    //add noise
    addNoise(partialFaceImageData, inputImageData, noiseImage, imageWidth, imageHeight, val);
    subEndTime = clock();
    LOGI("        add noise: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    
    subStartTime = clock();
    //render to a texture with gradient and noise (big texture)
    unsigned char * noiseTexture = new unsigned char[TextureWidth * TextureHeight * 4];
    if(!noiseTexture)
    {
        LOGI("failed to alloc memory");
    }
    projectImageToTexture(headVertexArray, head_MVP, headVertexNum, noiseImage, imageWidth, imageHeight, noiseTexture, TextureWidth, TextureHeight);
    flipImageData(noiseTexture, TextureWidth, TextureHeight);
    delete [] noiseImage;
    subEndTime = clock();
    LOGI("        proj noise tex: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    
    subStartTime = clock();
    //composition (replace with noise part)
    doCompositionWithNoiseImage(noiseTexture, _HeadTextureData, TextureWidth, TextureHeight);
    delete [] noiseTexture;
    subEndTime = clock();
    LOGI("        doComposition2: %f\n", ((float)(subEndTime - subStartTime) / CLOCKS_PER_SEC));
    /////////////////////////////////

    glDeleteBuffers(1, &headVertexBuffer);
    glDeleteVertexArraysOES(1, &headVertexArray);
    delete headModel;
    
    LOGI("Pass 4");
}

void TexGenerator::projectImageToTexture(GLuint headVertexArray, const glm::mat4 & mvp, int headFaceNum, const unsigned char * imageData, int imageWidth, int imageHeight, unsigned char * textureData, GLint textureWidth, GLint textureHeight)
{
    
    LOGI("Img to Tex 0");
    
    GLuint imgTex;
    loadTextureFromImageData(imageData, imageWidth, imageHeight, &imgTex);
    
    LOGI("Img to Tex 1");
    
    GLuint frameBuffer;
    GLuint targetTexture;
    GLuint depthBuffer;
    
    glGenTextures(1, &targetTexture);
    glBindTexture(GL_TEXTURE_2D , targetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    
    LOGI("Img to Tex 2");
    
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth, textureHeight);
    
    
    glGenFramebuffers(1, &frameBuffer);
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    LOGI("Img to Tex 3");
    
    //render to texture
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);
    if(!checkCurrenFrameBufferStatus())
        return;
    
    LOGI("Img to Tex 4");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, imgTex);
    
    glUseProgram(_program4FaceTex);
    glUniformMatrix4fv(uniforms4FaceTex[FACETEX_UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, glm::value_ptr(mvp));
    glUniform1i(uniforms4FaceTex[FACETEX_UNIFORM_SAMPLERFACE], 0);
    
    glViewport(0, 0, textureWidth, textureHeight);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    LOGI("Img to Tex 5");
    
    glBindVertexArrayOES(headVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, headFaceNum);
    
    LOGI("Img to Tex 6");
    
    glReadPixels(0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    
    LOGI("Img to Tex 7");
    
    resetAllBinds();
    glUseProgram(0);
    
    glDeleteTextures(1, &imgTex);
    glDeleteTextures(1, &targetTexture);
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteFramebuffers(1, &frameBuffer);
    
    LOGI("Img to Tex 8");
}

void TexGenerator::projectTextureToImage(GLuint headVertexArray, const glm::mat4 & mvp, int headFaceNum, const unsigned char * textureData, GLint textureWidth, GLint textureHeight, unsigned char * imageData, int imageWidth, int imageHeight)
{
    LOGI("Tex to Img 1");
    
    GLuint texTex;
    loadTextureFromImageData(textureData, textureWidth, textureHeight, &texTex);
    
    LOGI("Tex to Img here");
    
    GLuint frameBuffer;
    GLuint targetTexture;
    GLuint depthBuffer;
    
    glGenTextures(1, &targetTexture);
    glBindTexture(GL_TEXTURE_2D , targetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    
    LOGI("Tex to Img 2");
    
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, imageWidth, imageHeight);
    
    LOGI("Tex to Img 3");
    
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    //render to texture
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);
    if(!checkCurrenFrameBufferStatus())
        return;
    
    LOGI("Tex to Img 4");
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texTex);
    
    glUseProgram(_program4FaceImage);
    glUniformMatrix4fv(uniforms4FaceImage[FACEIMAGE_UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, glm::value_ptr(mvp));
    glUniform1i(uniforms4FaceImage[FACEIMAGE_UNIFORM_SAMPLERFACE], 1);
    
    glViewport(0, 0, imageWidth, imageHeight);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    LOGI("Tex to Img 5");
    
    glBindVertexArrayOES(headVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, headFaceNum);
    
    LOGI("Tex to Img 6");
    
    glReadPixels(0, 0, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    
    LOGI("Tex to Img 7");
    
    resetAllBinds();
    glUseProgram(0);
    
    glDeleteTextures(1, &texTex);
    glDeleteTextures(1, &targetTexture);
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteFramebuffers(1, &frameBuffer);
    
    LOGI("Tex to Img 8");
}

void TexGenerator::createGradientImage(GLuint headVertexArray, const glm::mat4 & mvp, int headFaceNum, unsigned char * imageData, int imageWidth, int imageHeight)
{
    GLuint frameBuffer;
    GLuint targetTexture;
    GLuint depthBuffer;
    
    glGenTextures(1, &targetTexture);
    glBindTexture(GL_TEXTURE_2D , targetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, imageWidth, imageHeight);
    
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    //render to texture
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);
    if(!checkCurrenFrameBufferStatus())
        return;
    
    glUseProgram(_program4FaceGradient);
    glUniformMatrix4fv(uniforms4FaceImage[FACEIMAGE_UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, glm::value_ptr(mvp));
    
    glViewport(0, 0, imageWidth, imageHeight);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindVertexArrayOES(headVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, headFaceNum);
    
    glReadPixels(0, 0, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    
    resetAllBinds();
    glUseProgram(0);
    
    glDeleteTextures(1, &targetTexture);
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteFramebuffers(1, &frameBuffer);
}

void TexGenerator::doPossionProcessingWithGradientTexture(unsigned char * imageData, const unsigned char * gradientTextureData, const unsigned char * otherImageData, int width, int height, Laplacian * laplacian)
{
    //CFAbsoluteTime startTime, endTime;
    //startTime = CFAbsoluteTimeGetCurrent();
    
    const int size = width * height;
    const int totalImageBytes = size * PixelChannels;
    const int xOffset[4] = {0, -1, +1, 0};
    const int yOffset[4] = {-1, 0, 0, +1};
    int indexOffset[4];
    int anchorPointOffset[4];
    
    indexOffset[0] = -width;
    indexOffset[1] = -1;
    indexOffset[2] = +1;
    indexOffset[3] = +width;
    
    anchorPointOffset[0] = -width * PixelChannels;
    anchorPointOffset[1] = -PixelChannels;
    anchorPointOffset[2] = PixelChannels;
    anchorPointOffset[3] = width * PixelChannels;
    
    //count poisson pixel num
    int possionNum = 0;
    for(int i=0; i<totalImageBytes; i+=PixelChannels)
        if(imageData[i+3] < 0xff && _MaskImageData[i] > 0x80)
            possionNum++;
    
    if(possionNum == 0)
    {
        LOGI("No need to perform head texture Poisson processing.\n");
        return;
    }
    
    // create poisson needed pixel maps between poisson index and image index
	int * mapPoisson2Tex = (int *)malloc(possionNum * sizeof(int));
	int * mapTex2Poisson = (int *)malloc(size * sizeof(int));
	int pIndex = 0;
    int anchorPoint = 0;
	for(int i=0; i<size; i++)
    {
		if(imageData[anchorPoint+3] < 0xff && _MaskImageData[anchorPoint] > 0x80)
		{
			mapTex2Poisson[i]= pIndex;
			mapPoisson2Tex[pIndex++] = i;
        }
        
        anchorPoint += PixelChannels;
    }
    LOGI("            F\n");
    LOGI("            possion num = %d\n", possionNum);
    //endTime = CFAbsoluteTimeGetCurrent();
    
    //create and init a, x and b
    //startTime = CFAbsoluteTimeGetCurrent();
    int selfIndex, selfPosX, selfPosY;
    int neiborCount = 0;
    int neiborNeedPossionCount = 0;
    int noneNeiborSumR, noneNeiborSumG, noneNeiborSumB;
    int neiborSum;
    int neiborPosX, neiborPosY;
    bool isNeiborAvalable[4];
    int neiborAnchorPoint = 0;
    anchorPoint =0;
    
    //calculate laplacian from gradientTextureData
    float * laplacianData = (float *)malloc(possionNum * sizeof(float));
    for(int i=0; i<possionNum; i++)
    {
        selfIndex = mapPoisson2Tex[i];
        selfPosX = selfIndex % width;
        selfPosY = selfIndex / width;
        anchorPoint = selfIndex * PixelChannels;
        
        if(gradientTextureData[anchorPoint + 3] < 0x80)
            laplacianData[i] = 0.0f;
        else
        {
            neiborCount = 0;
            neiborSum = 0;
            for(int j=0; j<4; j++)
            {
                neiborPosX = selfPosX + xOffset[j];
                neiborPosY = selfPosY + yOffset[j];
                neiborAnchorPoint = anchorPoint + anchorPointOffset[j];
                if( (neiborPosX >= 0 && neiborPosX < width) && (neiborPosY >= 0 && neiborPosY < height) && (gradientTextureData[neiborAnchorPoint+3] >= 0x80) && _MaskImageData[anchorPoint] > 0x80)
                {
                    neiborCount++;
                    neiborSum += gradientTextureData[neiborAnchorPoint];
                }
            }
            if(neiborCount <= 0)
                laplacianData[i] = 0.0f;
            else
                laplacianData[i] = (float)((gradientTextureData[anchorPoint] * neiborCount) - neiborSum);
        }
    }
    
    float * otherImagelaplacianDataR = (float *)malloc(possionNum * sizeof(float));
    float * otherImagelaplacianDataG = (float *)malloc(possionNum * sizeof(float));
    float * otherImagelaplacianDataB = (float *)malloc(possionNum * sizeof(float));
    int neiborSumR, neiborSumG, neiborSumB;
    for(int i=0; i<possionNum; i++)
    {
        selfIndex = mapPoisson2Tex[i];
        selfPosX = selfIndex % width;
        selfPosY = selfIndex / width;
        anchorPoint = selfIndex * PixelChannels;
        
        if(otherImageData[anchorPoint + 3] < 0x80)
        {
            otherImagelaplacianDataR[i] = 0.0f;
            otherImagelaplacianDataG[i] = 0.0f;
            otherImagelaplacianDataB[i] = 0.0f;
        }
        else
        {
            neiborCount = 0;
            //neiborSum = 0;
            neiborSumR = neiborSumG = neiborSumB = 0;
            for(int j=0; j<4; j++)
            {
                neiborPosX = selfPosX + xOffset[j];
                neiborPosY = selfPosY + yOffset[j];
                neiborAnchorPoint = anchorPoint + anchorPointOffset[j];
                if( (neiborPosX >= 0 && neiborPosX < width) && (neiborPosY >= 0 && neiborPosY < height) && (otherImageData[neiborAnchorPoint+3] >= 0x80) && _MaskImageData[anchorPoint] > 0x80)
                {
                    neiborCount++;
                    //neiborSum += otherImagelaplacianData[neiborAnchorPoint];
                    neiborSumR += otherImageData[neiborAnchorPoint];
                    neiborSumG += otherImageData[neiborAnchorPoint + 1];
                    neiborSumB += otherImageData[neiborAnchorPoint + 2];
                }
            }
            if(neiborCount <= 0)
            {
                otherImagelaplacianDataR[i] = 0.0f;
                otherImagelaplacianDataG[i] = 0.0f;
                otherImagelaplacianDataB[i] = 0.0f;
            }
            else
            {
                otherImagelaplacianDataR[i] = (float)((otherImageData[anchorPoint] * neiborCount) - neiborSumR);
                otherImagelaplacianDataG[i] = (float)((otherImageData[anchorPoint + 1] * neiborCount) - neiborSumG);
                otherImagelaplacianDataB[i] = (float)((otherImageData[anchorPoint + 2] * neiborCount) - neiborSumB);
            }
        }
    }
    
    //init A and B
//    const float * channelR = laplacian->getRedChannel();
//    const float * channelG = laplacian->getGreenChannel();
//    const float * channelB = laplacian->getBlueChannel();
    
    int * Ap = (int *)malloc((possionNum + 1) * sizeof(int));
    int * Ai = (int *)malloc((possionNum * 5) * sizeof(int));
    double * Ax = (double *)malloc((possionNum * 5) * sizeof(double));
    double * bR = (double *)malloc(possionNum * sizeof(double));
    double * bG = (double *)malloc(possionNum * sizeof(double));
    double * bB = (double *)malloc(possionNum * sizeof(double));
    double * xR = (double *)malloc(possionNum * sizeof(double));
    double * xG = (double *)malloc(possionNum * sizeof(double));
    double * xB = (double *)malloc(possionNum * sizeof(double));
    
    int AIndex = 0;
    Ap[0] = 0;
    anchorPoint =0;
    for(int i=0; i<possionNum; i++)
    {
        selfIndex = mapPoisson2Tex[i];
        selfPosX = selfIndex % width;
        selfPosY = selfIndex / width;
        anchorPoint = selfIndex * PixelChannels;
        // for point (selfPosX, selfPosY)
        // count neibor
        neiborCount = 0;
        neiborNeedPossionCount = 0;
        noneNeiborSumR = noneNeiborSumG = noneNeiborSumB = 0;
        for(int j=0; j<4; j++)
        {
            neiborPosX = selfPosX + xOffset[j];
            neiborPosY = selfPosY + yOffset[j];
            isNeiborAvalable[j] = false;
            
            if( (neiborPosX >= 0 && neiborPosX < width) && (neiborPosY >= 0 && neiborPosY < height) )
            {
                neiborCount++;
                
                neiborAnchorPoint = anchorPoint + anchorPointOffset[j];
                // updatad 将mask 黑色区域的点（imageData rgb（0，0，0））加入已知点
                if(imageData[neiborAnchorPoint + 3] == 0xff || _MaskImageData[neiborAnchorPoint] <0x80) 
                {
                    noneNeiborSumR += imageData[neiborAnchorPoint];
                    noneNeiborSumG += imageData[neiborAnchorPoint + 1];
                    noneNeiborSumB += imageData[neiborAnchorPoint + 2];
                }
                else
                {
                    isNeiborAvalable[j] = true;
                    neiborNeedPossionCount++;
                }
            }
        }
        Ap[i + 1] = Ap[i] + neiborNeedPossionCount + 1;
        
        // fill matrix
        for(int j=0; j<2; j++)
        {
            if(isNeiborAvalable[j])
            {
                Ai[AIndex] = mapTex2Poisson[selfIndex + indexOffset[j]];
                Ax[AIndex++] = -1.0;
            }
            
        }
        
        Ai[AIndex] = i;
        Ax[AIndex++] = (double)neiborCount;
        
        for(int j=2; j<4; j++)
        {
            if(isNeiborAvalable[j])
            {
                Ai[AIndex] = mapTex2Poisson[selfIndex + indexOffset[j]];
                Ax[AIndex++] = -1.0;
            }
        }
        
//        bR[i] = laplacianData[i] + channelR[selfIndex] + noneNeiborSumR;
//        bG[i] = laplacianData[i] + channelG[selfIndex] + noneNeiborSumG;
//        bB[i] = laplacianData[i] + channelB[selfIndex] + noneNeiborSumB;
//        bR[i] = channelR[selfIndex] + noneNeiborSumR;
//        bG[i] = channelG[selfIndex] + noneNeiborSumG;
//        bB[i] = channelB[selfIndex] + noneNeiborSumB;
//        bR[i] = laplacianData[i] + noneNeiborSumR;
//        bG[i] = laplacianData[i] + noneNeiborSumG;
//        bB[i] = laplacianData[i] + noneNeiborSumB;
//        bR[i] = otherImagelaplacianDataR[i] + noneNeiborSumR;
//        bG[i] = otherImagelaplacianDataG[i] + noneNeiborSumG;
//        bB[i] = otherImagelaplacianDataB[i] + noneNeiborSumB;
        bR[i] = laplacianData[i] + otherImagelaplacianDataR[i] + noneNeiborSumR;
        bG[i] = laplacianData[i] + otherImagelaplacianDataG[i] + noneNeiborSumG;
        bB[i] = laplacianData[i] + otherImagelaplacianDataB[i] + noneNeiborSumB;
    }
    free(laplacianData);
    free(otherImagelaplacianDataR);
    free(otherImagelaplacianDataG);
    free(otherImagelaplacianDataB);
    //endTime = CFAbsoluteTimeGetCurrent();
    
    
    //perform lv factor
    //startTime = CFAbsoluteTimeGetCurrent();
    double Info [UMFPACK_INFO], Control [UMFPACK_CONTROL];
    int status;
    void * Symbolic, *Numeric;
    umfpack_di_defaults (Control) ;
    Control [UMFPACK_PRL] = 6 ;
    Control [UMFPACK_PRL] = 5 ;
    status = umfpack_di_symbolic(possionNum, possionNum, Ap, Ai, Ax, &Symbolic, Control, Info);
    if (status < 0)
    {
        umfpack_di_report_info (Control, Info) ;
        umfpack_di_report_status (Control, status) ;
        LOGI("umfpack_di_symbolic failed\n");
    }
    status = umfpack_di_numeric(Ap, Ai, Ax, Symbolic, &Numeric, Control, Info);
    if (status < 0)
    {
        //umfpack_di_report_info (Control, Info) ;
        //umfpack_di_report_status (Control, status) ;
        //LOGI("umfpack_di_numeric failed\n") ;
    }
    umfpack_di_free_symbolic(&Symbolic);
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        lu factor: %lf s\n", endTime - startTime);
    
    
    //evaluation
    //startTime = CFAbsoluteTimeGetCurrent();
    status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, xR, bR, Numeric, Control, Info);
    if (status < 0)
    {
        LOGI("umfpack_di_solve Red failed\n") ;
    }
    
    status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, xG, bG, Numeric, Control, Info);
    if (status < 0)
    {
        LOGI("umfpack_di_solve Green failed\n") ;
    }
    
    status = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, xB, bB, Numeric, Control, Info);
    if (status < 0)
    {
        LOGI("umfpack_di_solve Blue failed\n") ;
    }
    
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        cal channels: %lf\n", endTime - startTime);
    
    umfpack_di_free_numeric(&Numeric);
    
    //write back to imageData
    //startTime = CFAbsoluteTimeGetCurrent();
    
    int R, G, B;
    for(int i=0; i<possionNum; i++)
    {
        R = (int)xR[i];
        G = (int)xG[i];
        B = (int)xB[i];
        
        R = Min(Max(R, 0), 255);
        G = Min(Max(G, 0), 255);
        B = Min(Max(B, 0), 255);
        
        anchorPoint = mapPoisson2Tex[i] * PixelChannels;
        
        imageData[anchorPoint] = R;
        imageData[anchorPoint + 1] = G;
        imageData[anchorPoint + 2] = B;
        imageData[anchorPoint + 3] = 0xff;
    }
    
    //release memory
    free(mapTex2Poisson);
    free(mapPoisson2Tex);
    free(Ap);
    free(Ai);
    free(Ax);
    free(bR);
    free(bG);
    free(bB);
    free(xR);
    free(xG);
    free(xB);
    
    //endTime = CFAbsoluteTimeGetCurrent();
}

/*
void TexGenerator::doPossionProcessingWithGradientTextureUsingEigen(unsigned char *imageData, const unsigned char *gradientTextureData, const unsigned char *otherImageData, int width, int height, Laplacian *laplacian)
{
    //CFAbsoluteTime startTime, endTime;
    //startTime = CFAbsoluteTimeGetCurrent();
    
    const int size = width * height;
    const int totalImageBytes = size * PixelChannels;
    const int xOffset[4] = {0, -1, +1, 0};
    const int yOffset[4] = {-1, 0, 0, +1};
    int indexOffset[4];
    int anchorPointOffset[4];
    
    indexOffset[0] = -width;
    indexOffset[1] = -1;
    indexOffset[2] = +1;
    indexOffset[3] = +width;
    
    anchorPointOffset[0] = -width * PixelChannels;
    anchorPointOffset[1] = -PixelChannels;
    anchorPointOffset[2] = PixelChannels;
    anchorPointOffset[3] = width * PixelChannels;
    
    //count poisson pixel num
    int possionNum = 0;
    for(int i=0; i<totalImageBytes; i+=PixelChannels)
        if(imageData[i+3] < 0xff && _MaskImageData[i] > 0x80)
            possionNum++;
    
    if(possionNum == 0)
    {
        LOGI("No need to perform head texture Poisson processing.\n");
        return;
    }
    
    // create poisson needed pixel maps between poisson index and image index
	int * mapPoisson2Tex = (int *)malloc(possionNum * sizeof(int));
	int * mapTex2Poisson = (int *)malloc(size * sizeof(int));
	int pIndex = 0;
    int anchorPoint = 0;
	for(int i=0; i<size; i++)
    {
		if(imageData[anchorPoint+3] < 0xff && _MaskImageData[anchorPoint] > 0x80)
		{
			mapTex2Poisson[i]= pIndex;
			mapPoisson2Tex[pIndex++] = i;
        }
        
        anchorPoint += PixelChannels;
    }
    LOGI("            F\n");
    LOGI("            possion num = %d\n", possionNum);
    //endTime = CFAbsoluteTimeGetCurrent();
    
    //create and init a, x and b
    //startTime = CFAbsoluteTimeGetCurrent();
    
    int selfIndex, selfPosX, selfPosY;
    int neiborCount = 0;
    int neiborNeedPossionCount = 0;
    int noneNeiborSumR, noneNeiborSumG, noneNeiborSumB;
    int neiborSum;
    int neiborPosX, neiborPosY, neiborIndex;
    bool isNeiborAvalable[4];
    int neiborAnchorPoint = 0;
    anchorPoint =0;
    
    //calculate laplacian from gradientTextureData
    float * laplacianData = (float *)malloc(possionNum * sizeof(float));
    for(int i=0; i<possionNum; i++)
    {
        selfIndex = mapPoisson2Tex[i];
        selfPosX = selfIndex % width;
        selfPosY = selfIndex / width;
        anchorPoint = selfIndex * PixelChannels;
        
        if(gradientTextureData[anchorPoint + 3] < 0x80)
            laplacianData[i] = 0.0f;
        else
        {
            neiborCount = 0;
            neiborSum = 0;
            for(int j=0; j<4; j++)
            {
                neiborPosX = selfPosX + xOffset[j];
                neiborPosY = selfPosY + yOffset[j];
                neiborAnchorPoint = anchorPoint + anchorPointOffset[j];
                if( (neiborPosX >= 0 && neiborPosX < width) && (neiborPosY >= 0 && neiborPosY < height) && (gradientTextureData[neiborAnchorPoint+3] >= 0x80) && _MaskImageData[anchorPoint] > 0x80)
                {
                    neiborCount++;
                    neiborSum += gradientTextureData[neiborAnchorPoint];
                }
            }
            if(neiborCount <= 0)
                laplacianData[i] = 0.0f;
            else
                laplacianData[i] = (float)((gradientTextureData[anchorPoint] * neiborCount) - neiborSum);
        }
    }
    
    float * otherImagelaplacianDataR = (float *)malloc(possionNum * sizeof(float));
    float * otherImagelaplacianDataG = (float *)malloc(possionNum * sizeof(float));
    float * otherImagelaplacianDataB = (float *)malloc(possionNum * sizeof(float));
    int neiborSumR, neiborSumG, neiborSumB;
    for(int i=0; i<possionNum; i++)
    {
        selfIndex = mapPoisson2Tex[i];
        selfPosX = selfIndex % width;
        selfPosY = selfIndex / width;
        anchorPoint = selfIndex * PixelChannels;
        
        if(otherImageData[anchorPoint + 3] < 0x80)
        {
            otherImagelaplacianDataR[i] = 0.0f;
            otherImagelaplacianDataG[i] = 0.0f;
            otherImagelaplacianDataB[i] = 0.0f;
        }
        else
        {
            neiborCount = 0;
            //neiborSum = 0;
            neiborSumR = neiborSumG = neiborSumB = 0;
            for(int j=0; j<4; j++)
            {
                neiborPosX = selfPosX + xOffset[j];
                neiborPosY = selfPosY + yOffset[j];
                neiborAnchorPoint = anchorPoint + anchorPointOffset[j];
                if( (neiborPosX >= 0 && neiborPosX < width) && (neiborPosY >= 0 && neiborPosY < height) && (otherImageData[neiborAnchorPoint+3] >= 0x80) && _MaskImageData[anchorPoint] > 0x80)
                {
                    neiborCount++;
                    //neiborSum += otherImagelaplacianData[neiborAnchorPoint];
                    neiborSumR += otherImageData[neiborAnchorPoint];
                    neiborSumG += otherImageData[neiborAnchorPoint + 1];
                    neiborSumB += otherImageData[neiborAnchorPoint + 2];
                }
            }
            if(neiborCount <= 0)
            {
                otherImagelaplacianDataR[i] = 0.0f;
                otherImagelaplacianDataG[i] = 0.0f;
                otherImagelaplacianDataB[i] = 0.0f;
            }
            else
            {
                otherImagelaplacianDataR[i] = (float)((otherImageData[anchorPoint] * neiborCount) - neiborSumR);
                otherImagelaplacianDataG[i] = (float)((otherImageData[anchorPoint + 1] * neiborCount) - neiborSumG);
                otherImagelaplacianDataB[i] = (float)((otherImageData[anchorPoint + 2] * neiborCount) - neiborSumB);
            }
        }
    }
    
    //init A and B
    //    const float * channelR = laplacian->getRedChannel();
    //    const float * channelG = laplacian->getGreenChannel();
    //    const float * channelB = laplacian->getBlueChannel();
    
    Eigen::SparseMatrix<float> A(possionNum, possionNum);
    A.reserve(Eigen::VectorXi::Constant(possionNum, 5));
    Eigen::VectorXf bR(possionNum);
    Eigen::VectorXf bG(possionNum);
    Eigen::VectorXf bB(possionNum);
    Eigen::VectorXf xR(possionNum);
    Eigen::VectorXf xG(possionNum);
    Eigen::VectorXf xB(possionNum);
    
    anchorPoint =0;
    for(int i=0; i<possionNum; i++)
    {
        selfIndex = mapPoisson2Tex[i];
        selfPosX = selfIndex % width;
        selfPosY = selfIndex / width;
        anchorPoint = selfIndex * PixelChannels;
        // for point (selfPosX, selfPosY)
        // count neibor
        neiborCount = 0;
        neiborNeedPossionCount = 0;
        noneNeiborSumR = noneNeiborSumG = noneNeiborSumB = 0;
        for(int j=0; j<4; j++)
        {
            neiborPosX = selfPosX + xOffset[j];
            neiborPosY = selfPosY + yOffset[j];
            isNeiborAvalable[j] = false;
            
            if( (neiborPosX >= 0 && neiborPosX < width) && (neiborPosY >= 0 && neiborPosY < height) )
            {
                neiborCount++;
                
                neiborAnchorPoint = anchorPoint + anchorPointOffset[j];
                // updatad 将mask 黑色区域的点（imageData rgb（0，0，0））加入已知点
                if(imageData[neiborAnchorPoint + 3] == 0xff || _MaskImageData[neiborAnchorPoint] <0x80)
                {
                    noneNeiborSumR += imageData[neiborAnchorPoint];
                    noneNeiborSumG += imageData[neiborAnchorPoint + 1];
                    noneNeiborSumB += imageData[neiborAnchorPoint + 2];
                }
                else
                {
                    isNeiborAvalable[j] = true;
                    neiborNeedPossionCount++;
                }
            }
        }
        
        // fill matrix
        for(int j=0; j<2; j++)
        {
            if(isNeiborAvalable[j])
            {
                neiborIndex = selfIndex + indexOffset[j];
                A.insert(i, mapTex2Poisson[neiborIndex]) = -1.0f;
            }
        }
        
        A.insert(i, i) = (float)neiborCount;
        
        for(int j=2; j<4; j++)
        {
            if(isNeiborAvalable[j])
            {
                neiborIndex = selfIndex + indexOffset[j];
                A.insert(i, mapTex2Poisson[neiborIndex]) = -1.0f;
            }
        }
        
        bR(i) = laplacianData[i] + otherImagelaplacianDataR[i] + noneNeiborSumR;
        bG(i) = laplacianData[i] + otherImagelaplacianDataG[i] + noneNeiborSumG;
        bB(i) = laplacianData[i] + otherImagelaplacianDataB[i] + noneNeiborSumB;
    }
    A.makeCompressed();
    free(laplacianData);
    free(otherImagelaplacianDataR);
    free(otherImagelaplacianDataG);
    free(otherImagelaplacianDataB);
    //endTime = CFAbsoluteTimeGetCurrent();

    //perform lu factor
    //startTime = CFAbsoluteTimeGetCurrent();
    Eigen::SparseLU< Eigen::SparseMatrix<float> > solver;
    solver.compute(A);
    if(solver.info() != Eigen::Success)
    {
        LOGI("Failed to perform lu factor.\n");
        return;
    }
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        lu factor: %lf s\n", endTime - startTime);
    
    
    //evaluation
    //startTime = CFAbsoluteTimeGetCurrent();
    xR = solver.solve(bR);
    if(solver.info() != Eigen::Success)
    {
        LOGI("Failed to evalute red channel.\n");
        return;
    }
    
    xG = solver.solve(bG);
    if(solver.info() != Eigen::Success)
    {
        LOGI("Failed to evalute red channel.\n");
        return;
    }
    
    xB = solver.solve(bB);
    if(solver.info() != Eigen::Success)
    {
        LOGI("Failed to evalute red channel.\n");
        return;
    }
    
    //endTime = CFAbsoluteTimeGetCurrent();
    //LOGI("        cal channels: %lf\n", endTime - startTime);
    
    //write back to imageData
    //startTime = CFAbsoluteTimeGetCurrent();
    int R, G, B;
    for(int i=0; i<possionNum; i++)
    {
        R = (int)xR(i);
        G = (int)xG(i);
        B = (int)xB(i);
        
        R = Min(Max(R, 0), 255);
        G = Min(Max(G, 0), 255);
        B = Min(Max(B, 0), 255);
        
        anchorPoint = mapPoisson2Tex[i] * PixelChannels;
        
        imageData[anchorPoint] = R;
        imageData[anchorPoint + 1] = G;
        imageData[anchorPoint + 2] = B;
        imageData[anchorPoint + 3] = 0xff;
    }
    
    //release memory
    free(mapTex2Poisson);
    free(mapPoisson2Tex);
    
    //endTime = CFAbsoluteTimeGetCurrent();
}
*/

void TexGenerator::addNoise(const unsigned char * partialFaceImageData, const unsigned char * maskImageData, unsigned char * targetImageData, int width, int height, float pmaxDiff)
{
//    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask ,YES);
//    NSString *documentsDirectory = paths[0];
//    NSString * drawingsDirectory = [documentsDirectory stringByAppendingPathComponent:@"work"];
    
    // generate detail image
    cv::Mat patchData = cv::Mat::zeros(height, width, CV_8UC4);
    int arrayIndex = 0;
    for(int i=0; i<height; i++)
        for(int j=0; j<width; j++)
        {
            for(int k=0; k<4; k++)
                patchData.at<cv::Vec4b>(i, j)[k] = partialFaceImageData[arrayIndex++];
        }
    cv::Mat blurData = cv::Mat::zeros(height, width, CV_8UC4);
	cv::GaussianBlur(patchData, blurData, cv::Size(15, 15), 0);
	cv::Mat diffData = cv::Mat(blurData.rows, blurData.cols, CV_32FC3, cv::Scalar(0.f, 0.f, 0.f));
	//cv::Mat rateData = cv::Mat(blurData.rows, blurData.cols, CV_32FC3, cv::Scalar(0.f, 0.f, 0.f));
	for (int h = 0; h < diffData.rows; h++)
		for (int w = 0; w < diffData.cols; w++)
			for (int d = 0; d < 3; d++){
				//rateData.at<cv::Vec3f>(h, w)[d] = patchData.at<cv::Vec4b>(h, w)[d] / (blurData.at<cv::Vec4b>(h, w)[d] + 0.001f);
				diffData.at<cv::Vec3f>(h, w)[d] = patchData.at<cv::Vec4b>(h, w)[d] - blurData.at<cv::Vec4b>(h, w)[d];
			}
    blurData.release();
    
    // valid patch
    //float maxDiff = 2000.f;
    float maxDiff = pmaxDiff;
	int patchSize = 4;
	vector<cv::Vec2i> validPatches;
    const int upHeight = height - patchSize;
    const int upWidth = width - patchSize;
    for(int cnt=1; cnt<=3; cnt++)
    {        
        for(int h = patchSize; h < upHeight; h++)
            for(int w = patchSize; w < upWidth; w++)
            {
                if(patchData.at<cv::Vec4b>(h, w)[3] == 255)
                {
                    bool valid = true;
                    float totalDiff = 0.f;
                    for(int hn = -patchSize; hn <= patchSize; hn++)
                    {
                        for(int wn = -patchSize; wn <= patchSize; wn++)
                        {
                            if(patchData.at<cv::Vec4b>(h + hn, w + wn)[3] < 255)
                            {
                                valid = false;
                                break;
                            }
                            for(int d = 0; d < 3; d++)
                                totalDiff += fabs(diffData.at<cv::Vec3f>(h + hn, w + wn)[d]);
                        }
                    }
                    if(valid && totalDiff < maxDiff)
                        validPatches.push_back(cv::Vec2i(w, h));
                }
            }
        
        if(validPatches.size() > 0)
            break;
        else
            maxDiff *= 2;
    }
    patchData.release();
    
    LOGI("patch size: %d\n", (int)validPatches.size());
    
    if(validPatches.size() <= 0)
    {
        validPatches.clear();
        diffData.release();
        return;
    }

    //get boundbox of noise area
    int bbMax[2] = {-1, -1};
    int bbMin[2] = {100000, 100000};
    arrayIndex = 3;
    for(int i=0; i<height; i++)
        for(int j=0; j<width; j++)
        {
            //for pixel (j, i)
            if(maskImageData[arrayIndex] < 0x80 && targetImageData[arrayIndex] == 0xff)
            {
                if(bbMax[0] < j)
                    bbMax[0] = j;
                if(bbMin[0] > j)
                    bbMin[0] = j;
                if(bbMax[1] < i)
                    bbMax[1] = i;
                if(bbMin[1] > i)
                    bbMin[1] = i;
            }
            
            arrayIndex += PixelChannels;
        }
    // assign patch
    srand(19881101u);
    cv::Mat accData = cv::Mat(height, width, CV_32FC3, cv::Scalar(0.f, 0.f, 0.f));
	cv::Mat weiData = cv::Mat(height, width, CV_32FC1, 0.f);
    
    int widthLow = bbMin[0]+ patchSize;
    int widthUp = bbMax[0] - patchSize;
    int heightLow = bbMin[1] + patchSize;
    int heightUp = bbMax[1] - patchSize;
    for(int h=heightLow; h<heightUp; h+=patchSize)
        for(int w=widthLow; w<widthUp; w+=patchSize)
        {
            int patchI = rand() % validPatches.size();
            cv::Vec2i patchInd = validPatches[patchI];
            
            for(int hn = -patchSize; hn <= patchSize; hn++)
				for(int wn = -patchSize; wn <= patchSize; wn++)
                {
					float weight = Max(patchSize - sqrtf(hn * hn + wn * wn), 0.f) / (float)patchSize;
                    cv::Vec3f rate = diffData.at<cv::Vec3f>(patchInd[1] + hn, patchInd[0] + wn);
                    accData.at<cv::Vec3f>(h + hn, w + wn) += rate * weight;
                    weiData.at<float>(h + hn, w + wn) += weight;
				}
        }
    for(int h=heightLow; h<heightUp; h+=patchSize)
        for(int w=widthLow; w<widthUp; w+=patchSize)
        {
            accData.at<cv::Vec3f>(h, w) /= Max(weiData.at<float>(h, w), 1e-6f);
        }
    
    validPatches.clear();
    //rateData.release();
    weiData.release();
    diffData.release();
    
    // add detail
    for(int h=bbMin[1]; h<bbMax[1]; h++)
        for(int w=bbMin[0]; w<bbMax[0]; w++)
        {
            //for pixel (w, h)
            arrayIndex = (h * width + w) * PixelChannels;
            if(maskImageData[arrayIndex + 3] < 0x80 && targetImageData[arrayIndex + 3] == 0xff)
            {
                for(int d=0; d<2; d++)
                {
                    int val = (int)(targetImageData[arrayIndex + d] + 1.5 * accData.at<cv::Vec3f>(h, w)[d] + 0.5f);
                    targetImageData[arrayIndex + d] = Max(Min(val, 255), 0);
                }
            }
        }
    accData.release();
}

void TexGenerator::doCompositionWithUpsampledImageAndMaskImage(const unsigned char * upsampledImageData, unsigned char * targetImageData, int width, int height)
{
    const int byteSize = width * height * PixelChannels;
    for(int anchorPoint=0; anchorPoint<byteSize; anchorPoint += PixelChannels)
    {
        if(targetImageData[anchorPoint + 3] < 0xff)
        {
            targetImageData[anchorPoint] = upsampledImageData[anchorPoint];
            targetImageData[anchorPoint + 1] = upsampledImageData[anchorPoint + 1];
            targetImageData[anchorPoint + 2] = upsampledImageData[anchorPoint + 2];
            targetImageData[anchorPoint + 3] = upsampledImageData[anchorPoint + 3];
        }
    }
}

void TexGenerator::doCompositionWithNoiseImage(const unsigned char * noiseImageData, unsigned char * targetImageData, int width, int height)
{
    int totalNum = width * height * PixelChannels;
    for(int arrayIndex=0; arrayIndex<totalNum; arrayIndex += PixelChannels)
    {
        if(noiseImageData[arrayIndex + 3] == 0xff)
        {
            targetImageData[arrayIndex] = noiseImageData[arrayIndex];
            targetImageData[arrayIndex+1] = noiseImageData[arrayIndex+1];
            targetImageData[arrayIndex+2] = noiseImageData[arrayIndex+2];
            targetImageData[arrayIndex+3] = noiseImageData[arrayIndex+3];
        }
    }
}

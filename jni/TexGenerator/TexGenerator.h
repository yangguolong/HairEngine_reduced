//
//  TexGenerator.h
//  Demo
//
//  Created by yangguolong on 13-7-9.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef TexGenerator_H
#define TexGenerator_H

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "Laplacian.h"
#include "glm.hpp"
#include "TexHeadModel.h"

class TexGenerator {

public:
    TexGenerator(const char * texInitFileDir);
    ~TexGenerator();
    void generateFaceTextureFromImageWithGradient(const unsigned char * inputImageData, const unsigned char * partialFaceImageData, int imageWidth, int imageHeight, const char * objModelString, const char * transformDataFileString, const char * VFMapFileString, const char * outputImageFilePath, float pmaxDiff);
    void generateFaceTextureFromImage(const unsigned char * inputImageData, int imageWidth, int imageHeight, const char * objModelString, const char * transformDataFileString, const char * VFMapFileString, const char * outputImageFilePath);
    void generateBackgroundTextureFromImage(const unsigned char * inputImageData, int srcWidth, int srcHeight, const char * outputImageFilePath);
    
    const unsigned char * getHeadTextureInfo(int & width, int & height);
    const unsigned char * getBackgroundTextureInfo(int & width, int & height);
    
    unsigned char * _MaskData;
    unsigned char * _ImageData;
    int _ImageWidth;
    int _ImageHeight;
    
private:
    //init
    void initLaplacianData();
    void setupGL();
    void releaseGL();
    bool loadShaders();
    bool loadShader4FaceTex(const char * vertexShaderFilePath, const char * fragmentShaderFilePath);
    bool loadShader4FaceImage(const char * vertexShaderFilePath, const char * fragmentShaderFilePath);
    bool loadShader4FaceGradient(const char * vertexShaderFilePath, const char * fragmentShaderFilePath);
    
    //face texture
    void projectImageToTextureByGLForOBJModelPath(const char * objFilePath, const char * transformFilePath, const char * VFMapFileString, const unsigned char * inputImageData, int imageWidth, int imageHeight, unsigned char * rstTextureData, GLint textureWidth, GLint textureHeight);
    void handleGLTextureData(unsigned char * imageData, int width, int height);
    void doPossionProcessingWithImage(unsigned char * imageData, Laplacian * laplacian, int width, int height);
    void doCompositionWithUpsampledImage(const unsigned char * upsampledImageData, unsigned char * targetImageData, int width, int height);
    
    //background texture
    void processBackgourndTextureData(unsigned char * imageData, int width, int height);
    //void processBackgourndTextureDataUsingEigen(unsigned char * imageData, int width, int height);
    
    //mean value coordiate cloning
    void processTextureMVCWithLaplacian(unsigned char * imageData, const unsigned char * laplacianImageData, int width, int height);
    void processTextureMVC(unsigned char * imageData, int width, int height);
    float evaluateMVCWeight(glm::vec2 v, glm::vec2 leftPoint, glm::vec2 midPoint, glm::vec2 rightPoint);
    
    //face texture with gradient and noise
    void projectImageToTexture(GLuint headVertexArray, const glm::mat4 & mvp, int headVertexNum, const unsigned char * imageData, int imageWidth, int imageHeight, unsigned char * textureData, GLint textureWidth, GLint textureHeight);
    void projectTextureToImage(GLuint headVertexArray, const glm::mat4 & mvp, int headVertexNum, const unsigned char * textureData, GLint textureWidth, GLint textureHeight, unsigned char * imageData, int imageWidth, int imageHeight);
    void createGradientImage(GLuint headVertexArray, const glm::mat4 & mvp, int headVertexNum, unsigned char * imageData, int imageWidth, int imageHeight);
    void doPossionProcessingWithGradientTexture(unsigned char * imageData, const unsigned char * gradientTextureData, const unsigned char * otherImageData, int width, int height, Laplacian * laplacian);
    //void doPossionProcessingWithGradientTextureUsingEigen(unsigned char * imageData, const unsigned char * gradientTextureData, const unsigned char * otherImageData, int width, int height, Laplacian * laplacian);
    void addNoise(const unsigned char * partialFaceImageData, const unsigned char * maskImageData, unsigned char * targetImageData, int width, int height, float pmaxDiff);
    void doCompositionWithUpsampledImageAndMaskImage(const unsigned char * upsampledImageData, unsigned char * targetImageData, int width, int height);
    void doCompositionWithNoiseImage(const unsigned char * noiseImageData, unsigned char * targetImageData, int width, int height);
    
    GLuint _program4FaceTex;
    GLuint _program4FaceImage;
    GLuint _program4FaceGradient;
    Laplacian * _Laplacian;
    unsigned char * _LaplacianImageData;
    unsigned char * _MaskImageData;
    
    char * _ShaderFileDir;
    
    unsigned char * _HeadTextureData;
    int _HeadTextureWidth;
    int _HeadTextureHeight;
    
    unsigned char * _BGTextureData;
    int _BGTextureWidth;
    int _BGTextureHeight;
    
    PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
    PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
    PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
};

#endif

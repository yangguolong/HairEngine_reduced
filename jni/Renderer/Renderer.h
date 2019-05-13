//
//  Renderer.h
//  Demo
//
//  Created by yangguolong on 13-7-15.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef Renderer_H
#define Renderer_H

#include "IRenderer.h"
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "ModelCommonData.h"
#include "HairModel.h"
#include "SurfaceModel.h"
#include "BackModel.h"
#include "IHeadDeformer.h"
#include "ImageBalancer.h"
#include "glm.hpp"
#include "type_ptr.hpp"
#include "matrix_transform.hpp"

#define AnimateExpression

class Renderer : public IRenderer
{
public:
    
    Renderer(const char * headDataDir, const char * shaderFileDir, const char * expressionFileDir);
    ~Renderer();
    void setHeadTextureData(const unsigned char * inputImageData, int width, int height);
    void setBackgroundTexutreData(const unsigned char * inputImageData, int width, int height);
    void setFaceImageData(const unsigned char * inputImageData, int width, int height);

    void initRenderer(int windowWidth, int windowHeight);
    void setRotation(float & xAngle, float & yAngle);
    void setScale(float & scale);
    void setHairPositionAdjustVector(float x, float y, float z);
    void setHairScale(float scale);

    void updateMatrixes();
    void render();
    void releaseMemory();
    void changeHairStyle(const char * hairStyleDir);
    void resizeViewport(int windowWidth, int windowHeight);
    unsigned char * takeScreenshot(int & width, int & height);
    //void evaluateNewSurfaceTransData();
    
    void enableExpression(bool isEnable);
    void changeExpression(int expressionID);
    void changeModifier(int modifierID);
    void changeHairColor(float r, float g, float b, float a);
    void resetExpression();

private:
    void checkSurfaceTransEvaluation(float newS ,float newX, float newY, float newZ);
    
    void initData();
    void updateProjectionMatrix();
    void setupGL();
    void tearDownGL();
    void initImageBalancer();
    void setupModel();

    void loadHeadModel();
    void loadHairModel();
    void loadQuadModel();
    void loadBackQuadModel();
    void loadBodyQuadModel();

    void setHeadAOData();
    void evaluateHairTransferMatrix();
    void updateBackModel();
    void updateHeadModel();
    void clearGLData4Head();
    void clearGLData4Hair();
    void clearGLData4Quad();
    void clearGLData4BackQuad();
    void clearGLData4BodyQuad();
    
    void drawHead(glm::mat4x4 & head_ModelViewProjectionMatrix4);
    void drawHairless(glm::mat4x4 & back_ModelViewProjectionMatrix4, glm::mat4x4 & head_ModelViewProjectionMatrix4);
    void drawSurfeceWithTriangleStrip(glm::mat4x4 & hair_ModelViewProjectionMatrix4);
    
    bool loadShaders();
    bool loadShader4BG(const char * vertexShaderFilePath, const char * fragmentShaderFilePath);
    bool loadShader4Quad(const char * vertexShaderFilePath, const char * fragmentShaderFilePath);
    bool loadShader4Head(const char * vertexShaderFilePath, const char * fragmentShaderFilePath);
    bool loadShader4Surface(const char * vertexShaderFilePath, const char * fragmentShaderFilePath);
    
    GLuint _program4BG;
    GLuint _program4Head;
    GLuint _program4Surface;
    
    glm::mat4x4 _ProjectionMatrix;
    glm::mat4x4 _Head_ModelViewMatrix4;
    glm::mat4x4 _Head_ModelViewProjectionMatrix4;
    glm::mat4x4 _Hair_ModelViewProjectionMatrix4;
    glm::mat4x4 _Back_ModelViewProjectionMatrix4;
    glm::mat4x4 _Body_ModelViewProjectionMatrix4;
    
    GLuint _HeadVertexArray;
    GLuint _HeadPositionBuffer;
    GLuint _HeadUVBuffer;
    GLuint _HeadNormalBuffer;
    GLuint _HeadTexture;
    
    GLuint _HairGTVertexArray;
    GLuint _HairGTVertexBuffer;
    GLuint _HairGTTexture;
    unsigned int _GTHairNum;
    
    int _HairImageHeight;
    int _HairImageWidth;
    
    GLuint _QuadVeretexArray;
    GLuint _QuadVertexBuffer;
    GLuint _QuadTexture;
    
    GLuint _BackQuadVertexArray;
    GLuint _BackQuadPositionBuffer;
    GLuint _BackQuadUVBuffer;
    GLuint _BackQuadIndexBuffer;
    GLuint _BackTexture;
    
    int _BackImageWidth;
    int _BackImageHeight;
    
    //currently no use
    GLuint _BodyQuadVertexArray;
    GLuint _BodyQuadVertexBuffer;
    GLuint _BodyTexture;
    int _BodyImageWidth;
    int _BodyImageHeight;

    float _RotateX;
    float _RotateY;
    float _Scale;
    
    float _LastRotateX;
    float _LastRotateY;
    float _LastScale;

    HeadModel * _HeadModel;
    SurfaceModel * _SurfaceModel;
    BackModel * _BackModel;
    
    IHeadDeformer * _HeadDeformer;
    ImageBalancer * _ImageBalancer;
    
    glm::mat4x4 _HairTransferMatrix;
    glm::mat4x4 _BackTransferMatrix;
    
    int _WindowWidth;
    int _WindowHeight;
    
    char * _HeadDataDir;
    char * _ShaderFileDir;
    char * _ExpressionFileDir;
    char   _CurrentHairStyleDir[500];
    
    unsigned char * _HeadTextureData;
    int _HeadTextureWidth;
    int _HeadTextureHeight;
    
    unsigned char * _BGTextureData;
    int _BGTextureWidth;
    int _BGTextureHeight;
    
    unsigned char * _FaceImageData;
    int _FaceImageWidth;
    int _FaceImageHeight;
    
    bool _EnableExpression;
    bool _AlreadlySelectHairStyle;
    
    float _HairPositionAdjustX;
    float _HairPositionAdjustY;
    float _HairPositionAdjustZ;
    float _HairScale;
    float _HairScaleX;

    unsigned char * _ScreenShotData;
    
    glm::mat4x4 _HairForwardMatrix;
    glm::mat4x4 _HairAdjustMatrix;
    
    glm::vec4 _BlendColor;

    PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
    PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
    PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
};

#endif

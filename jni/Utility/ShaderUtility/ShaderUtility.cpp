//
//  ShaderUtility.cpp
//  Demo
//
//  Created by yangguolong on 13-7-8.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "ShaderUtility.h"
#include "ImageUtility.h"
#include <stdio.h>
#include <stdlib.h>

#include <android/log.h>
#define LOG_TAG "ShaderUtility"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,##__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,##__VA_ARGS__)

bool readShaderStringFromFile(const char * filePath, char * sourceString, int sourceSize)
{
    FILE * file = fopen(filePath, "rb");
    if(!file)
    {
        LOGE("Failed to open shader file %s.\n", filePath);
        return false;
    }
    int readByteNum = fread(sourceString, sizeof(char), sourceSize, file);
    if(readByteNum >= sourceSize)
    {
        LOGE("Read shader file error: the size (%d) of source string is not enough.\n", sourceSize);
        fclose(file);
        return false;
    }
    fread(sourceString, sizeof(char), readByteNum, file);
    sourceString[readByteNum] = '\0';
    fclose(file);
    return true;
}

bool compileShader(GLuint * shader, GLenum type, const char * filePath, int fileBytes)
{
	GLCheck(compileShader);
    GLint status;
    GLchar *source = new GLchar[fileBytes];
    if(!source)
    {
        LOGE("Failed to alloc memory for char array of shader %s.\n",filePath);
        return false;
    }
    readShaderStringFromFile(filePath, source, fileBytes);
    
	glCheck("before create shader", filePath);
    *shader = glCreateShader(type);
	glCheck("after create shader", filePath);

    glShaderSource(*shader, 1, const_cast<const GLchar**>(&source), nullptr);
    glCompileShader(*shader);
    delete [] source;
    
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        printf("Shader compile log:\n%s", log);
        free(log);
    }
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        glDeleteShader(*shader);
        return false;
    }
    
    return true;
}

bool linkProgram(GLuint prog)
{
	GLCheck(linkProgram);
    GLint status;
    glLinkProgram(prog);
    
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        printf("Program link log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0) {
        return false;
    }
    
    return true;
}

bool validateProgram(GLuint prog)
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        printf("Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0) {
        return false;
    }
    
    return true;
}

bool checkCurrenFrameBufferStatus()
{
    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    bool isSuccess = false;
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE_OES: isSuccess = true;break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES: LOGE("Framebuffer error: attachment points are not complete.\n");break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES: LOGE("Framebuffer error: attachment do not have the same width and height.\n");break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_OES: LOGE("Framebuffer error: internal format used by the attachments is not renderable.\n");break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES: LOGE("Framebuffer error: no valid attachments in the framebuffer.\n");break;
        case GL_FRAMEBUFFER_UNSUPPORTED_OES: LOGE("Framebuffer error: Combination of internal formats results in a nonrenderable target.\n");;break;
        default:LOGE("unknow status for framebuffer.\n");break;
    }
    
    return isSuccess;
}

bool loadTexture(const char * filePath, unsigned int * textureID, int * theWidth, int * theHeight)
{
    int width, height;
    GLubyte * imageData;
    GLenum err;
    
    *textureID = 0;
    if(theWidth != NULL)
        (*theWidth) = 0;
    if(theHeight != NULL)
        (*theHeight) = 0;
    
    imageData = getImageDataAndInfoForImagePath(filePath, &width, &height);
    if(!imageData)
    {
        LOGE("Failed to load texture.\n");
        return false;
    }
    
    //翻转图片，适配OpengGL
    flipImageData(imageData, width, height);
    
    glGenTextures(1, textureID);
    glBindTexture(GL_TEXTURE_2D, (*textureID));
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    
    err = glGetError();
    if (err != GL_NO_ERROR)
        LOGE("Error uploading texture. glError: 0x%04X\n", err);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    delete [] imageData;
    
    if(theWidth != NULL)
        (*theWidth) = width;
    if(theHeight != NULL)
        (*theHeight) = height;
    
    return true;
}

bool loadTextureFromImageData(const unsigned char * inputImageData, int width, int height, unsigned int * textureID)
{
    GLenum err;
    
    *textureID = 0;
    
    if(!inputImageData)
    {
        printf("Failed to load texture.\n");
        return false;
    }
    unsigned char * imageData = new unsigned char[width * height * 4];
    memcpy(imageData, inputImageData, width*height*4);
    
    //翻转图片，适配OpengGL
    flipImageData(imageData, width, height);
    
    glGenTextures(1, textureID);
    glBindTexture(GL_TEXTURE_2D, (*textureID));
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    
    err = glGetError();
	if (err != GL_NO_ERROR) {
		LOGE("Error uploading texture. glError: 0x%04X\n", err);
		LOGE("width:%d height:%d addr:%p", width, height, imageData);
	}
    
    glBindTexture(GL_TEXTURE_2D, 0);
    delete [] imageData;
    
    return true;
}

void printError(GLenum error)
{
    switch (error)
    {
        case GL_NO_ERROR:
            LOGE("NO GL ERROR.\n");
            break;
        case GL_INVALID_ENUM:
            LOGE("GL_INVALID_ENUM.\n");
            break;
        case GL_INVALID_VALUE:
            LOGE("GL_INVALID_VALUE.\n");
            break;
        case GL_INVALID_OPERATION:
            LOGE("GL_INVALID_OPERATION.\n");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            LOGE("GL_INVALID_FRAMEBUFFER_OPERATION.\n");
            break;
        case GL_OUT_OF_MEMORY:
            LOGE("GL_OUT_OF_MEMORY.\n");
            break;
        default:
            LOGE("in printError, unknow gl error 0x%04X.\n", error);
            break;
    }
}

void resetAllBinds()
{
    PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC) eglGetProcAddress("glBindVertexArrayOES");
    
    glUseProgram(0);
    glBindVertexArrayOES(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D , 0);
}

void glCheck(const char* tag, const char* msg)
{
	auto err = glGetError();
	if (err) {
		__android_log_print(ANDROID_LOG_ERROR, tag, "glCheck Err:%s msg:%s", GLChecker::transCode (err), msg);
	}
}

GLChecker::~GLChecker()
{
	auto err = glGetError();
	const char* msg = transCode(err);
	if (err) {
		__android_log_print(
			ANDROID_LOG_ERROR, 
			"GLChecker", "OpenGL Error: 0x%08x in function:%s file:%s line:%d, msg:%s", 
			err, m_func, m_file, m_line, transCode(err));
	}
}

const char* GLChecker::transCode(int errCode)
{
	switch (errCode) {
	case GL_NO_ERROR: 
		return"GL_NO_ERROR";
	case GL_INVALID_ENUM: 
		return"GL_INVALID_ENUM"; 
	case GL_INVALID_VALUE: 
		return"GL_INVALID_VALUE"; 
	case GL_INVALID_OPERATION: 
		return"GL_INVALID_OPERATION"; 
	case GL_INVALID_FRAMEBUFFER_OPERATION: 
		return"GL_INVALID_FRAMEBUFFER_OPERATION"; 
	case GL_OUT_OF_MEMORY: 
		return"GL_OUT_OF_MEMORY"; 
	default: 
		return"Invalid OpenGL Error"; 
	}
}

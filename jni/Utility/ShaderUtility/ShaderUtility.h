//
//  ShaderUtility.h
//  Demo
//
//  Created by yangguolong on 13-7-8.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef ShaderUtility_H
#define ShaderUtility_H

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef enum {
    HVertexAttribPosition,
    HVertexAttribNormal,
    HVertexAttribColor,
    HVertexAttribTexCoord0,
    HVertexAttribTexCoord1,
} HVertexAttrib;

bool readShaderStringFromFile(const char * filePath, char * sourceString, int sourceSize);
bool compileShader(GLuint * shader, GLenum type, const char * filePath, int fileBytes);
bool linkProgram(GLuint prog);
bool validateProgram(GLuint prog);
bool checkCurrenFrameBufferStatus();
bool loadTexture(const char * filePath, unsigned int * textureID, int * theWidth, int * theHeight);
bool loadTextureFromImageData(const unsigned char * imageData, int width, int height, unsigned int * textureID);
void printError(GLenum error);
void resetAllBinds();

struct GLChecker
{
	GLChecker(const char* file, int line, const char* func) : 
		m_file(file), m_line(line), m_func(func) {}
	~GLChecker();

	static const char* transCode(int errCode);

private:
	const char* m_file = "invalid file";
	int	m_line = 0;
	const char* m_func = "invalid function";
};

#define GLCheck(func) GLChecker __glChecker__{__FILE__, __LINE__, #func}
void glCheck(const char* tag, const char* msg);

#endif

//
//  HeadModel.cpp
//  OBJModel
//
//  Created by yangguolong on 13-6-22.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "HeadModel.h"

#include <android/log.h>
#define LOG_TAG "HeadModel"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

HeadModel::HeadModel()
{
    _FaceNum = 0;
    _DisplacementZ = 0.0f;
    _DisplacementY = 0.0f;
    _PositionDataArray = NULL;
    _UVDataArray = NULL;
    _OBJModel = NULL;
}

HeadModel::~HeadModel()
{
    resetModel();
}

bool HeadModel::loadModelFromFile(const char * objModelFilePath, const char * transformDataFilePath)
{
    if(objModelFilePath == NULL || transformDataFilePath == NULL)
        return false;
    
    resetModel();
    
    //load transform file
    FILE * transformFile = fopen(transformDataFilePath, "r");
    if(!transformFile)
    {
        printf("Failed to load head transform file (%s)", transformDataFilePath);
        return false;
    }
    for(int i=0; i<9;i++)
        fscanf(transformFile, "%f", &_TransformMatrix.rotation[i]);
    for(int i=0; i<3; i++)
        fscanf(transformFile, "%f", &_TransformMatrix.positionTrans[i]);
    fscanf(transformFile, "%f", &_TransformMatrix.scaleFactor);

    //read in the displacement value, ignore hair translate
    for(int i=0; i<4; i++)
        fscanf(transformFile, "%f", &_DisplacementX);
    fscanf(transformFile, "%f", &_DisplacementY);
    fscanf(transformFile, "%f", &_DisplacementZ);
    
    LOGI("DisplacementX %f, DisplacementY %f, DisplacementZ %f\n", _DisplacementX, _DisplacementY, _DisplacementZ);

    fclose(transformFile);
    
    // load obj file
    _OBJModel = new OBJModel();
    if(!_OBJModel->loadOBJFile(objModelFilePath))
    {
        printf("Failed to load obj file (%s)", objModelFilePath);
        resetModel();
        return false;
    }
    //get _FaceNum
    _FaceNum = 0;
	for(unsigned int i = 0; i<_OBJModel->vGroups.size();i++)
		_FaceNum += ((OBJGroup)_OBJModel->vGroups[i]).vFaces.size();
    
    //get _BoundBox
     _BoundBox = _OBJModel->boundBox;
    
    //get _VertexDataArray
    _PositionDataArray = new char[_FaceNum * 3 * sizeof(float) * 3];
    _UVDataArray = new char[_FaceNum * 3 * sizeof(float) * 2];
    
    float * positionArray = (float *)_PositionDataArray;
    float * uvArray = (float *)_UVDataArray;
    int positionIndex = 0;
    int uvIndex = 0;
    float * tmpArray;
    vector<OBJGroup> & mGroups = _OBJModel->vGroups;
	for(unsigned int i = 0;i<mGroups.size();i++)
	{
		const OBJGroup& group = mGroups[i];
		for(unsigned int j = 0;j<group.vFaces.size();j++)
		{
			const OBJFace & face = group.vFaces[j];
			const vector<OBJFaceVertex> & faceVertexs = face.vFaceVertexs;
			for(unsigned int p = 0;p<faceVertexs.size();p++)
			{
                tmpArray = _OBJModel->vVertexs[faceVertexs[p].vertexIndex].position;
                for(int k=0;k<3;k++)
                    positionArray[positionIndex++] = tmpArray[k];
                tmpArray = _OBJModel->vTexcoords[faceVertexs[p].uvIndex].uv;
                for(int k=0;k<2;k++)
                    uvArray[uvIndex++] = tmpArray[k];
			}
		}
	}
    
    assert( (positionIndex == (_FaceNum * 3 * 3)) );
    assert( (uvIndex == (_FaceNum * 3 * 2)) );
    
    _ExpressionVertex.resize(_OBJModel->vVertexs.size());
    
    return true;
}

void HeadModel::resetModel()
{
    _FaceNum = 0;
    
    if(_PositionDataArray)
    {
        delete [] _PositionDataArray;
        _PositionDataArray = NULL;
    }
    if(_UVDataArray)
    {
        delete [] _UVDataArray;
        _UVDataArray = NULL;
    }
    
    if(_OBJModel)
    {
        _OBJModel->resetOBJModel();
        delete _OBJModel;
        _OBJModel = NULL;
    }
    
    _ExpressionVertex.clear();
}


char * HeadModel::getPositionArray()
{
    return _PositionDataArray;
}

void HeadModel::clearPositionDataArray()
{
    if(_PositionDataArray)
    {
        delete [] _PositionDataArray;
        _PositionDataArray = NULL;
    }
}

char * HeadModel::getUVDataArray()
{
    return _UVDataArray;
}

void HeadModel::clearUVDataArray()
{
    if(_UVDataArray)
    {
        delete [] _UVDataArray;
        _UVDataArray = NULL;
    }
}

bool HeadModel::generateNormalDataArrayFromAO(float * aoDataArray)
{
    //loadModelFromFile should be called first to get _OBJModel and _FaceNum
    _NormalDataArray = new char[_FaceNum * 3 * sizeof(float) * 3];
    if(!_NormalDataArray)
    {
        printf("Failed to alloc memory for normal data array.\n");
        return false;
    }
    float * normalArray = (float *)_NormalDataArray;
    int vertexIndex = 0;
    float tmpVal;
    vector<OBJGroup> & mGroups = _OBJModel->vGroups;
	for(unsigned int i = 0;i<mGroups.size();i++)
	{
		const OBJGroup& group = mGroups[i];
		for(unsigned int j = 0;j<group.vFaces.size();j++)
		{
			const OBJFace & face = group.vFaces[j];
			const vector<OBJFaceVertex> & faceVertexs = face.vFaceVertexs;
			for(unsigned int p = 0;p<faceVertexs.size();p++)
			{
                int index = faceVertexs[p].vertexIndex;
                tmpVal = aoDataArray[index];
                normalArray[vertexIndex++] = tmpVal;
                normalArray[vertexIndex++] = tmpVal;
                normalArray[vertexIndex++] = tmpVal;
			}
		}
	}
    
    assert( ((vertexIndex * sizeof(float)) == (_FaceNum * 3 * sizeof(float) * 3)) );
    
    return true;
}

// ao data inside
char * HeadModel::getNormalDataArray()
{
    return _NormalDataArray;
}

void HeadModel::clearNormalDataArray()
{
    if(_NormalDataArray)
    {
        delete [] _NormalDataArray;
        _NormalDataArray = NULL;
    }
}

vector<OBJVertex> & HeadModel::getOBJVertexVector()
{
    return _OBJModel->vVertexs;
}

vector<OBJVertex> & HeadModel::getExpressionOBJVertexVector()
{
    return _ExpressionVertex;
}

BoundBox * HeadModel::getBoundBox()
{
    return &_BoundBox;
}

TransformMatrix * HeadModel::getTransformMatrix()
{
    return &_TransformMatrix;
}

unsigned int HeadModel::getVertexNum()
{
    return _FaceNum * 3;
}

void HeadModel::updataHeadPositionArrayWithOBJVertexVector(vector<OBJVertex> & vertexVector)
{
    float * positionArray = (float *)_PositionDataArray;
    int positionIndex = 0;
    float * tmpArray;
    vector<OBJGroup> & mGroups = _OBJModel->vGroups;
	for(unsigned int i = 0;i<mGroups.size();i++)
	{
		const OBJGroup& group = mGroups[i];
		for(unsigned int j = 0;j<group.vFaces.size();j++)
		{
			const OBJFace & face = group.vFaces[j];
			const vector<OBJFaceVertex> & faceVertexs = face.vFaceVertexs;
			for(unsigned int p = 0;p<faceVertexs.size();p++)
			{
                tmpArray = vertexVector[faceVertexs[p].vertexIndex].position;
                for(int k=0;k<3;k++)
                    positionArray[positionIndex++] = tmpArray[k];
			}
		}
	}
    
    assert( (positionIndex == (_FaceNum * 3 * 3)) );
}


char * HeadModel::getOriginPositionArray()
{
    float * positionArray = new float[_FaceNum * 3 * 3];
    int positionIndex = 0;
    float * tmpArray;
    vector<OBJGroup> & mGroups = _OBJModel->vGroups;
	for(unsigned int i = 0;i<mGroups.size();i++)
	{
		const OBJGroup& group = mGroups[i];
		for(unsigned int j = 0;j<group.vFaces.size();j++)
		{
			const OBJFace & face = group.vFaces[j];
			const vector<OBJFaceVertex> & faceVertexs = face.vFaceVertexs;
			for(unsigned int p = 0;p<faceVertexs.size();p++)
			{
                tmpArray = _OBJModel->vVertexs[faceVertexs[p].vertexIndex].position;
                for(int k=0;k<3;k++)
                    positionArray[positionIndex++] = tmpArray[k];
			}
		}
	}
    
    assert( (positionIndex == (_FaceNum * 3 * 3)) );
    
    return (char *)positionArray;
}

float HeadModel::getDisplacementZ()
{
    return _DisplacementZ;
}

float HeadModel::getDisplacementY()
{
    return _DisplacementY;
}

float HeadModel::getDisplacementX()
{
	return _DisplacementX;
}

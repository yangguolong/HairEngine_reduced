//
//  HeadModel.cpp
//  OBJModel
//
//  Created by yangguolong on 13-6-22.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <vector>
#include "TexHeadModel.h"
#include "glm.hpp"

TexHeadModel::TexHeadModel()
{
    _FaceNum = 0;
    _VertexDataArray = NULL;
}

TexHeadModel::~TexHeadModel()
{
    resetModel();
}

bool TexHeadModel::loadModelFromFile(const char * objModelFilePath, const char * transformDataFilePath)
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
    fclose(transformFile);
    
    // load obj file
    OBJModel * pModel = new OBJModel();
    if(!pModel->loadOBJFile(objModelFilePath))
    {
        printf("Failed to load obj file (%s)\n", objModelFilePath);
        pModel->resetOBJModel();
        delete pModel;
        return false;
    }
    //get _FaceNum
    _FaceNum = 0;
	for(unsigned int i = 0; i<pModel->vGroups.size();i++)
		_FaceNum += ((OBJGroup)pModel->vGroups[i]).vFaces.size();
    
    //get _BoundBox
     _BoundBox = pModel->boundBox;
    
    unsigned int unitLength = getVertexUnitSize();
    _VertexDataArray = new char[_FaceNum * 3 * unitLength];
    
    float * vertexArray = (float *)_VertexDataArray;
    int vertexIndex = 0;
    float * tmpArray;
    vector<OBJGroup> & mGroups = pModel->vGroups;
	for(unsigned int i = 0;i<mGroups.size();i++)
	{
		const OBJGroup& group = mGroups[i];
		for(unsigned int j = 0;j<group.vFaces.size();j++)
		{
			const OBJFace & face = group.vFaces[j];
			const vector<OBJFaceVertex> & faceVertexs = face.vFaceVertexs;
			for(unsigned int p = 0;p<faceVertexs.size();p++)
			{
                tmpArray = pModel->vVertexs[faceVertexs[p].vertexIndex].position;
                for(int k=0;k<3;k++)
                    vertexArray[vertexIndex++] = tmpArray[k];
                tmpArray = pModel->vTexcoords[faceVertexs[p].uvIndex].uv;
                for(int k=0;k<2;k++)
                    vertexArray[vertexIndex++] = tmpArray[k];
			}
		}
	}
    
    assert( ((vertexIndex * sizeof(float)) == (_FaceNum * 3 * unitLength)) );
    
    pModel->resetOBJModel();
    delete pModel;
    
    return true;
}

void TexHeadModel::resetModel()
{
    _FaceNum = 0;
    
    if(_VertexDataArray)
    {
        delete [] _VertexDataArray;
        _VertexDataArray = NULL;
    }
}

void TexHeadModel::clearVertexDataArray()
{
    if(_VertexDataArray)
    {
        delete [] _VertexDataArray;
        _VertexDataArray = NULL;
    }
}

char * TexHeadModel::getVertexDataArray()
{
    return _VertexDataArray;
}

BoundBox * TexHeadModel::getBoundBox()
{
    return &_BoundBox;
}

TransformMatrix * TexHeadModel::getTransformMatrix()
{
    return &_TransformMatrix;
}

unsigned int TexHeadModel::getVertexNum()
{
    return _FaceNum * 3;
}

unsigned int TexHeadModel::getVertexUnitSize()
{
    return sizeof(float) * 3 + sizeof(float) * 2;
}
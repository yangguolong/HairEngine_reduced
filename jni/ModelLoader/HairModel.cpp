//
//  HairModel.cpp
//  OBJModel
//
//  Created by yangguolong on 13-6-24.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "HairModel.h"
#include <stdio.h>
#include <stdlib.h>

HairModel::HairModel()
{
    _SortedVertexNum = 0;
    _AONum = 0;
    _VertexDataArray = NULL;
    _AODataArray = NULL;
}

HairModel::~HairModel()
{
    resetModel();
}

bool HairModel::loadModelFromFile(const char * hairModelFilePath, const char * transformDataFilePath, const char * aoFilePath)
{
    if(hairModelFilePath == NULL || transformDataFilePath == NULL || aoFilePath == NULL)
        return false;
    
    FILE * transformFile = fopen(transformDataFilePath, "r");
    if(!transformFile)
    {
        printf("Failed to open hair transform data file.\n");
        return false;
    }
    for(int i=0; i<9;i++)
        fscanf(transformFile, "%f", &_TransformMatrix.rotation[i]);
    for(int i=0; i<3; i++)
        fscanf(transformFile, "%f", &_TransformMatrix.positionTrans[i]);
    fscanf(transformFile, "%f", &_TransformMatrix.scaleFactor);
    fclose(transformFile);
    
    FILE * aoFile = fopen(aoFilePath, "rb");
    if(!aoFile)
    {
        printf("Failed to open hair ao data file.\n");
        return false;
    }
    fread(&_AONum, sizeof(int), 1, aoFile);
    _AODataArray = new float[_AONum];
    if(!_AODataArray)
    {
        printf("Failed to allocate memory for origin hair ao.\n");
        fclose(aoFile);
        return false;
    }
    fread(_AODataArray, sizeof(float), _AONum, aoFile);
    fclose(aoFile);
    
    FILE * hairModelFile = fopen(hairModelFilePath, "rb");
    if(!hairModelFile)
    {
        printf("Failed to open hair modelfile.\n");
        resetModel();
        return false;
    }
    fread(&_SortedVertexNum, sizeof(unsigned int), 1, hairModelFile);
    fread(&_BoundBox, sizeof(BoundBox), 1, hairModelFile);
    
    unsigned int unitLength =  HairModel::getVertexUnitSize();
    _VertexDataArray = (char *)calloc(_SortedVertexNum,unitLength);
    if(!_VertexDataArray)
    {
        printf("Failed to allocate memory for hair vertexs.\n");
        fclose(hairModelFile);
        resetModel();
        return false;
    }
    fread(_VertexDataArray, HairModel::getVertexUnitSize(), _SortedVertexNum, hairModelFile);
    fclose(hairModelFile);
    
    return true;
}

void HairModel::resetModel()
{
    _SortedVertexNum = 0;
    _AONum = 0;
    
    if(_VertexDataArray)
    {
        delete [] _VertexDataArray;
        _VertexDataArray = NULL;
    }
    if(_AODataArray)
    {
        delete [] _AODataArray;
        _AODataArray = NULL;
    }
}

void HairModel::clearVertexDataArray()
{
    if(_VertexDataArray)
    {
        delete [] _VertexDataArray;
        _VertexDataArray = NULL;
    }
}

void HairModel::clearAODataArray()
{
    if(_AODataArray)
    {
        delete [] _AODataArray;
        _AODataArray = NULL;
    }
}

char * HairModel::getVertexDataArray()
{
    return _VertexDataArray;
}

float * HairModel::getAODataArray()
{
    return _AODataArray;
}

BoundBox * HairModel::getBoundBox()
{
    return &_BoundBox;
}

TransformMatrix * HairModel::getTransformMatrix()
{
    return &_TransformMatrix;
}

unsigned int HairModel::getSortedVertexNum()
{
    return _SortedVertexNum;
}

int HairModel::getAONum()
{
    return _AONum;
}

unsigned int HairModel::getVertexUnitSize()
{
    return sizeof(float) * 3;
}
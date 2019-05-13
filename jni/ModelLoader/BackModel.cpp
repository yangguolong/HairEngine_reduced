//
//  BackModel.cpp
//  Hair Preview
//
//  Created by yangguolong on 13-6-27.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "BackModel.h"
#include <stdio.h>
#include <stdlib.h>

#define DEPTH 0.0f
#define QUALITY 10
#define BLENDWIDTH 100.0f

BackModel::BackModel()
{
    _OriginPositionArray = NULL;
    _MapNodeArray = NULL;
    _HeadModelVertexArray = NULL;
    //_DeformVecArray = NULL;
    _FeaturePointNum = 0;
    
    _PositionArray = NULL;
    _UVArray = NULL;
    _IndexArray = NULL;
    
    _ImageWidth = 0;
    _ImageHeight = 0;
    _NumMeshVertexW = 0;
    _NumMeshVertexH = 0;
    _TotalMeshVertex = 0;
    _IndexNum = 0;
    _NumMapNode = 0;
}

BackModel::~BackModel()
{
    resetModel();
}

void BackModel::resetModel()
{
    if(_OriginPositionArray)
        delete [] _OriginPositionArray;
    if(_MapNodeArray)
        delete [] _MapNodeArray;
//    if(_DeformVecArray)
//        delete [] _DeformVecArray;
    if(_HeadModelVertexArray)
        delete [] _HeadModelVertexArray;
    if(_PositionArray)
        delete [] _PositionArray;
    if(_UVArray)
        delete [] _UVArray;
    if(_IndexArray)
        delete [] _IndexArray;
    
    _OriginPositionArray = NULL;
    _MapNodeArray = NULL;
    _HeadModelVertexArray = NULL;
    //_DeformVecArray = NULL;
    _FeaturePointNum = 0;
    
    _PositionArray = NULL;
    _UVArray = NULL;
    _IndexArray = NULL;
    
    _ImageWidth = 0;
    _ImageHeight = 0;
    _NumMeshVertexW = 0;
    _NumMeshVertexH = 0;
    _TotalMeshVertex = 0;
    _IndexNum = 0;
    _NumMapNode = 0;
}

bool BackModel::loadBackModel(int width, int height, const char * boundaryFilePath, HeadModel * headModel, glm::mat4x4 matrix)
{    
    if( (!initFeaturePointArray(boundaryFilePath, headModel, matrix, height)) ||
        (!initMesh(width, height)) ||
       (!evaluateMapNodeArray(width, height, _FeaturePointArray, _OriginPositionArray)))
    {
        resetModel();
        return false;
    }
    
    return true;
}

bool BackModel::initFeaturePointArray(const char *boundaryFilePath, HeadModel * headModel, glm::mat4x4 matrix, int imageHeight)
{
    //CFAbsoluteTime startTime, endTime;
    
    //startTime = CFAbsoluteTimeGetCurrent();
    
    if(boundaryFilePath == NULL)
        return false;
    
    FILE * boundaryFile = fopen(boundaryFilePath, "r");
    if(!boundaryFile)
    {
        printf("Failed to open boundary file %s\n", boundaryFilePath);
        return false;
    }
    
    fscanf(boundaryFile, "%d", &_FeaturePointNum);
    _HeadModelVertexArray = new MVec3f[_FeaturePointNum];
    _FeaturePointArray = new MVec2f[_FeaturePointNum];
    if(!_FeaturePointNum || !_HeadModelVertexArray)
    {
        printf("Failed to alloc memory for feature points.\n");
        fclose(boundaryFile);
        return false;
    }
    int index;
    float x,y;
    //GLKVector4 vec;
    glm::vec4 vec;
    vector<OBJVertex> vVertexes = headModel->getOBJVertexVector();
    for(int i=0; i<_FeaturePointNum; i++)
    {
        fscanf(boundaryFile, "%f %f %d", &x, &y, &index);
        memcpy(_HeadModelVertexArray[i].position, vVertexes[index].position, sizeof(float) * 3);
        vec = glm::vec4(_HeadModelVertexArray[i].x, _HeadModelVertexArray[i].y, _HeadModelVertexArray[i].z, 1.0f);
        vec = matrix * vec;
        _FeaturePointArray[i].x = vec.x;
        _FeaturePointArray[i].y = vec.y;
    }
    fclose(boundaryFile);
    
    //endTime = CFAbsoluteTimeGetCurrent();
    //printf("                    init feature: %lf s\n", endTime - startTime);
    
    return true;
}

bool BackModel::initMesh(int width, int height)
{
    //CFAbsoluteTime startTime, endTime;
    
    //startTime = CFAbsoluteTimeGetCurrent();
    
    _ImageWidth = width;
    _ImageHeight = height;
    
    int numCellsX = _ImageWidth / QUALITY;
    int numCellsY = _ImageHeight / QUALITY;
    
    _NumMeshVertexW = numCellsX + 1;
    _NumMeshVertexH = numCellsY + 1;
    _TotalMeshVertex = _NumMeshVertexW * _NumMeshVertexH;
    _IndexNum = numCellsX * numCellsY * 2 * 3;
    
    _OriginPositionArray = new MVec3f[_TotalMeshVertex];
    MVec3f * positionArray = new MVec3f[_TotalMeshVertex];
    MVec2f * uvArray = new MVec2f[_TotalMeshVertex];
    unsigned int * indexArray = new unsigned int[_IndexNum];
    if( !_OriginPositionArray || !positionArray || ! uvArray || !indexArray)
    {
        printf("Failed to alloc memory for back model OpenGL data.\n");
        return false;
    }
    int arrayIndex = 0;
    float u,v,x,y;
    for(int i=0; i<_NumMeshVertexH; i++)
    {
        v = ((float)i) / ((float)(_NumMeshVertexH - 1));
        y = v * ((float)_ImageHeight);
        
        for(int j=0; j<_NumMeshVertexW; j++)
        {
            // for mesh index (j, i)
            u = ((float)j) / ((float)(_NumMeshVertexW - 1));
            x= u * ((float)_ImageWidth);
            
            _OriginPositionArray[arrayIndex] = MVec3f(x, y, DEPTH);
            uvArray[arrayIndex] = MVec2f(u, v);
            arrayIndex++;
        }
    }
    
    assert(arrayIndex == _TotalMeshVertex);
    
    arrayIndex = 0;
    unsigned int topLeft, topRight, bottomLeft, bottomRight;
    for(int i=0; i<numCellsY; i++)
        for(int j=0; j<numCellsX; j++)
        {
            //for cell (j, i)
            bottomLeft = i * _NumMeshVertexW + j;
            bottomRight = bottomLeft + 1;
            topLeft = bottomLeft + _NumMeshVertexW;
            topRight = topLeft + 1;
            
            indexArray[arrayIndex++] = topLeft;
            indexArray[arrayIndex++] = bottomLeft;
            indexArray[arrayIndex++] = topRight;
            
            indexArray[arrayIndex++] = topRight;
            indexArray[arrayIndex++] = bottomLeft;
            indexArray[arrayIndex++] = bottomRight;
        }
    
    _PositionArray = (float *)positionArray;
    _UVArray = (float *)uvArray;
    _IndexArray = indexArray;
    
    //endTime = CFAbsoluteTimeGetCurrent();
    //printf("                    init mesh: %lf s\n", endTime - startTime);
    return true;
}

bool BackModel::evaluateMapNodeArray(int width, int height, const MVec2f * featurePointsArray, const MVec3f * positionArray)
{
    //坐标轴同OpenGL， y轴向上，坐标原点在图片左下角
    
    int arrayIndex ,tmpIndex;
    float disX, disY, dist;
    float weight;
    
   // grandStartTime = CFAbsoluteTimeGetCurrent();
    //init dist table
    int rectSize = 2 * BLENDWIDTH +1;
    float * weightTable = new float[rectSize * rectSize];
    if(!weightTable)
    {
        printf("Failed to alloc memory for dist table.\n");
        return false;
    }
    arrayIndex = 0;
    const int R2 = BLENDWIDTH * BLENDWIDTH;
    for(int i=0; i<rectSize; i++)
        for(int j=0; j<rectSize; j++)
        {
            //center point is (BLENDWIDTH, BLENDWIDTH)
            //for point (j, i)
            disX = (j + 0.5f) - BLENDWIDTH;
            disY = (i + 0.5f)  - BLENDWIDTH;
            dist = MDIST2(disX, disY);
            
            if(dist < R2)
            {
                dist = sqrtf(dist);
                weight = 1.0f - dist / BLENDWIDTH;
                weight = MMIN(MMAX(weight, 0.0f), 1.0f);
            }
            else
                weight = 0.0f;
    
            weightTable[arrayIndex++] = weight;
        }
    //grandEndTime = CFAbsoluteTimeGetCurrent();
    //printf("                            tag1: %lf s\n", grandEndTime - grandStartTime);
    
    //grandStartTime = CFAbsoluteTimeGetCurrent();
    //cal weight for whole image
    BMWeightNode * imageWeightNodeArray = new BMWeightNode[width * height];
    if(!imageWeightNodeArray)
    {
        printf("Failed to alloc memory for image weight node array.\n");
        delete [] weightTable;
        return false;
    }
    memset(imageWeightNodeArray, 0, sizeof(BMWeightNode) * width * height);
    
    int bboxMin[2];
    int bboxMax[2];
    bboxMin[0] = bboxMin[1] = 1000000;
    bboxMax[0] = bboxMax[1] = -1;
    int x, y;
    int bottom, top, left, right;
    const int UPWIDTH = width - 2;
    const int UPHEIGHT = height - 2;
    BMWeightNode * nodePtr;
    for(int i=0; i<_FeaturePointNum; i++)
    {
        //for _FeaturePointArray[i]
        x = _FeaturePointArray[i].x;
        y = _FeaturePointArray[i].y;
        
        if(x > bboxMax[0])
            bboxMax[0] = x;
        if(x < bboxMin[0])
            bboxMin[0] = x;
        
        if(y > bboxMax[1])
            bboxMax[1] = y;
        if(y < bboxMin[1])
            bboxMin[1] = y;
        
        bottom = (int)(y - BLENDWIDTH);
        top = (int)(y + BLENDWIDTH);
        left = (int)(x - BLENDWIDTH);
        right = (int)(x + BLENDWIDTH);
        //保证背景图片边框不动
        bottom = MMIN(MMAX(bottom, 1), UPHEIGHT);
        top = MMIN(MMAX(top, 1), UPHEIGHT);
        left = MMIN(MMAX(left, 1), UPWIDTH);
        right = MMIN(MMAX(right, 1), UPWIDTH);
        
        for(int j = bottom; j<top; j++)
        {
            for(int k = left; k<right; k++)
            {
                //for pixel (k + 0.5f, j + 0.5f)            
                //for point (BLENDWIDTH + k - x, BLENDWIDTH + j - y)
                tmpIndex = (BLENDWIDTH + j - y) * rectSize + (BLENDWIDTH + k - x);
                arrayIndex = j * width + k;
                weight = weightTable[tmpIndex];
                nodePtr = imageWeightNodeArray + arrayIndex;
                if(weight > nodePtr->weight)
                {
                    nodePtr->featurePointIndex = i;
                    nodePtr->weight = weight;
                }
            }
        }
    }
    
    delete [] weightTable;
    
    bboxMin[0] -= BLENDWIDTH;
    bboxMin[1] -= BLENDWIDTH;
    bboxMax[0] += BLENDWIDTH;
    bboxMax[1] += BLENDWIDTH;
    //不考虑背景图片边框
    bboxMin[0] = MMAX(bboxMin[0], 1);
    bboxMin[1] = MMAX(bboxMin[1], 1);
    bboxMax[0] = MMIN(bboxMax[0], UPWIDTH);
    bboxMax[1] = MMIN(bboxMax[1], UPHEIGHT);
    int lengthY = bboxMax[1] - bboxMin[1];
    int upBound = (int)(lengthY * 0.5f);
    int lowBound = (int)(lengthY * 0.4f);
    int lengthDis = upBound - lowBound;
    float additionWeight;
    for(int i=bboxMin[1]; i<bboxMax[1]; i++)
        for(int j=bboxMin[0]; j<bboxMax[0]; j++)
        {
            //for pixel (j, i)
            int disInBBox = i - bboxMin[1];
            if(disInBBox <= lowBound)
                continue;
            else if(disInBBox < upBound)
            {
                additionWeight = (upBound - disInBBox) / ((float)lengthDis);
                additionWeight = MMAX(additionWeight, 0.0f);
            }
            else
                additionWeight = 0.0f;
            
            //printf("(%d %d) %f\n", j, i, additionWeight);
            arrayIndex = i * width + j;
            imageWeightNodeArray[arrayIndex].weight *= additionWeight;
        }
    
    
    //grandEndTime = CFAbsoluteTimeGetCurrent();
    //printf("                            tag2: %lf s\n", grandEndTime - grandStartTime);
    
    //subEndTime = CFAbsoluteTimeGetCurrent();
    //printf("                        cal Weight: %lf s\n", subEndTime - subStartTime);

    //pick out deformation needed pixles
    //subStartTime = CFAbsoluteTimeGetCurrent();
    BMMapNode * tmpMapNodeArray = new BMMapNode[_TotalMeshVertex];
    if(!tmpMapNodeArray)
    {
        printf("Failed to alloc memory for tmp map node array.\n");
        delete [] imageWeightNodeArray;
        return false;
    }
    
    int xPos, yPos;
    const int MAXWIDTH = width - 1;
    const int MAXHEIGHT = height - 1;
    _NumMapNode = 0;
    for(int i=0; i<_TotalMeshVertex; i++)
    {
        xPos = (int)(MMIN(MMAX(_OriginPositionArray[i].x, 0.0f), MAXWIDTH));
        yPos = (int)(MMIN(MMAX(_OriginPositionArray[i].y, 0.0f), MAXHEIGHT));
        
        arrayIndex = yPos * width + xPos;
        if(imageWeightNodeArray[arrayIndex].weight > 0.0f)
        {
            tmpMapNodeArray[_NumMapNode++] = BMMapNode(i,
                                                       imageWeightNodeArray[arrayIndex].featurePointIndex,
                                                       imageWeightNodeArray[arrayIndex].weight);
            //printf("(%d, %d) = %f\n", xPos, yPos, imageWeightNodeArray[arrayIndex].weight);
        }
    }
    
    _MapNodeArray = new BMMapNode[_NumMapNode];
    if(!_MapNodeArray)
    {
        printf("Failed to alloc memory for map node array.\n");
        delete [] imageWeightNodeArray;
        delete [] tmpMapNodeArray;
        return false;
    }
    memcpy(_MapNodeArray, tmpMapNodeArray, sizeof(BMMapNode) * _NumMapNode);
    
    delete [] imageWeightNodeArray;
    delete [] tmpMapNodeArray;
    
    //subEndTime = endTime = CFAbsoluteTimeGetCurrent();
    //printf("                        create map node: %lf s\n                    evaluate Map: %lf s\n", subEndTime - subStartTime, endTime - startTime);
    
    return true;
}

void BackModel::runDeform(glm::mat4x4 matrix)
{
    MVec2f * deformVecArray = new MVec2f[_FeaturePointNum];
    glm::vec4 vec;
    for(int i=0; i<_FeaturePointNum; i++)
    {
        vec = glm::vec4(_HeadModelVertexArray[i].x, _HeadModelVertexArray[i].y, _HeadModelVertexArray[i].z, 1.0f);
        vec = matrix * vec;
        //vec.y = _ImageHeight - vec.y;
        deformVecArray[i] = MVec2f(vec.x - _FeaturePointArray[i].x, vec.y - _FeaturePointArray[i].y);
    }
    
    MVec3f * tmpArray = (MVec3f *)_PositionArray;
    memcpy(tmpArray, _OriginPositionArray, sizeof(MVec3f) * _TotalMeshVertex);
    int meshPointIndex, deformVecIndex;
    float weight;
    for(int i=0; i<_NumMapNode; i++)
    {
        meshPointIndex = _MapNodeArray[i].meshPointIndex;
        deformVecIndex = _MapNodeArray[i].featurePointIndex;
        weight = _MapNodeArray[i].weight;
        
        tmpArray[meshPointIndex].x += deformVecArray[deformVecIndex].x * weight;
        tmpArray[meshPointIndex].y += deformVecArray[deformVecIndex].y * weight;
    }
    
    delete [] deformVecArray;
}

char * BackModel::getPositionArray()
{
    return (char *)_PositionArray;
}

char * BackModel::getUVArray()
{
    return (char *)_UVArray;
}

char * BackModel::getIndexArray()
{
    return (char *)_IndexArray;
}

int BackModel::getVertexNum()
{
    return _TotalMeshVertex;
}

int BackModel::getIndexNum()
{
    return _IndexNum;
}

void BackModel::clearPositionArray()
{
    if(_PositionArray)
    {
        delete [] _PositionArray;
        _PositionArray = NULL;
    }
    
}

void BackModel::clearUVArray()
{
    if(_UVArray)
    {
        delete [] _UVArray;
        _UVArray = NULL;
    }
    
}

void BackModel::clearIndexArray()
{
    if(_IndexArray)
    {
        delete [] _IndexArray;
        _IndexArray = NULL;
    }
}
//
//  BackModel.h
//  Hair Preview
//
//  Created by yangguolong on 13-6-27.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef BackModel_H
#define BackModel_H

#include "HeadModel.h"
#include "ModelCommonData.h"
#include "glm.hpp"

struct BMWeightNode
{
    BMWeightNode()
    {
        featurePointIndex = -1;
        weight = 0.0f;
    }
    int featurePointIndex;
    float weight;
};

struct BMMapNode
{
    BMMapNode()
    {
        meshPointIndex = -1;
        featurePointIndex = -1;
        weight = 0.0f;
    }
    BMMapNode(int pointIndex, int featureIndex, float value)
    {
        meshPointIndex = pointIndex;
        featurePointIndex = featureIndex;
        weight = value;
    }
    int meshPointIndex;
    int featurePointIndex;
    float weight;
};

class BackModel
{
public:
    BackModel();
    ~BackModel();
    bool loadBackModel(int width, int height, const char * boundaryFilePath, HeadModel * headModel, glm::mat4x4 matrix);
    void runDeform(glm::mat4x4 matrix);
    
    char * getPositionArray();
    char * getUVArray();
    char * getIndexArray();
    
    int getVertexNum();
    int getIndexNum();
    
    void clearPositionArray();
    void clearUVArray();
    void clearIndexArray();
    void resetModel();
    
private:
    bool initFeaturePointArray(const char * boundaryFilePath, HeadModel * headModel, glm::mat4x4 matrix, int imageHeight);
    bool initMesh(int width, int height);
    bool evaluateMapNodeArray(int width, int height, const MVec2f * featurePointsArray, const MVec3f * positionArray);
    
    int _ImageWidth;    // background image width
    int _ImageHeight;   // background image height
    
    MVec3f * _OriginPositionArray; //mesh
    int _NumMeshVertexW;
    int _NumMeshVertexH;
    int _TotalMeshVertex;
    
    MVec3f * _HeadModelVertexArray;
    MVec2f * _FeaturePointArray; //origin feature points
    //MVec2f * _DeformVecArray;    //newly transformed feature points and new vec
    int _FeaturePointNum;
    
    BMMapNode * _MapNodeArray;   //mesh vertex whose position needs deformation
    int _NumMapNode;
    
    //data for OpenGL
    float * _PositionArray;
    float * _UVArray;
    unsigned int * _IndexArray;
    int _IndexNum;
};

#endif

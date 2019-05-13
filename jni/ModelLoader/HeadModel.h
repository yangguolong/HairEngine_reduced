//
//  HeadModel.h
//  OBJModel
//
//  Created by yangguolong on 13-6-22.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef HeadModel_H
#define HeadModel_H

#include "ModelCommonData.h"
#include "OBJModel.h"
#include <vector>

class HeadModel
{
public:
    HeadModel();
    ~HeadModel();
    bool loadModelFromFile(const char * objModelFilePath, const char * transformDataFilePath);
    void resetModel();
    
    char * getPositionArray();
    char * getUVDataArray();
    void clearPositionDataArray();
    void clearUVDataArray();
    
    bool generateNormalDataArrayFromAO(float * aoDataArray);
    char * getNormalDataArray(); // ao data inside
    void clearNormalDataArray();
    
    vector<OBJVertex> & getOBJVertexVector();
    vector<OBJVertex> & getExpressionOBJVertexVector();
    
    void updataHeadPositionArrayWithOBJVertexVector(vector<OBJVertex> & vertexVector);
    
    char * getOriginPositionArray();
    
    BoundBox * getBoundBox();
    TransformMatrix * getTransformMatrix();
    unsigned int getVertexNum();

    float getDisplacementZ();
    float getDisplacementY();
    float getDisplacementX();
    
private:
    OBJModel * _OBJModel;
    unsigned int _FaceNum;
    BoundBox _BoundBox;
    TransformMatrix _TransformMatrix;
    char * _PositionDataArray;
    char * _UVDataArray;
    char * _NormalDataArray;
    float _DisplacementZ;
    float _DisplacementY;
    float _DisplacementX;
    
    vector<OBJVertex> _ExpressionVertex;
};

#endif

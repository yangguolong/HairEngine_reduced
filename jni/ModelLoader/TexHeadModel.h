//
//  HeadModel.h
//  OBJModel
//
//  Created by yangguolong on 13-6-22.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef TexHeadModel_H
#define TexHeadModel_H

#include "ModelCommonData.h"
#include "OBJModel.h"

class TexHeadModel
{
public:
    TexHeadModel();
    ~TexHeadModel();
    bool loadModelFromFile(const char * objModelFilePath, const char * transformDataFilePath);
    void resetModel();
    void clearVertexDataArray();
    char * getVertexDataArray();
    BoundBox * getBoundBox();
    TransformMatrix * getTransformMatrix();
    unsigned int getVertexNum();
    static unsigned int getVertexUnitSize();
    
private:
    unsigned int _FaceNum;
    BoundBox _BoundBox;
    TransformMatrix _TransformMatrix;
    char * _VertexDataArray;
};

#endif

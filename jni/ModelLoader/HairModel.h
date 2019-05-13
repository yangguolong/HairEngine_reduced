//
//  HairModel.h
//  OBJModel
//
//  Created by yangguolong on 13-6-24.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef HairModel_H
#define HairModel_H

#include "ModelCommonData.h"

class HairModel {
public:
    HairModel();
    ~HairModel();
    bool loadModelFromFile(const char * hairModelFilePath, const char * transformDataFilePath, const char * aoFilePath);
    void resetModel();
    void clearVertexDataArray();
    void clearAODataArray();
    char * getVertexDataArray();
    float * getAODataArray();
    BoundBox *getBoundBox();
    TransformMatrix * getTransformMatrix();
    unsigned int getSortedVertexNum();
    int getAONum();
    static unsigned int getVertexUnitSize();
    
private:
    BoundBox _BoundBox;
    TransformMatrix _TransformMatrix;
    char * _VertexDataArray;
    unsigned int _SortedVertexNum;
    float * _AODataArray; // passed to vertex shader as vertex normal
    int _AONum;
};

#endif

//
//  SurfaceModel.h
//  OBJModel
//
//  Created by yangguolong on 13-6-25.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef SurfaceModel_H
#define SurfaceModel_H

#include "ModelCommonData.h"

struct MeshVertex
{
	MeshVertex()
	{
        
	}
	MeshVertex(float x, float y, float z)
	{
		position[0] = x;
		position[1] = y;
		position[2] = z;
	}
	float position[3];
};

class SurfaceModel {
public:
    SurfaceModel();
    ~SurfaceModel();
    bool loadModelFromFile(const char * surfaceModelFilePath, const char * transformDataFilePath, const char * aoFilePath);
    bool loadModelFromDepthFile(const char * depthImagePath, const char * depthInfoFilePath, const char * transformDataFilePath);
    void resetModel();
    void clearVertexDataArray();
    void clearAODataArray();
    char * getVertexDataArray();
    float * getAODataArray();
    TransformMatrix * getTransformMatrix();
    unsigned int getSortedVertexNum();
    int getAONum();
    static unsigned int getVertexUnitSize();
    
    float getDisplacementX();
    float getDisplacementY();
    float getDisplacementZ();

private:
    bool loadSUFDataWithXYPostionIndex(const char * surfaceModelFilePath);
    bool loadSUFDataWithDepthImage(const char * depthImgPath, const char * depthInfoFilePath);
    MeshVertex * createTriangleStripFromDepthCullDepthZeroIfAllPointCenterCustomQuadSize(const unsigned char * depthDataArray, int width, int height, int quadSize, int & meshVertexNum);
    
    TransformMatrix _TransformMatrix;
    char * _VertexDataArray;
    unsigned int _SortedVertexNum;
    float * _AODataArray; // passed to vertex shader as vertex normal
    int _AONum;

    float _DisplacementX;
    float _DisplacementY;
    float _DisplacementZ;
};

#endif

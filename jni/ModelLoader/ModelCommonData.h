//
//  ModelCommonData.h
//  OBJModel
//
//  Created by yangguolong on 13-6-22.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef ModelCommonData_H
#define ModelCommonData_H

#define MMAX(a,b) ((a)>(b)?a:b)
#define MMIN(a,b) ((a)<(b)?a:b)
#define MDIST2(a, b) ( ((a) * (a)) + ((b) * (b)) )
#define MDIST(a, b) ( sqrtf( ((a) * (a)) + ((b) * (b)) ) )

#define MC_PI  3.141592f

struct BoundBox
{
	BoundBox()
	{
		for(int i=0;i<3;i++)
		{
			boundMin[i] = 1e10;
			boundMax[i] = -1e10;
			boundCenter[i] = 0.0f;
		}
	}
	float boundMin[3];
	float boundMax[3];
	float boundCenter[3];
	float xLength;
	float yLength;
	float zLength;
};

struct TransformMatrix
{
    float rotation[9];
    float positionTrans[3];
    float scaleFactor;
    
};

union MVec3f
{
    MVec3f()
    {
        
    }
    MVec3f(float a, float b, float c)
    {
        x = a;
        y = b;
        z = c;
    }
    float position[3];
    struct {float x,y,z;};
};

union MVec2f
{
    MVec2f()
    {
        
    }
    MVec2f(float a, float b)
    {
        x = a;
        y = b;
    }
    float position[2];
    struct {float x,y;};
};

#endif

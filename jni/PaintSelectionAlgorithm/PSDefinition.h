//
//  PSDefinition.h
//  MyPainter
//
//  Created by yangguolong on 13-5-9.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef PSDEFINITION_H
#define PSDEFINITION_H

#include <stdlib.h>

#define SQRLENGTH(x,y,z) ((x) * (x) + (y) * (y) + (z) * (z))
#define LENGTH(x,y,z) (sqrtf((x) * (x) + (y) * (y) + (z) * (z)))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Min(a,b) ((a)<(b)?(a):(b))

union PSColor
{
    PSColor(){};
    PSColor(unsigned char R, unsigned char G, unsigned char B)
    {
        r = R;
        g = G;
        b = B;
    }
    
    unsigned char data[3];
    struct {unsigned char r, g, b;};
    struct {unsigned char x, y, z;};
};

union PSColorF
{
    PSColorF(){};
    PSColorF(float R, float G, float B)
    {
        r = R;
        g = G;
        b = B;
    }
    PSColorF(unsigned char R, unsigned char G, unsigned char B)
    {
        r = (float)R;
        g = (float)G;
        b = (float)B;
    }
    
    float data[3];
    struct {float r, g, b;};
    struct {float x, y, z;};
};

union PSPoint
{
    PSPoint(){};
    PSPoint(int theX, int theY)
    {
        x = theX;
        y = theY;
    }
    int data[2];
    struct {int x,y;};
};

union PSPointF
{
    PSPointF(){};
    PSPointF(float theX, float theY)
    {
        x = theX;
        y = theY;
    }
    float data[2];
    struct {float x,y;};
};

struct PSRect
{
    PSRect(){};
    PSRect(int xmin, int xmax, int ymin, int ymax)
    {
        xMin = xmin;
        xMax = xmax;
        yMin = ymin;
        yMax = ymax;
    };
    
    int xMin, xMax, yMin, yMax;
};

struct PSContour
{
    PSContour()
    {
        contourNum = 0;
        totalPointsNum = 0;
        contourPointNumArray  =NULL;
        contourPoints = NULL;
    }
    
    int contourNum;
    int * contourPointNumArray;
    int totalPointsNum;
    PSPointF * contourPoints;
};

#endif

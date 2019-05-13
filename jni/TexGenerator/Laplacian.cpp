//
//  Laplacian.cpp
//  Demo
//
//  Created by yangguolong on 13-7-8.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "Laplacian.h"
#include <stdio.h>
#include <stdlib.h>

Laplacian::Laplacian()
{
    _Width = 0;
    _Height = 0;
    for (int i=0; i<3; i++)
        _Channels[i] = NULL;
}

Laplacian::~Laplacian()
{
    resetModel();
}

void Laplacian::resetModel()
{
    _Width = 0;
    _Height = 0;
    
    for (int i=0; i<3; i++)
        if(_Channels[i])
        {
            delete [] _Channels[i];
            _Channels[i] = NULL;
        }
}

bool Laplacian::loadLaplacianFromFile(const char * filePath)
{
    if(filePath == NULL)
        return false;
    
    resetModel();
    FILE * file = fopen(filePath, "rb");
    if(!file)
    {
        printf("Failed to load laplacian data file %s.\n", filePath);
        return false;
    }
    fread(&_Width, sizeof(int), 1, file);
    fread(&_Height, sizeof(int), 1, file);
    int size = _Width * _Height;
    for(int i=0; i<3; i++)
    {
        _Channels[i] = new float[size];
        if(!_Channels[i])
        {
            printf("Failed to alloc memory for laplacian channel %d.\n", i);
            resetModel();
            return false;
        }
        fread(_Channels[i], sizeof(float), size, file);
    }
    fclose(file);
    
    return true;
}

const float * Laplacian::getRedChannel()
{
    return _Channels[0];
}

const float * Laplacian::getGreenChannel()
{
    return _Channels[1];
}

const float * Laplacian::getBlueChannel()
{
    return _Channels[2];
}

int Laplacian::getWidth()
{
    return _Width;
}

int Laplacian::getHeight()
{
    return _Height;
}

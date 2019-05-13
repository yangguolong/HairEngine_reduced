//
//  Laplacian.h
//  Demo
//
//  Created by yangguolong on 13-7-8.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#ifndef Laplacian_H
#define Laplacian_H

class Laplacian{
    
public:
    Laplacian();
    ~Laplacian();
    void resetModel();
    bool loadLaplacianFromFile(const char * filePath);
    const float * getRedChannel();
    const float * getGreenChannel();
    const float * getBlueChannel();
    int getWidth();
    int getHeight();
    
private:
    float * _Channels[3];
    int _Width;
    int _Height;
};

#endif

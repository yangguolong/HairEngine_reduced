//
//  SurfaceModel.cpp
//  OBJModel
//
//  Created by yangguolong on 13-6-25.
//  Copyright (c) 2013年 yangguolong. All rights reserved.
//

#include "SurfaceModel.h"
#include "HalfFloat.h"
#include "ImageUtility.h"
#include <stdio.h>
#include <stdlib.h>


#include <android/log.h>
#define LOG_TAG "SurfaceModel"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define QUADSIZE 5

struct HalfPoint {
    unsigned short xyIndex;
    short          zFactor;
};

struct FloatPoint{
    float x;
    float y;
    float z;
};

SurfaceModel::SurfaceModel()
{
    _SortedVertexNum = 0;
    _AONum = 0;
    _VertexDataArray = NULL;
    _AODataArray = NULL;

    _DisplacementX = 0;
    _DisplacementY = 0;
    _DisplacementZ = 0;
}

SurfaceModel::~SurfaceModel()
{
    resetModel();
}

bool SurfaceModel::loadModelFromFile(const char * surfaceModelFilePath, const char * transformDataFilePath, const char * aoFilePath)
{
    if(surfaceModelFilePath == NULL || transformDataFilePath == NULL)
        return false;
    
    FILE * transformFile = fopen(transformDataFilePath, "r");
    if(!transformFile)
    {
        LOGI("Failed to open hair transform data file.\n");
        return false;
    }

    /*
    for(int i=0; i<9;i++)
        fscanf(transformFile, "%f", &_TransformMatrix.rotation[i]);
    for(int i=0; i<3; i++)
        fscanf(transformFile, "%f", &_TransformMatrix.positionTrans[i]);
    fscanf(transformFile, "%f", &_TransformMatrix.scaleFactor);

    for(int i=0; i<4; i++)
          fscanf(transformFile, "%f", &_DisplacementX);
    fscanf(transformFile, "%f", &_DisplacementY);
    fscanf(transformFile, "%f", &_DisplacementZ);
	*/

    //dynamic read for old and new hair version
    std::vector<float> transformData;
    while(!feof(transformFile))
    {
    	float temp = 0;
    	fscanf(transformFile, "%f", &temp);
    	transformData.push_back(temp);
    }

    //LOGI("%d\n", transformData.size());
    //19+1, 1 zero eof
    if(transformData.size() == 20)
    {
    	for(int i=0; i<9;i++)
			_TransformMatrix.rotation[i] = transformData.at(i);
		for(int i=0; i<3; i++)
			_TransformMatrix.positionTrans[i] = transformData.at(i+9);
		_TransformMatrix.scaleFactor = transformData.at(12);
		_DisplacementX = transformData.at(16);
		_DisplacementY = transformData.at(17);
		_DisplacementZ = transformData.at(18);
    }
    else if (transformData.size() == 17)
    {
    	for(int i=0; i<9;i++)
			_TransformMatrix.rotation[i] = transformData.at(i);
		for(int i=0; i<3; i++)
			_TransformMatrix.positionTrans[i] = transformData.at(i+9);
		_TransformMatrix.scaleFactor = transformData.at(12);
    }
    //LOGI("model load %f, %f, %f\n", _DisplacementX, _DisplacementY, _DisplacementZ);

    fclose(transformFile);

//    reading vertex that is (float ,float, float)
//    FILE * surfaceModelFile = fopen(surfaceModelFilePath, "rb");
//    if(!surfaceModelFile)
//    {
//        printf("Failed to open hair modelfile.\n");
//        resetModel();
//        return false;
//    }
//    fread(&_SortedVertexNum, sizeof(unsigned int), 1, surfaceModelFile);
//    //fread(&_BoundBox, sizeof(BoundBox), 1, surfaceModelFile);
//    
//    unsigned int unitLength =  SurfaceModel::getVertexUnitSize();
//    _VertexDataArray = (char *)calloc(_SortedVertexNum,unitLength);
//    if(!_VertexDataArray)
//    {
//        printf("Failed to allocate memory for hair vertexs.\n");
//        fclose(surfaceModelFile);
//        resetModel();
//        return false;
//    }
//    fread(_VertexDataArray, SurfaceModel::getVertexUnitSize(), _SortedVertexNum, surfaceModelFile);
//    fclose(surfaceModelFile);
    
    if(!loadSUFDataWithXYPostionIndex(surfaceModelFilePath))
    {
        resetModel();
        return false;
    }
    
    if(aoFilePath)
    {
        FILE * aoFile = fopen(aoFilePath, "rb");
        if(!aoFile)
        {
            printf("Failed to open hair ao data file.\n");
            return false;
        }
        fread(&_AONum, sizeof(int), 1, aoFile);
        _AODataArray = new float[_AONum];
        if(!_AODataArray)
        {
            printf("Failed to allocate memory for origin hair ao.\n");
            fclose(aoFile);
            return false;
        }
        fread(_AODataArray, sizeof(float), _AONum, aoFile);
        fclose(aoFile);
    }
    
    return true;
}

bool SurfaceModel::loadSUFDataWithXYPostionIndex(const char * surfaceModelFilePath)
{
    FILE * surfaceModelFile = fopen(surfaceModelFilePath, "rb");
    if(!surfaceModelFile)
    {
        printf("Failed to open hair modelfile.\n");
        return false;
    }
    //_SortedVertexNum  _VertexDataArray   SurfaceModel::getVertexUnitSize()
    
    int width, height, quadSize;
    int frontNum, midNum, backNum, totalNum;
    float zMin, zMax, zLength;
    HalfPoint * halfPointsArray;
    
    //step 1
    fread(&width, sizeof(int), 1, surfaceModelFile);
    fread(&height, sizeof(int), 1, surfaceModelFile);
    fread(&quadSize, sizeof(int), 1, surfaceModelFile);
    
    //step 2
    fread(&frontNum, sizeof(int), 1, surfaceModelFile);
    fread(&midNum, sizeof(int), 1, surfaceModelFile);
    fread(&backNum, sizeof(int), 1, surfaceModelFile);
    totalNum = frontNum + midNum + backNum;
    
    //step 3
    fread(&zMin, sizeof(float), 1, surfaceModelFile);
    fread(&zMax, sizeof(float), 1, surfaceModelFile);
    zLength = zMax - zMin;
    
    //step 4
    halfPointsArray = (HalfPoint *)calloc(totalNum, sizeof(HalfPoint));
    if(!halfPointsArray)
    {
        printf("Failed to allocate memory for hair vertexs.\n");
        fclose(surfaceModelFile);
        return false;
    }
    fread(halfPointsArray, sizeof(HalfPoint), totalNum, surfaceModelFile);
    
    //translate HalfPoint to FloatPoint
    _SortedVertexNum = totalNum;
    _VertexDataArray = (char *)calloc(_SortedVertexNum,SurfaceModel::getVertexUnitSize());
    if(!_VertexDataArray)
    {
        printf("Failed to allocate memory for hair vertexs.\n");
        free(halfPointsArray);
        fclose(surfaceModelFile);
        return false;
    }
    
    FloatPoint * floatPointsArray = (FloatPoint *)_VertexDataArray;
    int PointsPerLine = width / quadSize + 1;
    float xVal, yVal, zVal;
    int arrayIndex, xIndex, yIndex;
    float zFactor;
    
    //process back surface
    for(int i=0; i<backNum; i++)
    {
        //for x y
        arrayIndex = (int)halfPointsArray[i].xyIndex;
        yIndex = arrayIndex / PointsPerLine;
        xIndex = arrayIndex % PointsPerLine;
        
        xVal = xIndex * quadSize + 0.5f + width; // for back surface
        yVal = yIndex * quadSize + 0.5f;
        
        //for z
        halfp2singles(&zFactor, &(halfPointsArray[i].zFactor), 1);
        zVal = zMin + zLength * zFactor;
        
        floatPointsArray[i].x = xVal;
        floatPointsArray[i].y = yVal;
        floatPointsArray[i].z = zVal;
    }
    
    //process mid and front surface
    for(int i=backNum; i<_SortedVertexNum; i++)
    {
        //for x y
        arrayIndex = (int)halfPointsArray[i].xyIndex;
        yIndex = arrayIndex / PointsPerLine;
        xIndex = arrayIndex % PointsPerLine;
        
        xVal = xIndex * quadSize + 0.5f;
        yVal = yIndex * quadSize + 0.5f;
        
        //for z
        halfp2singles(&zFactor, &(halfPointsArray[i].zFactor), 1);
        zVal = zMin + zLength * zFactor;
        
        floatPointsArray[i].x = xVal;
        floatPointsArray[i].y = yVal;
        floatPointsArray[i].z = zVal;
    }
    
    free(halfPointsArray);
    
    return true;
}

bool SurfaceModel::loadModelFromDepthFile(const char * depthImagePath, const char * depthInfoFilePath, const char * transformDataFilePath)
{
    if(depthImagePath == NULL || depthInfoFilePath == NULL || transformDataFilePath == NULL)
        return false;
    
    FILE * transformFile = fopen(transformDataFilePath, "r");
    if(!transformFile)
    {
        printf("Failed to open hair transform data file.\n");
        return false;
    }

    //original file read method
    /*
    for(int i=0; i<9;i++)
        fscanf(transformFile, "%f", &_TransformMatrix.rotation[i]);
    for(int i=0; i<3; i++)
        fscanf(transformFile, "%f", &_TransformMatrix.positionTrans[i]);
    fscanf(transformFile, "%f", &_TransformMatrix.scaleFactor);

    for(int i=0; i<4; i++)
            fscanf(transformFile, "%f", &_DisplacementX);
    fscanf(transformFile, "%f", &_DisplacementY);
    fscanf(transformFile, "%f", &_DisplacementZ);
*/
    //dynamic read for old and new hair version
    std::vector<float> transformData;
	while(!feof(transformFile))
	{
		float temp = 0;
		fscanf(transformFile, "%f", &temp);
		transformData.push_back(temp);
	}

	LOGI("data size %d\n", transformData.size());

	if(transformData.size() == 20)
	{
		for(int i=0; i<9;i++)
			_TransformMatrix.rotation[i] = transformData.at(i);
		for(int i=0; i<3; i++)
			_TransformMatrix.positionTrans[i] = transformData.at(i+9);
		_TransformMatrix.scaleFactor = transformData.at(12);
		_DisplacementX = transformData.at(16);
		_DisplacementY = transformData.at(17);
		_DisplacementZ = transformData.at(18);
	}
	else if(transformData.size() == 17)
	{
		for(int i=0; i<9;i++)
			_TransformMatrix.rotation[i] = transformData.at(i);
		for(int i=0; i<3; i++)
			_TransformMatrix.positionTrans[i] = transformData.at(i+9);
		_TransformMatrix.scaleFactor = transformData.at(12);
	}

    //LOGI("depth load DisplacementX %f, DisplacementY %f, DisplacementZ %f\n", _DisplacementX, _DisplacementY, _DisplacementZ);


    fclose(transformFile);

    if(!loadSUFDataWithDepthImage(depthImagePath, depthInfoFilePath))
    {
        resetModel();
        return false;
    }
    
    return true;
}

bool SurfaceModel::loadSUFDataWithDepthImage(const char * depthImgPath, const char * depthInfoFilePath)
{
    //load depth info file
    float zMinArray[3];
    float zMaxArray[3];
    
    FILE * infoFile = fopen(depthInfoFilePath, "r");
    if(!infoFile)
    {
        printf("Failed to load depthInfo file(%s).\n", depthInfoFilePath);
        return false;
    }
    for(int i=0; i<3; i++)
        fscanf(infoFile, "%f %f", zMinArray + i, zMaxArray + i);
    fclose(infoFile);
    
    //load depth image
    int width, height, totalPixelNum;
    unsigned char * rgbData = NULL;
    
    time_t startTime, endTime;
    startTime = clock();
    //rgbData = getJpgImageDataAndInfoForImagePath(depthImgPath, &width, &height, 2); // return format is 2, continue R channnel buffer and then G, B
    rgbData = getPngImageDataAndInfoForImagePath(depthImgPath, &width, &height, 2);
    endTime = clock();
    printf("read png: %f\n", ((float)(endTime - startTime)) / CLOCKS_PER_SEC);
    if(!rgbData)
    {
        printf("Faile to load depth image(%s).\n", depthImgPath);
        return false;
    }
    totalPixelNum = width * height;
    
    //evaluate triangle strips for front mid and back
    MeshVertex * surfaceMeshArray[3];
    int meshVertexNumArray[3];
    for(int i=0; i<3; i++)
    {
        surfaceMeshArray[i] = createTriangleStripFromDepthCullDepthZeroIfAllPointCenterCustomQuadSize(rgbData + totalPixelNum * i,
                                                                                                      width,
                                                                                                      height,
                                                                                                      QUADSIZE,
                                                                                                      meshVertexNumArray[i]);
    }
    delete [] rgbData;
    
    //gen _VertexDataArray
    _SortedVertexNum = 0;
    for(int i=0; i<3; i++)
        _SortedVertexNum += meshVertexNumArray[i];
    _VertexDataArray = (char *)calloc(_SortedVertexNum, SurfaceModel::getVertexUnitSize());
    
    MeshVertex * meshVertexArray = (MeshVertex *)_VertexDataArray;
    int arrayIndex = 0;
    for(int i=2; i>=0; i--)
    {
        //add from back to front
        memcpy(meshVertexArray + arrayIndex, surfaceMeshArray[i], sizeof(MeshVertex) * meshVertexNumArray[i]);
        arrayIndex += meshVertexNumArray[i];
    }
    for(int i=0; i<3; i++)
        delete [] surfaceMeshArray[i];
    
    //process z and x of _VertexDataArray
    arrayIndex = 0;
    MeshVertex * surfaceMeshPtr;
    for(int i=2; i>=0; i--)
    {
        float zMin = zMinArray[i];
        float zMax = zMaxArray[i];
        float zLength = zMax - zMin;
        int meshVertexNum = meshVertexNumArray[i];
        surfaceMeshPtr = meshVertexArray + arrayIndex;
        
        for(int j=0; j<meshVertexNum; j++)
        {
            surfaceMeshPtr[j].position[2] = (surfaceMeshPtr[j].position[2] / 254.0f) * zLength + zMin;
        }
        
        arrayIndex += meshVertexNum;
    }
    
    int backVertexNum = meshVertexNumArray[2];
    surfaceMeshPtr = meshVertexArray;
    for(int i=0; i<backVertexNum; i++)
        surfaceMeshPtr[i].position[0] += width;
    
    return true;
}

MeshVertex * SurfaceModel::createTriangleStripFromDepthCullDepthZeroIfAllPointCenterCustomQuadSize(const unsigned char * depthDataArray, int width, int height, int quadSize, int & meshVertexNum)
{
	//行末和高末不足quad size的被忽略
	//定义：非空点——depth 不为0的点
    const int PointsPerWidth = width / quadSize + 1;
    const int PointsPerHeight = height / quadSize + 1;
    
	MeshVertex * rstMeshVertexArray = new MeshVertex[(PointsPerWidth * PointsPerHeight * 4)];
	meshVertexNum = 0;
	bool isFull[2];
	bool isUsed[2];
	bool stripGenedForLine = false;
	for(int i=quadSize;i<height;i += quadSize)
	{
		//init state
		isFull[0] = isFull[1] = false;
		isUsed[0] = isUsed[1] = true;
		stripGenedForLine = false;
		// for start two vertex (0.5, i+0.5) and (0.5,(i-1) + 0.5)
		int tmp0 = i;
		int tmp1 = i - quadSize;
		unsigned char tmpDepth0 = depthDataArray[tmp0 * width];
		unsigned char tmpDepth1 = depthDataArray[tmp1 * width];
		if( (tmpDepth0 != 0xff) || (tmpDepth1 != 0xff) )
		{
			int tmpDepth = ( (tmpDepth0 != 0xff) ?tmpDepth0 : tmpDepth1);
			if(tmpDepth == 0xff)
				printf("tag1\n");
			rstMeshVertexArray[meshVertexNum++] = MeshVertex(0.5, tmp0 + 0.5, (float)tmpDepth); // depth mark
		}
        
		for(int j=0;j<width;j += quadSize)
		{
			//for vertex (j + 0.5, i+0.5) and (j+0.5,(i-1) + 0.5)
			for(int k = 0;k>= -1;k--)
			{
				int ii = i + k * quadSize;
				//for vertex (j + 0.5,ii + 0.5);
				unsigned char curDepth = depthDataArray[ii * width + j];
				
				if(curDepth != 0xff)
				{
					stripGenedForLine = true;
                    
					// a full point
					if(isFull[0] || isFull[1])
					{
						//前两个点中有非空点
						if(curDepth == 0xff)
							printf("tag2\n");
						rstMeshVertexArray[meshVertexNum++] = MeshVertex(j + 0.5, ii + 0.5, (float)curDepth);
						isFull[0] = isFull[1];
						isFull[1] = true;
						isUsed[0] = isUsed[1];
						isUsed[1] = true;
					}
					else
					{
						//前两个点中无非空点
						if( isUsed[0] && isUsed[1] )
						{
							//前两个空点都被使用
							if(curDepth == 0xff)
								printf("tag3\n");
							rstMeshVertexArray[meshVertexNum++] = MeshVertex(j + 0.5, ii + 0.5, (float)curDepth);
							isFull[0] = isFull[1];
							isFull[1] = true;
							isUsed[0] = isUsed[1];
							isUsed[1] = true;
						}
						else if( isUsed[0] && (!isUsed[1]) )
						{
						    //次临点被使用 最临点未被使用
							int x1, y1; //最邻近点坐标
							if(k == 0)
							{
								x1 = j - quadSize;
								y1 = i - quadSize;
							}
							else
							{
								x1 = j;
								y1 = i;
							}
							if(curDepth == 0xff)
								printf("tag4\n");
							rstMeshVertexArray[meshVertexNum++] = MeshVertex(x1 + 0.5, y1 + 0.5, (float)curDepth); //depth mark
							rstMeshVertexArray[meshVertexNum++] = MeshVertex(j + 0.5, ii + 0.5, (float)curDepth);
							isFull[0] = isFull[1];
							isFull[1] = true;
							isUsed[0] = true;
							isUsed[1] = true;
						}
						else if( (!isUsed[0]) && (!isUsed[1]) )
						{
							//前两个空点都未被使用
							int x1, y1; //最邻近点坐标
							int x0, y0; //次临近点坐标
							if(k == 0)
							{
								x1 = j - quadSize;
								y1 = i - quadSize;
								x0 = j - quadSize;
								y0 = i;
							}
							else
							{
								x1 = j;
								y1 = i;
								x0 = j - quadSize;
								y0 = i - quadSize;
							}
                            
							//给上一段triangle strip添后缀
							if(meshVertexNum > 0)
							{
								rstMeshVertexArray[meshVertexNum] = rstMeshVertexArray[meshVertexNum - 1];
								meshVertexNum++;
							}
							
							//给新的triangle strip填前缀
							if(curDepth == 0xff)
								printf("tag5\n");
							rstMeshVertexArray[meshVertexNum++] = MeshVertex(x0 + 0.5, y0 + 0.5, (float)curDepth); //depth mark
							rstMeshVertexArray[meshVertexNum++] = MeshVertex(x0 + 0.5, y0 + 0.5, (float)curDepth); //depth mark
                            
							rstMeshVertexArray[meshVertexNum++] = MeshVertex(x1 + 0.5, y1 + 0.5, (float)curDepth); //depth mark
							rstMeshVertexArray[meshVertexNum++] = MeshVertex(j + 0.5, ii + 0.5, (float)curDepth);
							isFull[0] = isFull[1];
							isFull[1] = true;
							isUsed[0] = true;
							isUsed[1] = true;
						}
						else
						{
							//次临点未被使用 最临点被使用
							printf("Opps, it should never enter this case.\n");
						}
					}
				}
				else
				{
					//a empty point
					if(isFull[0] || isFull[1])
					{
						stripGenedForLine = true;
                        
						//前两个点中有非空点
						unsigned char fullDepth;
						int x1, y1; //最邻近点坐标
						int x0, y0; //次临近点坐标
						if(k == 0)
						{
							x1 = j - quadSize;
							y1 = i - quadSize;
							x0 = j - quadSize;
							y0 = i;
						}
						else
						{
							x1 = j;
							y1 = i;
							x0 = j - quadSize;
							y0 = i - quadSize;
						}
                        
						if(isFull[1])
							fullDepth = depthDataArray[y1 * width + x1];
						else
							fullDepth = depthDataArray[y0 * width + x0];
						
						if(fullDepth == 0xff)
						{
							printf("tag6\n");
							printf("cur (%d %d)	1 (%d %d)	0 (%d %d)\n", j, i, x1, y1, x0, y0);
							printf("1: %d\n2: %d\n", depthDataArray[y1 * width + x1], depthDataArray[y0 * width + x0]);
						}
						rstMeshVertexArray[meshVertexNum++] = MeshVertex(j + 0.5, ii + 0.5, (float)fullDepth); //depth mark
						isFull[0] = isFull[1];
						isFull[1] = false;
						isUsed[0] = isUsed[1];
						isUsed[1] = true;
					}
					else
					{
						//前两个点中无非空点
						isFull[0] = isFull[1];
						isFull[1] = false;
						isUsed[0] = isUsed[1];
						isUsed[1] = false;
					}
				}
			}
		}
        
		//给每行末端加后缀
		if(stripGenedForLine)
		{
			if(rstMeshVertexArray[meshVertexNum - 1].position[2] == 255.f)
			{
				printf("tag7\n");
			}
            
			rstMeshVertexArray[meshVertexNum] = rstMeshVertexArray[meshVertexNum - 1];
			meshVertexNum++;
		}
	}
    
	return rstMeshVertexArray;
    
}

void SurfaceModel::resetModel()
{
    _SortedVertexNum = 0;
    _AONum = 0;
    
    if(_VertexDataArray)
    {
        delete [] _VertexDataArray;
        _VertexDataArray = NULL;
    }
    if(_AODataArray)
    {
        delete [] _AODataArray;
        _AODataArray = NULL;
    }
}

void SurfaceModel::clearVertexDataArray()
{
    if(_VertexDataArray)
    {
        delete [] _VertexDataArray;
        _VertexDataArray = NULL;
    }
}

void SurfaceModel::clearAODataArray()
{
    if(_AODataArray)
    {
        delete [] _AODataArray;
        _AODataArray = NULL;
    }
}

char * SurfaceModel::getVertexDataArray()
{
    return _VertexDataArray;
}

float * SurfaceModel::getAODataArray()
{
    return _AODataArray;
}

TransformMatrix * SurfaceModel::getTransformMatrix()
{
    return &_TransformMatrix;
}

unsigned int SurfaceModel::getSortedVertexNum()
{
    return _SortedVertexNum;
}

int SurfaceModel::getAONum()
{
    return _AONum;
}

unsigned int SurfaceModel::getVertexUnitSize()
{
    return sizeof(float) * 3;
}

float SurfaceModel::getDisplacementX()
{
	return _DisplacementX;
}

float SurfaceModel::getDisplacementY()
{
	return _DisplacementY;
}

float SurfaceModel::getDisplacementZ()
{
	return _DisplacementZ;
}

//
//  HeadDeformer.cpp
//  Hair Preview
//
//  Created by yangguolong on 13-6-29.
//  Copyright (c) 2013ๅนด yangguolong. All rights reserved.
//

#include "HeadDeformer.h"
#include <stdio.h>
#include <time.h>
#include "glm.hpp"
#include "type_ptr.hpp"
#include "matrix_transform.hpp"
#include "HalfFloat.h"

//#define ExpressionPeriod       36
//#define ModifierPeriod         12
//#define MaxExpressionWeight    1.0f
//#define MaxModifierWeight      0.6f

#define ExpressionPeriod       24
#define ModifierPeriod         24
#define MaxExpressionWeight    1.8f
#define MaxModifierWeight      0.9f

#include <android/log.h>
#define LOG_TAG "HeadDeformer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

HeadDeformer::HeadDeformer()
{
    _CurrentExpressionID = 0;
    _CurrentModifierID = 0;
    _CurrentExpressionWeight = 0.0f;
    _CurrentModifierWeight = 0.0f;
    
    _TransformedExpressions = NULL;
    _TransformedModifiers = NULL;
    _ExpressionNum = 0;
    _ModifierNum = 0;
    _VertexNumPerExpression = 0;
    _VertexNumPerModifier = 0;
    
    resetDeformer();
}

HeadDeformer::~HeadDeformer()
{
    clearExpressionData();
}

void HeadDeformer::clearExpressionData()
{
    _CurrentExpressionID = 0;
    _CurrentModifierID = 0;
    _CurrentExpressionWeight = 0.0f;
    _CurrentModifierWeight = 0.0f;
    
    if(_TransformedExpressions)
    {
        delete [] _TransformedExpressions;
        _TransformedExpressions = NULL;
    }
    if(_TransformedModifiers)
    {
        delete [] _TransformedModifiers;
        _TransformedModifiers = NULL;
    }
    
    _ExpressionNum = 0;
    _ModifierNum = 0;
    _VertexNumPerExpression = 0;
    _VertexNumPerModifier = 0;
}

bool HeadDeformer::loadExpressionData(const char * expressionFilePath, const char * modifierFilePath, const char * transformFilePath)
{
    if(expressionFilePath == NULL || modifierFilePath == NULL || transformFilePath == NULL)
        return false;
    
    int totalVertexNum;
    
    FILE * expressionFile = fopen(expressionFilePath, "rb");
    if(!expressionFile)
    {
        printf("Failed to load expression file %s.\n", expressionFilePath);
        clearExpressionData();
        return false;
    }
    fread(&_VertexNumPerExpression, sizeof(int), 1, expressionFile);
    fread(&_ExpressionNum, sizeof(int), 1, expressionFile);
    totalVertexNum = _VertexNumPerExpression * _ExpressionNum;
    _TransformedExpressions = new MVec3f[totalVertexNum];
    if(!_TransformedExpressions)
    {
        printf("Failed to alloc memory for transformed expression array.\n");
        fclose(expressionFile);
        clearExpressionData();
        return false;
    }
    short * tmpExpressionShort = new short[totalVertexNum * 3];
    fread(tmpExpressionShort, sizeof(short), totalVertexNum * 3, expressionFile);
    halfp2singles(_TransformedExpressions, tmpExpressionShort, totalVertexNum * 3);
    delete [] tmpExpressionShort;
    fclose(expressionFile);
    
    FILE * modifierFile = fopen(modifierFilePath, "rb");
    if(!modifierFile)
    {
        printf("Failed to load modifier file %s.\n", modifierFilePath);
        clearExpressionData();
        return false;
    }
    fread(&_VertexNumPerModifier, sizeof(int), 1, modifierFile);
    fread(&_ModifierNum, sizeof(int), 1, modifierFile);
    totalVertexNum = _VertexNumPerModifier * _ModifierNum;
    _TransformedModifiers = new MVec3f[totalVertexNum];
    if(!_TransformedModifiers)
    {
        printf("Failed to alloc memory for transformed modifier array.\n");
        fclose(modifierFile);
        clearExpressionData();
        return false;
    }
    short * tmpModifierShort = new short[totalVertexNum * 3];
    fread(tmpModifierShort, sizeof(short), totalVertexNum * 3, modifierFile);
    halfp2singles(_TransformedModifiers, tmpModifierShort, totalVertexNum * 3);
    delete [] tmpModifierShort;
    fclose(modifierFile);
    
//    FILE * rstExpFile = fopen(expressionFilePath, "wb");
//    int rstTotalExpVertexNum = 2 * _VertexNumPerExpression;
//    MVec3f * tmpExpVertex = new MVec3f[rstTotalExpVertexNum];
//    memcpy(tmpExpVertex, _TransformedExpressions + 3 * _VertexNumPerExpression, sizeof(MVec3f) * _VertexNumPerExpression);
//    memcpy(tmpExpVertex + _VertexNumPerExpression, _TransformedExpressions + 4 * _VertexNumPerExpression, sizeof(MVec3f) * _VertexNumPerExpression);
//    
//    short * rstExpVertex = new short[rstTotalExpVertexNum * 3];
//    singles2halfp(rstExpVertex, tmpExpVertex, rstTotalExpVertexNum * 3);
//    
//    fwrite(&_VertexNumPerExpression, sizeof(int), 1, rstExpFile);
//    int rstExpNumber = 2;
//    fwrite(&rstExpNumber, sizeof(int), 1, rstExpFile);
//    fwrite(rstExpVertex, sizeof(short), rstTotalExpVertexNum * 3, rstExpFile);
//    
//    delete [] tmpExpVertex;
//    delete [] rstExpVertex;
//    
//    fclose(rstExpFile);
//    
//    FILE * rstModFile = fopen(modifierFilePath, "wb");
//    
//    short * rstModVertex = new short[_VertexNumPerModifier * _ModifierNum * 3];
//    singles2halfp(rstModVertex, _TransformedModifiers, _VertexNumPerModifier * _ModifierNum * 3);
//    
//    fwrite(&_VertexNumPerModifier, sizeof(int), 1, rstModFile);
//    fwrite(&_ModifierNum, sizeof(int), 1, rstModFile);
//    fwrite(rstModVertex, sizeof(short), _VertexNumPerModifier * _ModifierNum * 3, rstModFile);
//    
//    delete [] rstModVertex;
//    
//    fclose(rstModFile);
    
    
    FILE * transformFile = fopen(transformFilePath, "r");
    if(!transformFile)
    {
        printf("Failed to load transform file %s.\n", transformFilePath);
        clearExpressionData();
        return false;
    }
    TransformMatrix matrixData;
    for(int i=0; i<9;i++)
        fscanf(transformFile, "%f", &matrixData.rotation[i]);
    for(int i=0; i<3; i++)
        fscanf(transformFile, "%f", &matrixData.positionTrans[i]);
    fscanf(transformFile, "%f", &matrixData.scaleFactor);
    fclose(transformFile);
    
    float * rotation = matrixData.rotation;
    float tmpMat[16];
    tmpMat[0] = rotation[0];    tmpMat[1] = rotation[1];    tmpMat[2] = rotation[2];    tmpMat[3] = 0.0f;
    tmpMat[4] = rotation[3];    tmpMat[5] = rotation[4];    tmpMat[6] = rotation[5];    tmpMat[7] = 0.0f;
    tmpMat[8] = rotation[6];    tmpMat[9] = rotation[7];    tmpMat[10] = rotation[8];   tmpMat[11] = 0.0f;
    tmpMat[12] = 0.0f;          tmpMat[13] = 0.0f;          tmpMat[14] = 0.0f;          tmpMat[15] = 1.0f;
    glm::mat4x4 transMatrix = glm::make_mat4x4(tmpMat);
    
    glm::vec4 tmpVec;
    totalVertexNum = _VertexNumPerExpression * _ExpressionNum;
    for(int i=0; i<totalVertexNum; i++)
    {
        tmpVec = glm::vec4(_TransformedExpressions[i].x, _TransformedExpressions[i].y, _TransformedExpressions[i].z, 1.0f);
        tmpVec = transMatrix * tmpVec;
        
        _TransformedExpressions[i] = MVec3f(tmpVec.x, tmpVec.y, tmpVec.z);
    }
    totalVertexNum = _VertexNumPerModifier * _ModifierNum;
    for(int i=0; i<totalVertexNum; i++)
    {
        tmpVec = glm::vec4(_TransformedModifiers[i].x, _TransformedModifiers[i].y, _TransformedModifiers[i].z, 1.0f);
        tmpVec = transMatrix * tmpVec;
        
        _TransformedModifiers[i] = MVec3f(tmpVec.x, tmpVec.y, tmpVec.z);
    }
    
    LOGI("expression num: %d\n", _ExpressionNum);
    LOGI("modifier num: %d\n", _ModifierNum);

    return true;
}

void HeadDeformer::runDeform(const vector<OBJVertex> & inVertexes, vector<OBJVertex> & outVertexes)
{
    float expressionWeight = _CurrentExpressionWeight * MaxExpressionWeight;
    float modifierWeight = _CurrentModifierWeight * MaxModifierWeight;
    
    MVec3f * expressionsSet = _TransformedExpressions + _CurrentExpressionID * _VertexNumPerExpression;
    MVec3f * modifierSet = _TransformedModifiers + _CurrentModifierID * _VertexNumPerModifier;

    OBJVertex originalVertex, finalVertex;
    MVec3f expressionVector, modifierVector;
    int vertexNum = inVertexes.size();
    for (int vertexI = 0; vertexI<vertexNum; vertexI++)
    {
        originalVertex = inVertexes[vertexI];
        expressionVector = expressionsSet[vertexI];
        modifierVector = modifierSet[vertexI];
        for(int i=0; i<3; i++)
        {
        	if(_CurrentExpressionID != -1 && _CurrentExpressionID >=0 && _CurrentExpressionID <=1)
        		finalVertex.position[i] = originalVertex.position[i] + expressionVector.position[i] * expressionWeight;
        	if(_CurrentModifierID != -1 && _CurrentModifierID >=0 && _CurrentModifierID <=12)
        		finalVertex.position[i] = originalVertex.position[i] + modifierVector.position[i] * modifierWeight;
        }
            outVertexes[vertexI] = finalVertex;
    }
    
    updateCounter();
}

void HeadDeformer::resetDeformer()
{
    _Counter = -1;
    _CurrentExpressionID = 0;
    _CurrentModifierID = 0;
    _CurrentExpressionWeight = 0.0f;
    _CurrentModifierWeight = 0.0f;
    //srand(time(0));
}


//expression num 2 0 anger 1 smile close
//modifier num 13 0 left eye blink 1 right eye blink 2 left eye brow move
void HeadDeformer::updateCounter()
{
    _Counter = (_Counter + 1) % (ExpressionPeriod * ModifierPeriod * 4);
    int expressionTime = _Counter % (ExpressionPeriod * 4);
    int modifierTime = _Counter % (ModifierPeriod * 4);
    
    /*
    if(expressionTime == 0)
    {
        _CurrentExpressionID = 0;//rand() % _ExpressionNum;
    }
    if(modifierTime == 0)
        _CurrentModifierID = 3;//rand() % _ModifierNum;
    */

    if (expressionTime < ExpressionPeriod)
		_CurrentExpressionWeight = (1.0f - cos((float)expressionTime / ExpressionPeriod * MC_PI)) * 0.5f;
	else if (expressionTime < 2.0f * ExpressionPeriod)
		_CurrentExpressionWeight = 1.0f;
	else if (expressionTime < 3.0f * ExpressionPeriod)
		_CurrentExpressionWeight = (1.0f - cos((3.0f - (float)expressionTime / ExpressionPeriod) * MC_PI)) * 0.5f;
	else
		_CurrentExpressionWeight = 0.0f;

    if (modifierTime < ModifierPeriod)
		_CurrentModifierWeight = (1.0f - cos((float)modifierTime / ModifierPeriod * MC_PI)) * 0.5f;
	else if (modifierTime < 2.0f * ModifierPeriod)
		_CurrentModifierWeight = 1.0f;
	else if (modifierTime < 3.0f * ModifierPeriod)
		_CurrentModifierWeight = (1.0f - cos((3.0f - (float)modifierTime / ModifierPeriod) * MC_PI)) * 0.5f;
	else
		_CurrentModifierWeight = 0.0f;
}

void HeadDeformer::setExpressionID(int id)
{
	_CurrentExpressionID = id;
}

void HeadDeformer::setModifierID(int id)
{
	_CurrentModifierID = id;
}

//
//  HeadDeformer.h
//  Hair Preview
//
//  Created by yangguolong on 13-6-29.
//  Copyright (c) 2013ๅนด yangguolong. All rights reserved.
//

#ifndef HeadDeformer_H
#define HeadDeformer_H

#include "ModelCommonData.h"
#include "IHeadDeformer.h"

class HeadDeformer : public IHeadDeformer
{
public:
    HeadDeformer();
    ~HeadDeformer();
    
    bool loadExpressionData(const char * expressionFilePath, const char * modifierFilePath, const char * transformFilePath);
    void clearExpressionData();
    void resetDeformer();
    void runDeform(const vector<OBJVertex> & inVertexes, vector<OBJVertex> & outVertexes);

    void setExpressionID(int id);
    void setModifierID(int id);

private:
    void updateCounter();
    
    int     _CurrentExpressionID;
    int     _CurrentModifierID;
    float   _CurrentExpressionWeight;
    float   _CurrentModifierWeight;
    
    MVec3f * _TransformedExpressions;
    MVec3f * _TransformedModifiers;
    int     _ExpressionNum;
    int     _ModifierNum;
    int     _VertexNumPerExpression;
    int     _VertexNumPerModifier;

    int     _Counter;
};

#endif

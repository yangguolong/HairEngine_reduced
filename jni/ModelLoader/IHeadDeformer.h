#ifndef IHeadDeformer_h__
#define IHeadDeformer_h__
#include <vector>
#include "OBJModel.h"

class IHeadDeformer
{
public:
	virtual ~IHeadDeformer() {}

	virtual bool loadExpressionData(const char * expressionFilePath, const char * modifierFilePath, const char * transformFilePath) = 0;
	virtual void clearExpressionData() = 0;
	virtual void resetDeformer() = 0;
	virtual void runDeform(const vector<OBJVertex> & inVertexes, vector<OBJVertex> & outVertexes) = 0;
	virtual void setExpressionID(int id) = 0;
	virtual void setModifierID(int id) = 0;
};

#endif // IHeadDeformer_h__

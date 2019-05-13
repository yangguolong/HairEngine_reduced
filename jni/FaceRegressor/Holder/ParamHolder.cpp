//	******************************************************
//	ParamHolder
//	Hold the parameters of the whole framework
//	Author: Chen Cao	19/08/2013
//	******************************************************

#include "ParamHolder.h"

//ParamHolder gPH;

//////////////////////////////////////////////////////////////////////////
// <Constructor>
ParamHolder::ParamHolder()
{
	// Initialize the global display
	initGlobalDis();

	// Initialize regression
	initReg();
}
// </Constructor>
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// <Global display>
void ParamHolder::initGlobalDis()
{
	mDisPtSize				= 4.f;
	mDisLineWidth			= 2.f;

	mDisLandLine			= true;
}
// </Global display>
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// <Regression>
void ParamHolder::initReg()
{
	// Display
	mRegDis					= false;
	mRegDisImg				= true;
	mRegDisFaceRect			= false;
	mRegDisLandInit			= false;
	mRegDisLandCur			= false;
	mRegDisSample			= false;
}
// </Regression>
//////////////////////////////////////////////////////////////////////////
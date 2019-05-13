//	******************************************************
//	ParamHolder
//	Hold the parameters of the whole framework
//	Author: Chen Cao	19/08/2013
//	******************************************************

#ifndef PARAM_HOLDER_H
#define PARAM_HOLDER_H

class ParamHolder
{
public:
	ParamHolder						();

	// ----------------------------------------
	// *Global display
	float							mDisPtSize;
	float							mDisLineWidth;
	
	bool							mDisLandLine;

	void initGlobalDis				();

	// ----------------------------------------
	// *Regression
	// Initialize
	void initReg					();

	// Display
	bool							mRegDis;
	bool							mRegDisImg;
	bool							mRegDisFaceRect;
	bool							mRegDisLandInit;
	bool							mRegDisLandCur;
	bool							mRegDisSample;
};

//extern ParamHolder gPH;

#endif
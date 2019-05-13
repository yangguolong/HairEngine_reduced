//	******************************************************
//	DataHolder
//	Hold the global data of the whole framework
//	Author: Chen Cao	19/08/2013
//	******************************************************

#ifndef DATA_HOLDER_H
#define DATA_HOLDER_H

// Include stl
#include <vector>
#include <string>

// Include common
#include "vec.h"
#include "list.h"
#include "ccmatrix.h"

using namespace cc;

// ----------------------------------------
//* Structure
struct sImage
{
	sByte3List						mImgData;
	sFloat3List						mLandList;
};

// ----------------------------------------
// *Global data holder

#define CCLandNum 74
class DataHolder
{
public:
	// ----------------------------------------
	// *Constructor
	DataHolder						();

	// ----------------------------------------
	// Save-load file format
	std::string						mFileFormatStr;
	
	void initFileFormat			();

	// ----------------------------------------
	// *Image & Land
	// Image list
	std::vector<sImage>				mFrmList;
	std::vector<std::string>		mImgNameList;

	int								mCurFrmId;
	sImage*							mCurFrm_P;

	// Image parameters
	myint2							mImgSize;
	myfloat4						mImgCoord;
	myfloat4						mImgRect;
	float							mImgDepth;
	Mat4f							mImgMat;

	void initImg					();

	// Land parameters
	int								mLandNum;
	std::vector<sIntList>			mLandLineIds;
	void initLand					();
};

//extern DataHolder gDH;

#endif
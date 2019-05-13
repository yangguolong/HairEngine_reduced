//	******************************************************
//	Regression Test
//	Face Alignment by Explicit Shape Regression
//	Author: Chen Cao	23/08/2013
//	******************************************************

#ifndef REGRESSION_H
#define REGRESSION_H

// STL
#include <vector>

// OpenCV
#include <opencv2/opencv.hpp>

// MyCommon
#include "vec.h"
#include "list.h"
#include "common.h"

// Brothers
#include "DataHolder.h"
#include "ParamHolder.h"

using namespace cc;

#define RegressorOldLandmarksNum 76

// *Special structures
// Regressor
struct sRegSP
{
	int									mLandId;
	myfloat2							mOffset;
};

struct sRegFernItem
{
	myint2								mSpPairIds;
	float								mThreshold;
};

struct sRegBin
{
	int									mDataCount;
	sFloat2List							mOutput;

	sRegBin()
	{
		mDataCount						= 0;
		mOutput							.clear();
	}
};

struct sRegFern
{
	sInt2List							mSpIdPairList;
	sFloatList							mThresList;

	std::vector<sRegBin>				mBinList;
};

struct sReg
{
	sFloat3List							mSampleList;	// x: landId, y,z: offset

	std::vector<sRegFern>				mFernList;
};


// *Regression Class
class RegTest
{
public:
	
	// *Constructor, destructor and initialize
	RegTest								(const char * dataDir);			// 0: RegTest mReg;
	~RegTest							();
	void initialize						();

	
	// *Load functions
	void loadRegressor					();
	void loadQMRegressor				();	// * 1: general.fr

	
	// *Access functions
	inline void setTestInitNum			( int initNum )	{ mTestInitNum = initNum;	}	// *2: 11
	inline void setTestCurLandsId		( int landsId )	{ mTestCurLandsId = landsId; }
	inline sFloat2List& getAvgLands		()	{ return mAvgLands; }

	void setImgData						( const unsigned char * imgData, int w, int h );			// *3: image data
	cv::Mat& getLandmarks               ();												// *5: Get data
    
	// *Test
	bool testFirstRegress				();												// *4: Regress

	bool testPlaceInitLands				();
	void testSamplePixels				( int regId );
	void testPassReg					( int regId );
	void testMedianResult				();
    
    void test                           ();

private:
	
	// *OpenCV
	void initDetector					();

private:
	
	// *Image
	sByte3List							mImgData;
	myint2								mImgSize;

	
	// *Landmark space
	sFloat2List							mAvgLands;
	std::vector<sFloat2List>			mLandSpace;

	
	// *Regressors
	std::vector<sReg>					mRegList;

	
	// *Test
	int									mTestInitNum;
	int									mTestCurLandsId;	

	std::vector<sFloat2List>			mTestInitLandsList;
	std::vector<sFloat2List>			mTestCurLandsList;

	myfloat4							mTestFaceRect;

	sFloat2List							mTestCurLands;
	Mat3f								mTestImgWarpMat;

	sFloat2List							mTestSamplePosList;
	sFloatList							mTestSampleValList;	

	
	// *OpenCV
	// Image
	cv::Mat								mCVImgMat;

	// Face detector
	cv::CascadeClassifier				mCVFaceDetector;
	bool								mCVFaceDetectorInit;
    
    char                                mDataDir[500];
    cv::Mat                             mFeatures;
};

#endif
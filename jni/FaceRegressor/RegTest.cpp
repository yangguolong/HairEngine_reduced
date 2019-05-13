//	******************************************************
//	Regression Test
//	Face Alignment by Explicit Shape Regression
//	Author: Chen Cao	23/08/2013
//	******************************************************

#include <time.h>
#include <string>
#include "RegTest.h"
#include "HalfFloat.h"

//map result feature points(#76) to regress feature points(#74)
static const int iEnd76ToMid76New[] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26,
	27, 66, 28, 69, 29, 68, 30, 67,
    0,
	31, 73, 32, 70, 33, 71, 34, 72,
    0,
	35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
    46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 62, 63, 64, 65
};

// <Constructor, destructor and initialize>
RegTest::RegTest(const char * dataDir)
{
	// Initialize
    if(!dataDir || strlen(dataDir) <=0)
        printf("invaild dataDir for RegTest.\n");
    else
        strcpy(mDataDir, dataDir);
    
	initialize();
}

RegTest::~RegTest()
{
    if(mFeatures.data)
        mFeatures.release();
}

void RegTest::initialize()
{
	
	// *Landmark space
	mAvgLands						.clear();
	mLandSpace						.clear();

	
	// *Regressors
	mRegList						.clear();

	
	// *Test
	mTestInitNum					= 1;
	mTestCurLandsId					= -1;

	mTestInitLandsList				.clear();
	mTestCurLandsList				.clear();

	mTestFaceRect					.zero();

	mTestCurLands					.clear();
	mTestImgWarpMat					.identity();

	mTestSamplePosList				.clear();
	mTestSampleValList				.clear();
	
	// OpenCV
	mCVFaceDetectorInit				= false;
}

 //<Load functions>
//half float
void RegTest::loadRegressor()
{
	FILE* fp;
    char regressorDataFile[500];
    sprintf(regressorDataFile, "%s/general.rgs", mDataDir);
	fp = fopen(regressorDataFile, "rb" );
	if( !fp ){
		printf( "Error: open %s\n", regressorDataFile);
		return;
	}

	// *Read information
	float version = 0.f;
	fread( &version, sizeof(float), 1, fp );
	printf( "Current reg version: %f\n", version );

	// ----------------------------------------
	// *Read average landmarks and land space
	int landNum = 0;
	fread( &landNum, sizeof(int), 1, fp );
	if( landNum != CCLandNum ){
		printf( "Error: landNum != gDH.mLandNum!\n" );
		fclose( fp );
		return;
	}
    
	std::vector<short> hpAvgLands( 2 * landNum );
	fread( &hpAvgLands[0], sizeof(short), 2 * landNum, fp );
	mAvgLands.resize( landNum );
	for( int landId=0; landId<landNum; landId++ ){
		myfloat2& land = mAvgLands[ landId ];
        
		short hpLandX = hpAvgLands[ 2 * landId + 0 ];
		short hpLandY = hpAvgLands[ 2 * landId + 1 ];
        
		halfp2singles( &land.x, &hpLandX, 1 );
		halfp2singles( &land.y, &hpLandY, 1 );
	}
    
	int shapeNum = 0;
	fread( &shapeNum, sizeof(int), 1, fp );
	if( shapeNum > 0 ){
		mLandSpace.resize( shapeNum );
		for( int i=0; i<shapeNum; i++ ){
			std::vector<short> hpLands( 2 * landNum );
			fread( &hpLands[0], sizeof(short), 2 * landNum, fp );
            
			sFloat2List& lands = mLandSpace[ i ];
			lands.resize( landNum );
			for( int landId=0; landId<landNum; landId++ ){
				myfloat2& land = lands[ landId ];
                
				short hpLandX = hpLands[ 2 * landId + 0 ];
				short hpLandY = hpLands[ 2 * landId + 1 ];
                
				halfp2singles( &land.x, &hpLandX, 1 );
				halfp2singles( &land.y, &hpLandY, 1 );
			}
		}
	}


	// *Regressors
	int regNum = 0;
	fread( &regNum, sizeof(int), 1, fp );
	mRegList.resize( regNum );
	printf( "Load %d regs\n", regNum );

	for( int regId=0; regId<regNum; regId++ ){
		sReg& reg = mRegList[ regId ];

		// Samples
		int spNum = 0;
		fread( &spNum, sizeof(int), 1, fp );
		reg.mSampleList.resize( spNum );
		fread( &reg.mSampleList[0], sizeof(myfloat3), spNum, fp );

		printf( "Load %d samples in %d reg\r", spNum, regId );

		// Ferns
		int fernNum = 0;
		fread( &fernNum, sizeof(int), 1, fp );
		reg.mFernList.resize( fernNum );
		for( int fernId=0; fernId<fernNum; fernId++ ){
			sRegFern& fern = reg.mFernList[ fernId ];

			// Feature indices
			int feaNum = 0;
			fread( &feaNum, sizeof(int), 1, fp );
			fern.mSpIdPairList.resize( feaNum );
			fern.mThresList.resize( feaNum );
			fread( &fern.mSpIdPairList[0], sizeof(myint2), feaNum, fp );
			fread( &fern.mThresList[0], sizeof(float), feaNum, fp );

			// Bin
			int binNum = 0;
			fread( &binNum, sizeof(int), 1, fp );
			fern.mBinList.resize( binNum );
			for( int binId=0; binId<binNum; binId++ ){
				sRegBin& bin = fern.mBinList[ binId ];

				fread( &bin.mDataCount, sizeof(int), 1, fp );

				int landNum = 0;
				fread( &landNum, sizeof(int), 1, fp );
				bin.mOutput.resize( landNum );
//				if( landNum > 0 )
//					fread( &bin.mOutput[0], sizeof(myfloat2), landNum, fp );
                
                if( landNum > 0 ){
                    std::vector<short> hpOutput( landNum * 2 );
                    fread( &hpOutput[0], sizeof(short), landNum * 2, fp );
                    
                    for( int landId=0; landId<landNum; landId++ ){
                        myfloat2& value = bin.mOutput[ landId ];
                        short hpValX = hpOutput[ 2 * landId + 0 ];
                        short hpValY = hpOutput[ 2 * landId + 1 ];
                        
                        halfp2singles( &value.x, &hpValX, 1 );
                        halfp2singles( &value.y, &hpValY, 1 );
                    }
                }
			}
		}
	}

	printf( "\nFinish loading regressor!\n" );
	fclose( fp );
}

/*
//full float
void RegTest::loadRegressor()
{
	FILE* fp;
    char regressorDataFile[500];
    sprintf(regressorDataFile, "%s/general.rgs", mDataDir);
	fp = fopen(regressorDataFile, "rb" );
	if( !fp ){
		printf( "Error: open %s\n", regressorDataFile);
		return;
	}
    
	// *Read information
	float version = 0.f;
	fread( &version, sizeof(float), 1, fp );
	printf( "Current reg version: %f\n", version );
    
	// *Read average landmarks and land space
	int landNum = 0;
	fread( &landNum, sizeof(int), 1, fp );
	if( landNum != CCLandNum ){
		printf( "Error: landNum != gDH.mLandNum!\n" );
		fclose( fp );
		return;
	}
	
	sFloat3List tmpLands( landNum );
	fread( &tmpLands[0], sizeof(myfloat3), landNum, fp );
	mAvgLands.resize( landNum );
	for( int i=0; i<landNum; i++ ){
		myfloat2& land = mAvgLands[ i ];
		const myfloat3& tmpLand = tmpLands[ i ];
		land = myfloat2( tmpLand.x, tmpLand.y );
	}
	printf( "Land num: %d\n", landNum );
    
	int shapeNum = 0;
	fread( &shapeNum, sizeof(int), 1, fp );
	mLandSpace.resize( shapeNum );
	for( int i=0; i<shapeNum; i++ ){
		
		sFloat3List tmpLands( landNum );
		fread( &tmpLands[0], sizeof(myfloat3), landNum, fp );
		
		sFloat2List& lands = mLandSpace[ i ];
		lands.resize( landNum );
		for( int i=0; i<landNum; i++ ){
			myfloat2& land = lands[ i ];
			const myfloat3& tmpLand = tmpLands[ i ];
			land = myfloat2( tmpLand.x, tmpLand.y );
		}
	}
	printf( "Lands space: %d landmarks\n", shapeNum );
    
	// *Regressors
	int regNum = 0;
	fread( &regNum, sizeof(int), 1, fp );
	mRegList.resize( regNum );
	printf( "Load %d regs\n", regNum );
    
	for( int regId=0; regId<regNum; regId++ ){
		sReg& reg = mRegList[ regId ];
        
		// Samples
		int spNum = 0;
		fread( &spNum, sizeof(int), 1, fp );
		reg.mSampleList.resize( spNum );
		fread( &reg.mSampleList[0], sizeof(myfloat3), spNum, fp );
        
		printf( "Load %d samples in %d reg\r", spNum, regId );
        
		// Ferns
		int fernNum = 0;
		fread( &fernNum, sizeof(int), 1, fp );
		reg.mFernList.resize( fernNum );
		for( int fernId=0; fernId<fernNum; fernId++ ){
			sRegFern& fern = reg.mFernList[ fernId ];
            
			// Feature indices
			int feaNum = 0;
			fread( &feaNum, sizeof(int), 1, fp );
			fern.mSpIdPairList.resize( feaNum );
			fern.mThresList.resize( feaNum );
			fread( &fern.mSpIdPairList[0], sizeof(myint2), feaNum, fp );
			fread( &fern.mThresList[0], sizeof(float), feaNum, fp );
            
			// Bin
			int binNum = 0;
			fread( &binNum, sizeof(int), 1, fp );
			fern.mBinList.resize( binNum );
			for( int binId=0; binId<binNum; binId++ ){
				sRegBin& bin = fern.mBinList[ binId ];
                
				fread( &bin.mDataCount, sizeof(int), 1, fp );
                
				int landNum = 0;
				fread( &landNum, sizeof(int), 1, fp );
				bin.mOutput.resize( landNum );
				if( landNum > 0 )
					fread( &bin.mOutput[0], sizeof(myfloat2), landNum, fp );
			}
		}
	}
    
	printf( "\nFinish loading regressor!\n" );
	fclose( fp );
}
 */

void RegTest::loadQMRegressor()
{
	FILE* fp;
	char regressorDataFile[500];
    sprintf(regressorDataFile, "%s/general.fr", mDataDir);
	fp = fopen(regressorDataFile, "rb" );
	if( !fp ){
		printf( "Error: open %s\n", regressorDataFile);
		return;
	}

	int regNum = 0;
	fread( &regNum, sizeof(int), 1, fp );
	mRegList.resize( regNum );

	// Regressors
	for( int regId=0; regId<regNum; regId++ ){
		sReg& reg = mRegList[ regId ];

		// Read samples
		int spNum = 0;
		fread( &spNum, sizeof(int), 1, fp );
		std::vector<sRegSP> spList( spNum );
		fread( &spList[0], sizeof(sRegSP), spNum, fp );		

		reg.mSampleList.resize( spNum );
		for( int spId=0; spId<spNum; spId++ ){
			const sRegSP& sp = spList[ spId ];
			reg.mSampleList[ spId ] = myfloat3( float(sp.mLandId) + 0.5f, 
				sp.mOffset.x, sp.mOffset.y );
		}

		// Read ferns
		int fernNum = 0;
		fread( &fernNum, sizeof(int), 1, fp );
		reg.mFernList.resize( fernNum );
		for( int fernId=0; fernId<fernNum; fernId++ ){
			sRegFern& fern = reg.mFernList[ fernId ];

			// Items
			int itemNum = 5;
			std::vector<sRegFernItem> itemList( itemNum );
			fread( &itemList[0], sizeof(sRegFernItem), itemNum, fp );

			fern.mSpIdPairList.resize( itemNum );
			fern.mThresList.resize( itemNum );
			for( int i=0; i<itemNum; i++ ){
				const sRegFernItem& item = itemList[ i ];
				fern.mSpIdPairList[ i ] = item.mSpPairIds;
				fern.mThresList[ i ] = item.mThreshold;
			}

			// Bin
			int binDeltaNum = 0;
			fread( &binDeltaNum, sizeof(int), 1, fp );
			sFloatList tmpBinData( binDeltaNum );
			fread( &tmpBinData[0], sizeof(float), binDeltaNum, fp );

			int binNum = 32;
			int landNum = binDeltaNum / ( binNum * 2 );
			fern.mBinList.resize( binNum );

			int binOffset = 0;
			for( int binId=0; binId<binNum; binId++ ){
				sRegBin& bin = fern.mBinList[ binId ];
				bin.mOutput.resize( landNum );

				for( int landId=0; landId<landNum; landId++ ){
					myfloat2& offset = bin.mOutput[ landId ];
					offset.x = tmpBinData[ binOffset + 2 * landId ];
					offset.y = tmpBinData[ binOffset + 2 * landId + 1 ];
				}

				binOffset += 2 * landNum;
			}
			unsigned avai = 0;
			fread( &avai, sizeof(unsigned), 1, fp );
		}
	}

	// Neutral pose
	int floatNum = 0;
	fread( &floatNum, sizeof(int), 1, fp );
	int landNum = floatNum / 2;
	mAvgLands.resize( landNum );
	fread( &mAvgLands[0], sizeof(myfloat2), landNum, fp );

	// Landmark space
	fread( &floatNum, sizeof(int), 1, fp );
	int landsNum = floatNum / landNum;
	mLandSpace.resize( landsNum );
	for( int landsId=0; landsId<landsNum; landsId++ ){
		sFloat2List& lands = mLandSpace[ landsId ];
		lands.resize( landNum );
		fread( &lands[0], sizeof(myfloat2), landNum, fp );
	}

	fclose( fp );
}

// <Access functions>
void RegTest::setImgData( const unsigned char * imgData, int w, int h )
{
	mImgSize = myint2( w, h );
	mImgData.resize( mImgSize.x * mImgSize.y );
	
	for( int y=0; y<mImgSize.y; y++ ){
		for( int x=0; x<mImgSize.x; x++ ){
			int pixId = y * mImgSize.x + x;

			myByte3& rgb = mImgData[ pixId ];
			int idOffset = pixId * 3;
			rgb.x = imgData[ idOffset + 0 ];
			rgb.y = imgData[ idOffset + 1 ];
			rgb.z = imgData[ idOffset + 2 ];
		}
	}
}

cv::Mat& RegTest::getLandmarks()
{
	return mFeatures;
}

// <Test>
bool RegTest::testFirstRegress()
{
    printf("I am Regressor ******************************\n");
    
	if( mLandSpace.size() == 0 ){
		printf( "Please firstly load the regressor!\n" );
		return false;
	}

	// Firstly, detect the face and place the initial landmarks
	if( !testPlaceInitLands() )	return false;

	// Then, regress from each initial landmarks
	int landsNum = mTestCurLandsList.size();
	for( int landsId=0; landsId<landsNum; landsId++ ){
		mTestCurLands = mTestCurLandsList[ landsId ];

		int regNum = mRegList.size();		
		for( int regId=0; regId<regNum; regId++ ){
			testSamplePixels( regId );
			testPassReg( regId );
		}

		mTestCurLandsList[ landsId ] = mTestCurLands;
	}

	testMedianResult();
    
    if(mFeatures.data)
        mFeatures.release();
    mFeatures = cv::Mat::zeros(1, RegressorOldLandmarksNum, CV_32FC2);
    for (int featureI = 0; featureI < RegressorOldLandmarksNum; featureI++)
    {
        int index = iEnd76ToMid76New[featureI];
        myfloat2 & point = mTestCurLands[index];
        mFeatures.at<cv::Vec2f>(0, featureI) = cv::Vec2f(point.x, mImgSize.y - 1 - point.y);
    }
    
    return true;
}

bool RegTest::testPlaceInitLands()
{
	// *Set OpenCV data
	sByte3List tmpImgData( mImgSize.x * mImgSize.y );
	for( int y=0; y<mImgSize.y; y++ ){
		for( int x=0; x<mImgSize.x; x++ ){
			int inPixId = y * mImgSize.x + x;
			int outPixId = ( mImgSize.y - y - 1 ) * mImgSize.x + x;
			myByte3& outPix = tmpImgData[ outPixId ];
			const myByte3& inPix = mImgData[ inPixId ];

			outPix.x = inPix.z;
			outPix.y = inPix.y;
			outPix.z = inPix.x;
		}
	}
    
    mCVImgMat = cv::Mat::zeros( mImgSize.y, mImgSize.x, CV_8UC3 );
	memcpy( mCVImgMat.data, &tmpImgData[0], mImgSize.x * mImgSize.y * sizeof(myByte3) );

	// Initialize the face detector
	if( !mCVFaceDetectorInit )	initDetector();

	// Transform 
	cv::Mat imgGray;
	cvtColor( mCVImgMat, imgGray, CV_BGR2GRAY );
    
	equalizeHist( imgGray, imgGray );
    
	// Detect the face
	std::vector<cv::Rect> faces;
	mCVFaceDetector.detectMultiScale( imgGray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, cv::Size( 120, 120 ) );
	int faceNum = faces.size();
    
	cv::Rect faceRect;
	if( faceNum < 1 )	return false;
	else if( faceNum == 1 )	faceRect = faces[ 0 ];
	else{
		int curFaceId = -1;
		int curSqr = 0;
        
		for( int faceId=0; faceId<faceNum; faceId++ ){
			const cv::Rect& face = faces[ faceId ];
			int sqr = face.width * face.height;
			if( sqr > curSqr ){
				curSqr = sqr;
				curFaceId = faceId;
			}
		}
        
		faceRect = faces[ curFaceId ];
	}
    
	mTestFaceRect.x = faceRect.x;
	mTestFaceRect.w = mImgSize.y - faceRect.y;
	mTestFaceRect.z = mTestFaceRect.x + faceRect.width;
	mTestFaceRect.y = mTestFaceRect.w - faceRect.height;
	
	// *Calculate the image warp matrix
	// Calculate the bounding box of average landmarks
	myfloat2 landBBoxMin, landBBoxMax;
	mAvgLands.boundingBox( landBBoxMin, landBBoxMax );
	myfloat2 bboxCen = ( landBBoxMax + landBBoxMin ) / 2.f;

	float landW = landBBoxMax.x - landBBoxMin.x;
	float landH = landBBoxMax.y - landBBoxMin.y;
	float landLen = sqrt( landW * landW + landH * landH );

	// Calculate face scale and translation
	float faceW = mTestFaceRect.z - mTestFaceRect.x;
	float faceH = mTestFaceRect.w - mTestFaceRect.y;
	float faceLen = sqrt( faceW * faceW + faceH * faceH ) * 0.7f;

	myfloat2 faceCen;
	faceCen.x = ( mTestFaceRect.x + mTestFaceRect.z ) / 2.f;
	float alpha = 0.35f, alpha_1 = 1.f - alpha;
	faceCen.y = mTestFaceRect.y * alpha_1 + mTestFaceRect.w * alpha;	

	float scale = faceLen / landLen;
	myfloat2 trans = faceCen - bboxCen * scale;

	mTestImgWarpMat.mergeTrans( trans, scale );

	
	// *Place the initial lands
	int landsNum = mLandSpace.size();
	std::vector<bool> landsChoose;
	landsChoose.assign( landsNum, false );

	mTestInitLandsList.resize( mTestInitNum );	
	srand( (unsigned)time(NULL) );
	for( int i=0; i<mTestInitNum; i++ ){
		int landsId = -1;
		do 
		{
			landsId = rand() % landsNum;
		} while( landsChoose[ landsId] );
		landsChoose[ landsId ] = true;

		mTestInitLandsList[ i ] = mLandSpace[ landsId ];
	}

	mTestCurLandsList = mTestInitLandsList;

	return true;
}

void RegTest::testSamplePixels( int regId )
{
	// *Warp current landmarks to average
	// *find the warp matrix
	float s;
	Mat2f R;
	myfloat2 t;
	comScaleAlignPtList( mTestCurLands, mAvgLands, s, R, t );
	Mat3f warpMat;
	warpMat.mergeTrans( s, R, t );
	Mat3f invWarpMat = warpMat.inverse();

	// Calculate the warped landmarks
	int landNum = mTestCurLands.size();
	sFloat2List warpLands( landNum );
	for( int landId=0; landId<landNum; landId++ )
		warpLands[ landId ] = warpMat * mTestCurLands[ landId ];

	// Calculate the sum warp
	Mat3f totalMat = mTestImgWarpMat * invWarpMat;

	
	// *Sample pixels, build appearance vector
	const sReg& reg = mRegList[ regId ];
	int spNum = reg.mSampleList.size();
	mTestSamplePosList.resize( spNum );
	mTestSampleValList.resize( spNum );
	for( int spId=0; spId<spNum; spId++ ){
		const myfloat3& sp = reg.mSampleList[ spId ];

		// Sample the pixel position
		int landId = int( sp.x );
		myfloat2 offset( sp.y, sp.z );

		myfloat2 pos = warpLands[ landId ] + offset;
		pos = totalMat * pos;

		mTestSamplePosList[ spId ] = pos;

		// Get the value
		myint2 posInt( pos.x, pos.y );
		posInt.x = MAX( 0, posInt.x );	posInt.x = MIN( mImgSize.x - 1, posInt.x );
		posInt.y = MAX( 0, posInt.y );	posInt.y = MIN( mImgSize.y - 1, posInt.y );

		const myByte3& rgb = mImgData[ posInt.y * mImgSize.x + posInt.x ];
		mTestSampleValList[ spId ] = comRGB2Lum( rgb );
	}
}

void RegTest::testPassReg( int regId )
{
	const sReg& reg = mRegList[ regId ];
	int fernNum = reg.mFernList.size();
	for( int fernId=0; fernId<fernNum; fernId++ ){
		const sRegFern& fern = reg.mFernList[ fernId ];
		int binId = 0;
		int feaNum = fern.mSpIdPairList.size();
		for( int feaId=0; feaId<feaNum; feaId++ ){
			const myint2& spIdPair = fern.mSpIdPairList[ feaId ];
			float val = mTestSampleValList[ spIdPair.x ] - mTestSampleValList[ spIdPair.y ];

			if( val > fern.mThresList[ feaId ] ){
				int offset = 1 << feaId;
				binId += offset;
			}
		}

		const sFloat2List& output = fern.mBinList[ binId ].mOutput;
		if( output.size() > 0 )
			mTestCurLands += output;
	}
}

void RegTest::testMedianResult()
{
	int initNum = mTestCurLandsList.size();
	int landNum = mAvgLands.size();
	sFloat2List tmpLands( landNum );
	for( int landId=0; landId<landNum; landId++ ){
		
		sFloatList xList( initNum );
		sFloatList yList( initNum );
		for( int initId=0; initId<initNum; initId++ ){
			const sFloat2List& lands = mTestCurLandsList[ initId ];
			xList[ initId ] = lands[ landId ].x;
			yList[ initId ] = lands[ landId ].y;
		}


		int midId = ( initNum + 1 ) / 2;
		myfloat2& medianLand = tmpLands[ landId ];
		for( int i=0; i<midId; i++ ){
			float curXMax = -INFITE_FLOAT, curYMax = -INFITE_FLOAT;
			int xId = -1, yId = -1;
			for( int initId=0; initId<initNum; initId++ ){
				float x = xList[ initId ];
				float y = yList[ initId ];

				if( x > curXMax ){
					curXMax = x;
					xId = initId;
				}

				if( y > curYMax ){
					curYMax = y;
					yId = initId;
				}
			}

			medianLand = myfloat2( curXMax, curYMax );
			xList[ xId ] = -INFITE_FLOAT;
			yList[ yId ] = -INFITE_FLOAT;
		}
	}

    mTestCurLands.resize( landNum );
    for( int landId=0; landId<landNum; landId++ ){
        mTestCurLands[ landId ] = mTestImgWarpMat * tmpLands[ landId ];
    }
    
	//mTestCurLands = tmpLands;
	mTestCurLandsList[ 0 ] = tmpLands;
}

// <OpenCV>
void RegTest::initDetector()
{
	// Face detector
    char faceDetectorDataFile[500];
    sprintf(faceDetectorDataFile, "%s/haarcascade_frontalface_alt2.xml", mDataDir);
	if( !mCVFaceDetector.load(faceDetectorDataFile) )
		printf( "Error: open face cascade!\n" );
	else
		mCVFaceDetectorInit = true;
}

void RegTest::test()
{
    sFloat2List xList = mAvgLands;
    
    float s;
    Mat2f R;
    myfloat2 T;
    
    s = 0.1f;
    
    float angle = -M_PI / 0.337f;
    R( 0, 0 ) = cos( angle );
    R( 1, 0 ) = sin( angle );
    R( 0, 1 ) = -R( 1, 0 );
    R( 1, 1 ) = R( 0, 0 );
    
    T = myfloat2( 2.5, 300 );
    
    printf( "Input:\n" );
    printf( "%f\n\n", s );
    R.print();
    T.print();
    
    int n = xList.size();
    sFloat2List yList( n );
    
    Mat3f warpMat;
    warpMat.mergeTrans( s, R, T );
    for( int i=0; i<n; i++ )
        yList[ i ] = warpMat * xList[ i ];
    
    // Scale and Align
    float tS;
    Mat2f tR;
    myfloat2 tT;
    
    comScaleAlignPtList( xList, yList, tS, tR, tT );
    
    printf( "Check:\n" );
    printf( "%f\n\n", tS );
    tR.print();
    tT.print();
}
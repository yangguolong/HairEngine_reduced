//	******************************************************
//	DataHolder
//	Hold the global data of the whole framework
//	Author: Chen Cao	19/08/2013
//	******************************************************

// stl
//#include <atlimage.h>

// Head
#include "DataHolder.h"

// Common
#include "common.h"

//DataHolder gDH;

//////////////////////////////////////////////////////////////////////////
// <Constructor>
DataHolder::DataHolder()
{
	// Initialize file format
	mFileFormatStr = "All Files(*.*);;\
					 Reg Files(*.rgs)";

	// Initialize image
	initImg();

	// Initialize land
	initLand();
}
// </Constructor>
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// <Save-load file format>
void DataHolder::initFileFormat()
{

}
// </Save-load file format>
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// <Image & Land>
// Image
void DataHolder::initImg()
{
	// Image list
	mFrmList				.clear();
	mImgNameList			.clear();

	mCurFrmId				= -1;
	mCurFrm_P				= NULL;

	// Image parameters
	mImgSize				= myint2( 640, 480 );
	mImgCoord				= myfloat4( 0.f, 0.f, 1.f, 1.f );
	mImgRect				= myfloat4( -1.2f, -0.9f, 1.2f, 0.9f );
	mImgDepth				= -400.f;

	// ----------------------------------------
	// *Construct the image matrix
	
	// Calculate the scale and offset
	myfloat2 dstDim, dstPos;
	dstDim.x = mImgRect.z - mImgRect.x;
	dstDim.y = mImgRect.w - mImgRect.y;

	dstPos.x = ( mImgRect.x + mImgRect.z ) / 2.f;
	dstPos.y = ( mImgRect.y + mImgRect.w ) / 2.f;

	myfloat2 srcDim, srcPos;
	srcDim = myfloat2( mImgSize.x, mImgSize.y );
	srcPos = srcDim / 2.f;

	myfloat2 scale, offset;
	scale.x = dstDim.x / srcDim.x;
	scale.y = dstDim.y / srcDim.y;

	offset.x = dstPos.x - srcPos.x * scale.x;
	offset.y = dstPos.y - srcPos.y * scale.y;

	// Set the matrix
	mImgMat.identity();

	mImgMat( 0, 0 ) = scale.x;
	mImgMat( 0, 3 ) = offset.x;

	mImgMat( 1, 1 ) = scale.y;
	mImgMat( 1, 3 ) = offset.y;

	mImgMat( 2, 2 ) = scale.x;
}

// Land
void DataHolder::initLand()
{
	mLandNum = CCLandNum;

	// Load land line ids
	std::string fileName = "Data\\LandmarkLine.txt";
	FILE* fp;
	fp = fopen(fileName.c_str(), "r" );
	if( fp ){
		int lineNum = 0;
		fscanf( fp, "%d", &lineNum );

		mLandLineIds.resize( lineNum );
		for( int lineId=0; lineId<lineNum; lineId++ ){
			sIntList& landIds = mLandLineIds[ lineId ];

			int landNum = 0;
			fscanf( fp, "%d", &landNum );
			landIds.resize( landNum );
			for( int i=0; i<landNum; i++ )
				fscanf( fp, "%d", &landIds[i] );
		}
	}

	fclose( fp );
}
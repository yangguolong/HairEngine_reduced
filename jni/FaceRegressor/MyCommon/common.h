//	******************************************************
//	Common
//	Classes of some common functions
//	Author: Chen Cao	19/08/2013
//	******************************************************

#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <string>

// Common
#include "list.h"
#include "ccmatrix.h"

namespace cc{
	// Scale and align xList to yList, with a scale, a rotation and a translation
	void comScaleAlignPtList					( const sFloat3List& xList,
												 const sFloat3List& yList,
												 float& scale,
												 Mat3f& rot,
												 myfloat3& trans,
												 bool ignoreZ = false );
    
	void comScaleAlignPtList					( const sFloat2List& xList,
                                                 const sFloat2List& yList,
                                                 float& scale,
                                                 Mat2f& rot,
                                                 myfloat2& trans );
	// *RGB to luminance
	inline float comRGB2Lum						( const myByte3& rgb )
	{
		float lum = float( rgb.x ) / 255.f * 0.2126f
        + float( rgb.y ) / 255.f * 0.7152f
        + float( rgb.z ) / 255.f * 0.0722f;
        
		return lum;
	}
    
    
}

#endif
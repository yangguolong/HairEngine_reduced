//	******************************************************
//	Common
//	Classes of some common functions
//	Author: Chen Cao	19/08/2013
//	******************************************************

#include "common.h"
#include "timing.h"

namespace cc{
    
    void comScaleAlignPtList( const sFloat3List& xList, const sFloat3List& yList, float& scale,
                             Mat3f& rot, myfloat3& trans, bool ignoreZ /* = false */ )
    {
        int n = xList.size();
        
        // muX and muY
        myfloat3 muX, muY;
        for( int i=0; i<n; i++ ){
            muX += xList[ i ];
            muY += yList[ i ];
        }
        muX /= float( n );
        muY /= float( n );
        
        if( ignoreZ ){
            muX.z = muY.z = 0.f;
        }
        
        // gammaX_2 and gammaY_2, and SigmaXY
        float gammaX = 0.f, gammaY = 0.f;
        Mat3f sigmaXY;
        sigmaXY.zero();
        for( int i=0; i<n; i++ ){
            myfloat3 deltaX = ( xList[ i ] - muX );
            myfloat3 deltaY = ( yList[ i ] - muY );
            
            if( ignoreZ ){
                deltaX.z = deltaY.z = 0.f;
            }
            
            gammaX += deltaX.squre();
            gammaY += deltaY.squre();
            
            for( int rowId=0; rowId<3; rowId++ ){
                for( int colId=0; colId<3; colId++ ){
                    float val = deltaY[ rowId ] * deltaX[ colId ];
                    sigmaXY( rowId, colId ) += val;
                }
            }
        }
        gammaX /= float( n );
        gammaY /= float( n );
        sigmaXY /= float( n );
        
        // SVD decomposition
        Mat3f U, V;
        myfloat3 D;
        sigmaXY.SVD( U, D, V );
        
        float detU = U.det();
        float detV = V.det();
        float detU_detV = detU * detV;
        
        Mat3f S;
        S.identity();
        if( detU_detV < 0.f )	S( 2, 2 ) = -1.f;
        
        // Rotation
        rot = U * S * V;
        
        // Scale
        float trDS = 0.f;
        for( int i=0; i<3; i++ )
            trDS += D[ i ] * S( i, i );
        scale = trDS / gammaX;
        
        // Translation
        trans = muY - rot * muX * scale;
    }
    
    void comScaleAlignPtList( const sFloat2List& xList, const sFloat2List& yList, float& scale,
                             Mat2f& rot, myfloat2& trans )
    {
        int n = xList.size();
        
        // muX and muY
        myfloat2 muX, muY;
        for( int i=0; i<n; i++ ){
            muX += xList[ i ];
            muY += yList[ i ];
        }
        muX /= float( n );
        muY /= float( n );
        
        // gammaX_2 and gammaY_2, and SigmaXY
        float gammaX = 0.f, gammaY = 0.f;
        Mat2f sigmaXY;
        sigmaXY.zero();
        for( int i=0; i<n; i++ ){
            myfloat2 deltaX = ( xList[ i ] - muX );
            myfloat2 deltaY = ( yList[ i ] - muY );
            
            gammaX += deltaX.squre();
            gammaY += deltaY.squre();
            
            for( int rowId=0; rowId<2; rowId++ ){
                for( int colId=0; colId<2; colId++ ){
                    float val = deltaY[ rowId ] * deltaX[ colId ];
                    sigmaXY( rowId, colId ) += val;
                }
            }
        }
        gammaX /= float( n );
        gammaY /= float( n );
        sigmaXY /= float( n );
        
        // SVD decomposition
        Mat2f U, V;
        myfloat2 D;
        sigmaXY.SVD( U, D, V );
        
        // Rotation
        rot = U * V;
        
        // Scale
        float trDS = 0.f;
        for( int i=0; i<2; i++ )
            trDS += D[ i ];
        scale = trDS / gammaX;
        
        // Translation
        trans = muY - rot * muX * scale;
    }
};
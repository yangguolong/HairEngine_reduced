//	******************************************************
//	Matrix
//	Classes of different matrix
//	Author: Chen Cao	20/08/2013
//	******************************************************

#ifndef CC_MATRIX_H
#define CC_MATRIX_H

//#include "Core.h"
//#include "LU.h"
//#include "Cholesky.h"
//#include "SVD.h"

#include "type.h"
#include "vec.h"
#include "quaternion.h"
#include "opencv2/opencv.hpp"

namespace cc{

	template<class T>
	class Mat2{
	public:
		T							mVal[ 4 ];

		// Constructor
		Mat2						() 
		{
			zero();
		}

		Mat2						( const T* valPtr )
		{
			for( int i=0; i<4; i++ )	mVal[ i ] = valPtr[ i ];
		}

		Mat2						( const Mat2<T>& copy )
		{
			for( int i=0; i<4; i++ )	mVal[ i ] = copy.mVal[ i ];
		}

		// Access operators
		inline T& operator()		( const int& i, const int& j )
		{
			return mVal[ j * 2 + i ];
		}

		inline const T& operator()	( const int& i, const int& j ) const
		{
			return mVal[ j * 2 + i ];
		}

		inline T& operator[]		( const int& i )
		{
			return mVal[ i ];
		}

		inline const T& operator[]	( const int& i ) const
		{
			return mVal[ i ];
		}

		inline T* ptr				()
		{
			return mVal;
		}

		inline const T* ptr			() const
		{
			return mVal;
		}

		inline void print			() const
		{
			for( int j=0; j<2; j++ ){
				for( int i=0; i<2; i++ )
					std::cout << (*this)( i, j ) << "\t";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		// Math operators
		inline Vec2<T> operator*	( const Vec2<T>& rhs ) const
		{
			Vec2<T> ret;
			ret.x = mVal[0] * rhs.x + mVal[2] * rhs.y;
			ret.y = mVal[1] * rhs.x + mVal[3] * rhs.y;

			return ret;
		}

		inline Mat2<T> operator*	( const Mat2<T>& rhs ) const
		{
			Mat2<T> ret;

			for( int j=0; j<2; j++ ){		// Column
				for( int i=0; i<2; i++ ){	// Row
					T& dst = ret( i, j );
					dst = 0;

					for( int k=0; k<2; k++ )
						dst += (*this)( i, k ) * rhs( k, j );
				}
			}

			return ret;
		}

		template<typename TYPE>
		inline void operator/=		( const TYPE& rhs )
		{
			for( int i=0; i<4; i++ )
				mVal[ i ] /= rhs;
		}

		inline void identity		()
		{
			zero();
			mVal[ 0 ] = mVal[ 3 ] = 1;
		}

		inline void zero			()
		{
			for( int i=0; i<4; i++ )	mVal[ i ] = 0;
		}

		void inverseLocal			()
		{
            cv::Matx22d mat;
            for( int j=0; j<2; j++ )
				for( int i=0; i<2; i++ )
					mat( i, j ) = (*this)( i, j );
            
            cv::Matx22d invMat = mat.inv();
            
            for( int j=0; j<2; j++ )
				for( int i=0; i<2; i++ )
					(*this)( i, j ) = invMat( i, j );
		}

		Mat2<T> inverse				() const
		{
			Mat2<T> ret( *this );
			ret.inverseLocal();
			return ret;
		}

		// A = USV*
		void SVD					( Mat2<T>& U, Vec2<T>& S, Mat2<T>& V ) const
		{
            cv::Matx22d mat;
            for( int j=0; j<2; j++ )
				for( int i=0; i<2; i++ )
					mat( i, j ) = (*this)( i, j );
            cv::SVD svd(mat);
            cv::Matx22d u(svd.u);
            cv::Matx22d v(svd.vt);
            //v = v.t();
            cv::Vec2d s(svd.w);
            
            for(int j=0; j<2; j++)
            {
				S[ j ] = s[ j ];
				for( int i=0; i<2; i++ ){
					U( i, j ) = u(i, j);
					V( i, j ) = v(i, j);
				}
            }
		}

		T det						() const
		{
//			Eigen::Matrix2d mat;
//			for( int j=0; j<2; j++ )
//				for( int i=0; i<2; i++ )
//					mat( i, j ) = (*this)( i, j );
//
//			T ret = mat.determinant();
//			return ret;
            
            cv::Matx22d mat;
            for( int j=0; j<2; j++ )
				for( int i=0; i<2; i++ )
					mat( i, j ) = (*this)( i, j );
            
            T ret = cv::determinant(mat);
            return ret;
		}
	};

	template<class T>
	class Mat3{
	public:
		T							mVal[ 9 ];

		// Constructor
		Mat3						() 
		{
			zero();
		}

		Mat3						( const T* valPtr )
		{
			for( int i=0; i<9; i++ )	mVal[ i ] = valPtr[ i ];
		}

		Mat3						( const Mat3<T>& copy )
		{
			for( int i=0; i<9; i++ )	mVal[ i ] = copy.mVal[ i ];
		}

		// Access operators
		inline T& operator()		( const int& i, const int& j )
		{
			return mVal[ j * 3 + i ];
		}

		inline const T& operator()	( const int& i, const int& j ) const
		{
			return mVal[ j * 3 + i ];
		}

		inline T& operator[]		( const int& i )
		{
			return mVal[ i ];
		}

		inline const T& operator[]	( const int& i ) const
		{
			return mVal[ i ];
		}

		inline T* ptr				()
		{
			return mVal;
		}

		inline const T* ptr			() const
		{
			return mVal;
		}

		template<typename TYPE>
		inline void transFromQuat	( const CCQuaternion<TYPE>& quat )
		{
			zero();

			// Calculate the rotation matrix
			T sqw = quat[ 3 ] * quat[ 3 ];
			T sqx = quat[ 0 ] * quat[ 0 ];
			T sqy = quat[ 1 ] * quat[ 1 ];
			T sqz = quat[ 2 ] * quat[ 2 ];

			mVal[ 0 ] =  sqx - sqy - sqz + sqw;	// 00
			mVal[ 4 ] = -sqx + sqy - sqz + sqw;	// 11
			mVal[ 8 ] = -sqx - sqy + sqz + sqw;	// 22

			T tmp1 = quat[ 0 ] * quat[ 1 ];
			T tmp2 = quat[ 2 ] * quat[ 3 ];
			mVal[ 1 ] = 2.f * ( tmp1 + tmp2 );	// 10
			mVal[ 3 ] = 2.f * ( tmp1 - tmp2 );	// 01

			tmp1 = quat[ 0 ] * quat[ 2 ];
			tmp2 = quat[ 1 ] * quat[ 3 ];
			mVal[ 2 ] = 2.f * ( tmp1 - tmp2 );	// 20
			mVal[ 6 ] = 2.f * ( tmp1 + tmp2 );	// 02

			tmp1 = quat[ 1 ] * quat[ 2 ];
			tmp2 = quat[ 0 ] * quat[ 3 ];
			mVal[ 5 ] = 2.f * ( tmp1 + tmp2 );	// 21
			mVal[ 7 ] = 2.f * ( tmp1 - tmp2 );	// 12
		}

		template<typename TYPE>
		inline void transToQuat		( CCQuaternion<TYPE>& quat ) const
		{
			const Mat3<T>& a = *this;
			T trace = a( 0, 0 ) + a( 1, 1 ) + a( 2, 2 );
			if( trace > 0 ){
				T s = 0.5 / sqrt( trace + 1.0 );
				quat[ 3 ] = 0.25 / s;
				quat[ 0 ] = ( a(2,1) - a(1,2) ) * s;
				quat[ 1 ] = ( a(0,2) - a(2,0) ) * s;
				quat[ 2 ] = ( a(1,0) - a(0,1) ) * s;
			}
			else{
				if ( a(0,0) > a(1,1) && a(0,0) > a(2,2) ) {
					T s = 2.0f * sqrt( 1.0 + a(0,0) - a(1,1) - a(2,2) );
					quat[ 3 ] = ( a(2,1) - a(1,2) ) / s;
					quat[ 0 ] = 0.25 * s;
					quat[ 1 ] = ( a(0,1) + a(1,0) ) / s;
					quat[ 2 ] = ( a(0,2) + a(2,0) ) / s;
				} else if ( a(1,1) > a(2,2)) {
					T s = 2.0 * sqrt( 1.0 + a(1,1) - a(0,0) - a(2,2) );
					quat[ 3 ] = ( a(0,2) - a(2,0) ) / s;
					quat[ 0 ] = ( a(0,1) + a(1,0) ) / s;
					quat[ 1 ] = 0.25 * s;
					quat[ 2 ] = ( a(1,2) + a(2,1) ) / s;
				} else {
					T s = 2.0 * sqrt( 1.0 + a(2,2) - a(0,0) - a(1,1) );
					quat[ 3 ] = ( a(1,0) - a(0,1) ) / s;
					quat[ 0 ] = ( a(0,2) + a(2,0) ) / s;
					quat[ 1 ] = ( a(1,2) + a(2,1) ) / s;
					quat[ 2 ] = 0.25 * s;
				}
			}

			quat.normalizeLocal();
		}

		inline void print			() const
		{
			for( int j=0; j<3; j++ ){
				for( int i=0; i<3; i++ )
					std::cout << (*this)( i, j ) << "\t";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		// Math operators
		inline Vec2<T> operator*	( const Vec2<T>& rhs ) const
		{
			Vec2<T> ret;
			ret.x = mVal[0] * rhs.x + mVal[3] * rhs.y + mVal[6];
			ret.y = mVal[1] * rhs.x + mVal[4] * rhs.y + mVal[7];
			
			return ret;
		}

		inline Vec3<T> operator*	( const Vec3<T>& rhs ) const
		{
			Vec3<T> ret;
			ret.x = mVal[0] * rhs.x + mVal[3] * rhs.y + mVal[6] * rhs.z;
			ret.y = mVal[1] * rhs.x + mVal[4] * rhs.y + mVal[7] * rhs.z;
			ret.z = mVal[2] * rhs.x + mVal[5] * rhs.y + mVal[8] * rhs.z;

			return ret;
		}

		inline Mat3<T> operator*	( const Mat3<T>& rhs ) const
		{
			Mat3<T> ret;

			for( int j=0; j<3; j++ ){		// Column
				for( int i=0; i<3; i++ ){	// Row
					T& dst = ret( i, j );
					dst = 0;

					for( int k=0; k<3; k++ )
						dst += (*this)( i, k ) * rhs( k, j );
				}
			}

			return ret;
		}

		template<typename TYPE>
		inline void operator/=		( const TYPE& rhs )
		{
			for( int i=0; i<9; i++ )
				mVal[ i ] /= rhs;
		}

		inline void identity		()
		{
			zero();
			mVal[ 0 ] = mVal[ 4 ] = mVal[ 8 ] = 1;
		}

		inline void zero			()
		{
			for( int i=0; i<9; i++ )	mVal[ i ] = 0;
		}

		template<typename TYPE>
		inline void mergeTrans		( const TYPE& scale, const Mat2<TYPE>& rot,
									const Vec2<TYPE>& trans )
		{
			zero();

			(*this)( 0, 0 )		= rot[ 0 ] * scale;
			(*this)( 1, 0 )		= rot[ 1 ] * scale;
			
			(*this)( 0, 1 )		= rot[ 2 ] * scale;
			(*this)( 1, 1 )		= rot[ 3 ] * scale;

			(*this)( 0, 2 )		= trans[ 0 ];
			(*this)( 1, 2 )		= trans[ 1 ];

			(*this)( 2, 2 )		= 1;
		}

		inline void mergeTrans		( const Vec2<T>& trans, const T& scale )
		{
			zero();

			(*this)( 0, 0 )		= scale;
			(*this)( 1, 1 )		= scale;
			(*this)( 2, 2 )		= scale;

			(*this)( 0, 2 )		= trans.x;
			(*this)( 1, 2 )		= trans.y;
		}

		void inverseLocal			()
		{
            cv::Matx33d mat;
            for( int j=0; j<3; j++ )
				for( int i=0; i<3; i++ )
					mat( i, j ) = (*this)( i, j );
            
            cv::Matx33d invMat = mat.inv();
            
            for( int j=0; j<3; j++ )
				for( int i=0; i<3; i++ )
					(*this)( i, j ) = invMat( i, j );
		}

		Mat3<T> inverse				() const
		{
			Mat3<T> ret( *this );
			ret.inverseLocal();
			return ret;
		}

		// A = USV*
		void SVD					( Mat3<T>& U, Vec3<T>& S, Mat3<T>& V ) const
		{
            cv::Matx33d mat;
            for( int j=0; j<3; j++ )
				for( int i=0; i<3; i++ )
					mat( i, j ) = (*this)( i, j );
            cv::SVD svd(mat);
            cv::Matx33d u(svd.u);
            cv::Matx33d v(svd.vt);
            //v = v.t();
            cv::Vec3d s(svd.w);
            
            for(int j=0; j<3; j++)
            {
				S[ j ] = s[ j ];
				for( int i=0; i<3; i++ ){
					U( i, j ) = u(i, j);
					V( i, j ) = v(i, j);   
				}
            }
		}

		T det						() const
		{
            cv::Matx33d mat;
            for( int j=0; j<3; j++ )
				for( int i=0; i<3; i++ )
					mat( i, j ) = (*this)( i, j );
            
            T ret = cv::determinant(mat);
            return ret;
		}
	};

	template<class T>
	class Mat4{
	public:
		T							mVal[ 16 ];

		// Constructor
		Mat4						() 
		{
			zero();
		}

		Mat4						( const T* valPtr )
		{
			for( int i=0; i<16; i++ )	mVal[ i ] = valPtr[ i ];
		}

		Mat4						( const Mat4<T>& copy )
		{
			for( int i=0; i<16; i++ )	mVal[ i ] = copy.mVal[ i ];
		}

		// Access operators
		inline T& operator()		( const int& i, const int& j )
		{
			return mVal[ j * 4 + i ];
		}

		inline const T& operator()	( const int& i, const int& j ) const
		{
			return mVal[ j * 4 + i ];
		}

		inline T& operator[]		( const int& i )
		{
			return mVal[ i ];
		}

		inline const T& operator[]	( const int& i ) const
		{
			return mVal[ i ];
		}

		inline T* ptr				()
		{
			return mVal;
		}

		inline const T* ptr			() const
		{
			return mVal;
		}

		inline void print			() const
		{
			for( int j=0; j<3; j++ ){
				for( int i=0; i<3; i++ )
					std::cout << (*this)( i, j ) << "\t";
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		// Math operators
		inline Vec3<T> operator*	( const Vec3<T>& rhs ) const
		{
			Vec3<T> ret;
			ret.x = mVal[0] * rhs.x + mVal[4] * rhs.y + mVal[8] * rhs.z + mVal[12];
			ret.y = mVal[1] * rhs.x + mVal[5] * rhs.y + mVal[9] * rhs.z + mVal[13];
			ret.z = mVal[2] * rhs.x + mVal[6] * rhs.y + mVal[10] * rhs.z + mVal[14];

			return ret;
		}

		inline Mat4<T> operator*	( const Mat4<T>& rhs ) const
		{
			Mat4<T> ret;

			for( int j=0; j<4; j++ ){		// Column
				for( int i=0; i<4; i++ ){	// Row
					T& dst = ret( i, j );
					dst = 0;

					for( int k=0; k<4; k++ )
						dst += (*this)( i, k ) * rhs( k, j );
				}
			}
		}

		template<typename TYPE>
		inline void operator/=		( const TYPE& rhs )
		{
			for( int i=0; i<16; i++ )
				mVal[ i ] /= rhs;
		}

		inline void identity		()
		{
			zero();
			mVal[ 0 ] = mVal[ 5 ] = mVal[ 10 ] = mVal[ 15 ] = 1;
		}

		inline void zero			()
		{
			for( int i=0; i<16; i++ )	mVal[ i ] = 0;
		}

		inline void mergeTrans		( const Vec3<T>& trans, const T& scale )
		{
			zero();

			(*this)( 0, 0 )		= scale;
			(*this)( 1, 1 )		= scale;
			(*this)( 2, 2 )		= scale;
			(*this)( 3, 3 )		= 1.f;

			(*this)( 0, 3 )		= trans.x;
			(*this)( 1, 3 )		= trans.y;
			(*this)( 2, 3 )		= trans.z;
		}

		inline void mergeTrans		( const T& scale, const Mat3<T>& rot, 
									const Vec3<T>& trans )
		{
			zero();

			(*this)( 0, 0 ) = rot[ 0 ] * scale;
			(*this)( 1, 0 ) = rot[ 1 ] * scale;
			(*this)( 2, 0 ) = rot[ 2 ] * scale;

			(*this)( 0, 1 ) = rot[ 3 ] * scale;
			(*this)( 1, 1 ) = rot[ 4 ] * scale;
			(*this)( 2, 1 ) = rot[ 5 ] * scale;

			(*this)( 0, 2 ) = rot[ 6 ] * scale;
			(*this)( 1, 2 ) = rot[ 7 ] * scale;
			(*this)( 2, 2 ) = rot[ 8 ] * scale;

			(*this)( 0, 3 ) = trans[ 0 ];
			(*this)( 1, 3 ) = trans[ 1 ];
			(*this)( 2, 3 ) = trans[ 2 ];

			(*this)( 3, 3 ) = 1.f;
		}

		void inverseLocal			()
		{            
            cv::Matx44d mat;
            for( int j=0; j<4; j++ )
				for( int i=0; i<4; i++ )
					mat( i, j ) = (*this)( i, j );
            
            cv::Matx44d invMat = mat.inv();
            for( int j=0; j<4; j++ )
				for( int i=0; i<4; i++ )
					(*this)( i, j ) = invMat( i, j );
		}

		Mat4<T> inverse				() const
		{
			Mat4<T> ret( *this );
			ret.inverseLocal();
			return ret;
		}

		// A = USV*
		void SVD					( Mat4<T>& U, Vec4<T>& S, Mat4<T>& V ) const
		{
            cv::Matx44d mat;
            for( int j=0; j<4; j++ )
				for( int i=0; i<4; i++ )
					mat( i, j ) = (*this)( i, j );
            cv::SVD svd(mat);
            cv::Matx44d u(svd.u);
            cv::Matx44d v(svd.vt);
			//v = v.t();
            cv::Vec4d s(svd.w);
            
            for(int j=0; j<4; j++)
            {
				S[ j ] = s[ j ];
				for( int i=0; i<4; i++ ){
					U( i, j ) = u(i, j);
					V( i, j ) = v(i, j); 
				}
            }
		}

		T det						() const
		{
            cv::Matx44d mat;
            for( int j=0; j<4; j++ )
				for( int i=0; i<4; i++ )
					mat( i, j ) = (*this)( i, j );
            T ret = cv::determinant(mat);
            return ret;
		}
	};

	typedef Mat2<double>		Mat2d;
	typedef Mat2<float>			Mat2f;	
	typedef Mat2<int>			Mat2i;

	typedef Mat3<double>		Mat3d;
	typedef Mat3<float>			Mat3f;	
	typedef Mat3<int>			Mat3i;

	typedef Mat4<double>		Mat4d;
	typedef Mat4<float>			Mat4f;	
	typedef Mat4<int>			Mat4i;
}

#endif
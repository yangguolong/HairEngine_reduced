//	******************************************************
//	Quaternion
//	Classes of quaternion
//	Author: Chen Cao	20/08/2013
//	******************************************************

#ifndef QUATERNION_H
#define QUATERNION_H


#include "type.h"
#include "vec.h"

#include <math.h>

namespace cc{

	template<class T>
	class CCQuaternion{
	public:
		T									mVal[ 4 ];

		typedef PROMOTE_T_TO_FLOAT CC_REAL;

		// Constructor
		CCQuaternion							( const T& theta, Vec3<T>& vector )
		{
			vector.normalizeLocal();

			T halfTheta = theta / 2.f;
			mVal[ 3 ] = cos( halfTheta );

			T sinHalfTheta = sin( halfTheta );
			mVal[ 0 ] = sinHalfTheta * vector.x;
			mVal[ 1 ] = sinHalfTheta * vector.y;
			mVal[ 2 ] = sinHalfTheta * vector.z;
		}

		CCQuaternion							( const T x = 0, const T y = 0,
											const T z = 0, const T w = 0 )
		{
			mVal[ 0 ] = x;
			mVal[ 1 ] = y;
			mVal[ 2 ] = z;
			mVal[ 3 ] = w;
		}

		CCQuaternion							( const CCQuaternion& copy )
		{
			for( int i=0; i<4; i++ )
				mVal[ i ] = copy.mVal[ i ];
		}

		// Access operators
		inline T& operator[]				( const int id )
		{
			return mVal[ id ];
		}

		inline const T& operator[]			( const int id ) const
		{
			return mVal[ id ];
		}

		inline void print					() const
		{
			for( int i=0; i<4; i++ )
				std::cout << mVal[ i ] << "\t";
			std::cout << std::endl << std::endl;
		}
		
		// Math operators
		inline CCQuaternion<T> operator=		( const CCQuaternion<T>& rhs )
		{
			for( int i=0; i<4; i++ ) mVal[ i ] = rhs.mVal[ i ];
			return (*this);
		}

		inline CCQuaternion<T>& operator+=	( const CCQuaternion<T>& rhs )
		{
			for( int i=0; i<4; i++ ) mVal[ i ] += rhs.mVal[ i ];
			return (*this);
		}

		inline CCQuaternion<T>& operator-=	( const CCQuaternion<T>& rhs )
		{
			for( int i=0; i<4; i++ ) mVal[ i ] -= rhs.mVal[ i ];
			return (*this);
		}

		inline CCQuaternion<T>& operator*=	( const CCQuaternion<T>&rhs )
		{
			T w1 = mVal[ 3 ], x1 = mVal[ 0 ], y1 = mVal[ 1 ], z1 = mVal[ 2 ];
			T w2 = rhs.mVal[ 3 ], x2 = rhs.mVal[ 0 ];
			T y2 = rhs.mVal[ 1 ], z2 = rhs.mVal[ 2 ];

			mVal[3] = w1*w2 - x1*x2 - y1*y2 - z1*z2;
			mVal[0] = w1*x2 + x1*w2 + y1*z2 - z1*y2;
			mVal[1] = w1*y2 - x1*z2 + y1*w2 + z1*x2;
			mVal[2] = w1*z2 + x1*y2 - y1*x2 + z1*w2;

			return (*this);
		}

		inline CCQuaternion<T>& operator/=	( const T& rhs )
		{
			for( int i=0; i<4; i++ ) mVal[ i ] /= rhs;
			return (*this);
		}

		inline CCQuaternion<T> operator+		( const CCQuaternion& rhs ) const
		{
			CCQuaternion<T> ret( *this );
			ret += rhs;
			return ret;
		}

		inline CCQuaternion<T> operator-		( const CCQuaternion& rhs ) const
		{
			CCQuaternion<T> ret( *this );
			ret -= rhs;
			return ret;
		}

		inline CCQuaternion<T> operator*		( const CCQuaternion& rhs ) const
		{
			CCQuaternion<T> ret( *this );
			ret *= rhs;
			return ret;
		}

		inline CCQuaternion<T> operator/		( const T& rhs ) const
		{
			CCQuaternion<T> ret( *this );
			ret /= rhs;
			return ret;
		}

		inline CCQuaternion<T> operator-		() const
		{
			CCQuaternion ret;
			ret.mVal[ 3 ] = mVal[ 3 ];
			for( int i=0; i<3; i++ )	ret.mVal[ i ] = -mVal[ i ];

			return ret;
		}

		template<typename TYPE>
		inline void rotate					( Vec3<TYPE>& rhs ) const
		{
			CCQuaternion con = -(*this);
			CCQuaternion tmp( rhs.x, rhs.y, rhs.z, 0 );

			tmp = (*this) * tmp * con;

			for( int i=0; i<3; i++ )
				rhs[ i ] = tmp[ i ];
		}

		inline CC_REAL squre				() const
		{
			CC_REAL ret = 0;
			ret += mVal[ 0 ] * mVal[ 0 ];
			ret += mVal[ 1 ] * mVal[ 1 ];
			ret += mVal[ 2 ] * mVal[ 2 ];
			ret += mVal[ 3 ] * mVal[ 3 ];
			return ret;
		}

		inline CC_REAL length				() const
		{
			return sqrt( squre() );
		}

		inline CCQuaternion<T>& normalizeLocal () 
		{
			CC_REAL len = length();
			mVal[ 0 ] /= len;
			mVal[ 1 ] /= len;
			mVal[ 2 ] /= len;
			mVal[ 3 ] /= len;
			return (*this);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// <Type Rename>
	typedef CCQuaternion<double>				QuatD;
	typedef CCQuaternion<float>				QuatF;
	// </Type Rename>
	//////////////////////////////////////////////////////////////////////////
}

#endif
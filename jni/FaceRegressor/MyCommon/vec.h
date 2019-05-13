//	******************************************************
//	Vector
//	Classes of different vector
//	Author: Chen Cao	19/08/2013
//	******************************************************

#ifndef VECTOR_H
#define VECTOR_H

#include <assert.h>
#include <iostream>
#include <math.h>

#include "type.h"
#include "typePromot.h"

namespace cc{

	//////////////////////////////////////////////////////////////////////////
	// <Vector 2D>
	template<class T>
	class Vec2{
	public:
		T							x, y;

		typedef PROMOTE_T_TO_FLOAT CC_REAL;

		// Constructor
		Vec2						( const T xVal = 0, const T yVal = 0 ) 
								: x( xVal ), y( yVal ) 
		{}

		Vec2						( const T* valPtr )
								: x( *valPtr ), y( *(valPtr+1) )
		{}

		// Access operator
		inline T& operator[]		( const int id )
		{
			assert( id == 0 || id == 1 );
			return *(&x+id);
		}

		inline const T& operator[]	( const int id ) const
		{
			assert( id == 0 || id == 1 );
			return *(&x+id);
		}

		inline void zero			()
		{
			x = y = 0;
		}

		inline T* ptr				()
		{
			return &x;
		}

		inline const T* ptr			() const
		{
			return &x;
		}

		inline void print			() const
		{
			for( int i=0; i<2; i++ )
				std::cout << (*this)[i] << "\t";
			std::cout << std::endl << std::endl;
		}

		// Math operator
		inline Vec2 operator-		() const 
		{
			Vec2 ret( -x, -y );
			return ret;
		}

		template<typename TYPE> 
		inline Vec2& operator=		(const Vec2<TYPE>& vec)
		{
			x = vec.x;
			y = vec.y;
			return *this;
		}

		template<typename TYPE>
		inline Vec2& operator=		( const TYPE& val )
		{
			x = val;
			y = val;
			return *this;
		}

		template<typename TYPE> 
		inline Vec2& operator+=		(const Vec2<TYPE>& vec)
		{
			x += vec.x;
			y += vec.y;
			return *this;
		}

		template<typename TYPE> 
		inline Vec2& operator-=		(const Vec2<TYPE>& vec)
		{
			x -= vec.x;
			y -= vec.y;
			return *this;
		}

		template<typename TYPE>	
		inline Vec2& operator*=		(const TYPE& val)
		{
			x *= val;
			y *= val;
			return *this;
		}

		template<typename TYPE> 
		inline Vec2& operator/=		(const TYPE& val)
		{
			x /= val;
			y /= val;
			return *this;
		}

		template<typename TYPE> 
		inline Vec2 operator+		(const Vec2<TYPE>& vec) const
		{
			Vec2 ret( *this );
			ret += vec;
			return ret;
		}

		template<typename TYPE> 
		inline Vec2 operator -		(const Vec2<TYPE>& vec) const
		{
			Vec2 ret( *this );
			ret -= vec;
			return ret;
		}

		template<typename TYPE> 
		inline Vec2 operator *		(const TYPE& val) const
		{
			Vec2 ret( *this );
			ret *= val;
			return ret;
		}

		template<typename TYPE> 
		inline Vec2 operator /		(const TYPE& val) const
		{
			Vec2 ret( *this );
			ret /= val;
			return ret;
		}	

		inline CC_REAL squre		() const
		{
			return ( x * x + y * y );
		}

		inline CC_REAL length		() const
		{
			return sqrt( squre() );
		}

		inline Vec2& normalizeLocal	()
		{
			CC_REAL len = length();
			(*this) /= len;
			return *this;
		}

		template<typename TYPE>
		inline Vec2<T> myMin		( const Vec2<TYPE>& b ) const
		{
			Vec2<T> ret;

			ret.x = ( x < b.x ) ? x : b.x;
			ret.y = ( y < b.y ) ? y : b.y;

			return ret;
		}

		template<typename TYPE>
		inline Vec2<T> myMax		( const Vec2<TYPE>& b ) const
		{
			Vec2<T> ret;

			ret.x = ( x > b.x ) ? x : b.x;
			ret.y = ( y > b.y ) ? y : b.y;

			return ret;
		}
	};

	// </Vector 2D>
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// <Vector 3D>
	template<class T>
	class Vec3{
	public:
		T							x, y, z;

		typedef PROMOTE_T_TO_FLOAT CC_REAL;

		// Constructor
		Vec3						( const T xVal = 0, const T yVal = 0, const T zVal = 0 ) 
			: x( xVal ), y( yVal ), z( zVal ) 
		{}

		Vec3						( const T* valPtr )
			: x( *valPtr ), y( *(valPtr+1) ), z( *(valPtr+2) )
		{}

		// Access operator
		inline T& operator[]		( const int id )
		{
			assert( id >= 0 && id <= 2 );
			return *(&x+id);
		}

		inline const T& operator[]	( const int id ) const
		{
			assert( id >= 0 && id <= 2 );
			return *(&x+id);
		}

		inline void zero			()
		{
			x = y = z = 0;
		}

		inline T* ptr				()
		{
			return &x;
		}

		inline const T* ptr			() const
		{
			return &x;
		}

		inline void print			() const
		{
			for( int i=0; i<3; i++ )
				std::cout << (*this)[i] << "\t";
			std::cout << std::endl << std::endl;
		}

		// Math operator
		inline Vec3 operator-		() const 
		{
			Vec3 ret( -x, -y, -z );
			return ret;
		}

		template<typename TYPE> 
		inline Vec3& operator=		(const Vec3<TYPE>& vec){
			x = vec.x;
			y = vec.y;
			z = vec.z;
			return *this;
		}

		template<typename TYPE> 
		inline Vec3& operator=		( const TYPE& val )
		{
			x = val;
			y = val;
			z = val;
			return *this;
		}

		template<typename TYPE> 
		inline Vec3& operator+=		(const Vec3<TYPE>& vec){
			x += vec.x;
			y += vec.y;
			z += vec.z;
			return *this;
		}

		template<typename TYPE> 
		inline Vec3& operator-=		(const Vec3<TYPE>& vec){
			x -= vec.x;
			y -= vec.y;
			z -= vec.z;
			return *this;
		}

		template<typename TYPE> 
		inline Vec3& operator*=		(const TYPE& val){
			x *= val;
			y *= val;
			z *= val;
			return *this;
		}

		template<typename TYPE> 
		inline Vec3& operator/=		(const TYPE& val){
			x /= val;
			y /= val;
			z /= val;
			return *this;
		}

		template<typename TYPE> 
		inline Vec3 operator+		(const Vec3<TYPE>& vec) const
		{
			Vec3 ret( *this );
			ret += vec;
			return ret;
		}

		template<typename TYPE> 
		inline Vec3 operator-		(const Vec3<TYPE>& vec) const
		{
			Vec3 ret( *this );
			ret -= vec;
			return ret;
		}

		template<typename TYPE> 
		inline Vec3 operator*		(const TYPE& val) const
		{
			Vec3 ret( *this );
			ret *= val;
			return ret;
		}

		template<typename TYPE> 
		inline Vec3 operator/		(const TYPE& val) const
		{
			Vec3 ret( *this );
			ret /= val;
			return ret;
		}	

		inline CC_REAL squre		() const
		{
			return ( x * x + y * y + z * z );
		}

		inline CC_REAL length		() const
		{
			return sqrt( squre() );
		}

		inline Vec3& normalizeLocal	()
		{
			CC_REAL len = length();
			(*this) /= len;
			return *this;
		}

		template<typename TYPE>
		inline Vec3<T> myMin		( const Vec3<TYPE>& b ) const
		{
			Vec3<T> ret;
			
			ret.x = ( x < b.x ) ? x : b.x;
			ret.y = ( y < b.y ) ? y : b.y;
			ret.z = ( z < b.z ) ? z : b.z;

			return ret;
		}

		template<typename TYPE>
		inline Vec3<T> myMax		( const Vec3<TYPE>& b ) const
		{
			Vec3<T> ret;

			ret.x = ( x > b.x ) ? x : b.x;
			ret.y = ( y > b.y ) ? y : b.y;
			ret.z = ( z > b.z ) ? z : b.z;

			return ret;
		}
	};

	// </Vector 3D>
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// <Vector 4D>
	template<class T>
	class Vec4{
	public:
		T							x, y, z, w;

		typedef PROMOTE_T_TO_FLOAT CC_REAL;

		// Constructor
		Vec4						( const T xVal = 0, const T yVal = 0, const T zVal = 0, const T wVal = 0 ) 
			: x( xVal ), y( yVal ), z( zVal ), w( wVal )
		{}

		Vec4						( const T* valPtr )
			: x( *valPtr ), y( *(valPtr+1) ), z( *(valPtr+2) ), w( *(valPtr+3) )
		{}

		// Access operator
		inline T& operator[]		( const int id )
		{
			assert( id >= 0 && id <= 3 );
			return *(&x+id);
		}

		inline const T& operator[] ( const int id ) const
		{
			assert( id >= 0 && id <= 3 );
			return *(&x+id);
		}

		inline void zero			()
		{
			x = y = z = w = 0;
		}

		inline T* ptr				()
		{
			return &x;
		}

		inline const T* ptr			() const
		{
			return &x;
		}

		inline void print			() const
		{
			for( int i=0; i<4; i++ )
				std::cout << (*this)[i] << "\t";
			std::cout << std::endl << std::endl;
		}

		// Math operator
		inline Vec4 operator-		() const 
		{
			Vec4 ret( -x, -y, -z, -w );
			return ret;
		}

		template<typename TYPE> 
		inline Vec4& operator=		(const Vec4<TYPE>& vec){
			x = vec.x;
			y = vec.y;
			z = vec.z;
			w = vec.w;
			return *this;
		}

		template<typename TYPE>
		inline Vec4& operator=		( const TYPE& val )
		{
			x = val;
			y = val;
			z = val;
			w = val;
			return *this;
		}

		template<typename TYPE> 
		inline Vec4& operator+=		(const Vec4<TYPE>& vec){
			x += vec.x;
			y += vec.y;
			z += vec.z;
			w += vec.w;
			return *this;
		}

		template<typename TYPE> 
		inline Vec4& operator-=		(const Vec4<TYPE>& vec){
			x -= vec.x;
			y -= vec.y;
			z -= vec.z;
			w -= vec.w;
			return *this;
		}

		template<typename TYPE> 
		inline Vec4& operator*=		(const TYPE& val){
			x *= val;
			y *= val;
			z *= val;
			w *= val;
			return *this;
		}

		template<typename TYPE> 
		inline Vec4& operator/=		(const TYPE& val){
			x /= val;
			y /= val;
			z /= val;
			w /= val;
			return *this;
		}

		template<typename TYPE> 
		inline Vec4 operator+		(const Vec4<TYPE>& vec) const
		{
			Vec4 ret( *this );
			ret += vec;
			return ret;
		}

		template<typename TYPE> 
		inline Vec4 operator-		(const Vec4<TYPE>& vec) const
		{
			Vec4 ret( *this );
			ret -= vec;
			return ret;
		}

		template<typename TYPE> 
		inline Vec4 operator*		(const TYPE& val) const
		{
			Vec4 ret( *this );
			ret *= val;
			return ret;
		}

		template<typename TYPE> 
		inline Vec4 operator/		(const TYPE val) const
		{
			Vec4 ret( *this );
			ret /= val;
			return ret;
		}	

		inline CC_REAL squre		() const
		{
			return ( x * x + y * y + z * z + w * w );
		}

		inline CC_REAL length		() const
		{
			return sqrt( squre() );
		}

		inline Vec4& normalizeLocal	()
		{
			CC_REAL len = length();
			(*this) /= len;
			return *this;
		}

		template<typename TYPE>
		inline Vec4<T> myMin		( const Vec4<TYPE>& b ) const
		{
			Vec4<T> ret;

			ret.x = ( x < b.x ) ? x : b.x;
			ret.y = ( y < b.y ) ? y : b.y;
			ret.z = ( z < b.z ) ? z : b.z;
			ret.w = ( w < b.w ) ? w : b.w;

			return ret;
		}

		template<typename TYPE>
		inline Vec4<T> myMax		( const Vec4<TYPE>& b ) const
		{
			Vec4<T> ret;

			ret.x = ( x > b.x ) ? x : b.x;
			ret.y = ( y > b.y ) ? y : b.y;
			ret.z = ( z > b.z ) ? z : b.z;
			ret.w = ( w > b.w ) ? w : b.w;

			return ret;
		}
	};

	// </Vector 3D>
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// <Type Rename>
	typedef Vec2<double>					mydouble2;
	typedef Vec2<float>						myfloat2;
	typedef Vec2<myInt32>						myint2;
	typedef Vec2<myInt64>						mylong2;

	typedef Vec3<double>					mydouble3;
	typedef Vec3<float>						myfloat3;
	typedef Vec3<myInt32>						myint3;
	typedef Vec3<myInt64>						mylong3;
	typedef Vec3<myByte>						myByte3;

	typedef Vec4<double>					mydouble4;
	typedef Vec4<float>						myfloat4;
	typedef Vec4<myInt32>						myint4;
	typedef Vec4<myInt64>						mylong4;
	typedef Vec4<myByte>						myByte4;
	// </Type Rename>
	//////////////////////////////////////////////////////////////////////////

};

#endif
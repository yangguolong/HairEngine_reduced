//	******************************************************
//	Vector
//	Classes of different list
//	Author: Chen Cao	19/08/2013
//	******************************************************

#ifndef LIST_H
#define LIST_H

#include <assert.h>
#include <iostream>
#include <vector>
#include <stdlib.h>

#include "vec.h"

namespace cc{

//////////////////////////////////////////////////////////////////////////
// <Vector list>
template<class T>
class sVectorList{
public:
	std::vector<T>				mData;

	typedef PROMOTE_T_TO_FLOAT  CC_REAL;

	// Constructor
	sVectorList					( int dim = 0 )
	{
		mData.resize( dim );
	}

	sVectorList					( const sVectorList& copy )
	{
		mData = copy.mData;
	}

	// Access
	inline T& operator[]		( const size_t id )
	{
		return mData[ id ];
	}

	inline const T& operator[]	( const size_t id ) const
	{
		return mData[ id ];
	}

	inline void resize			( const size_t dim )
	{
		mData.resize( dim );
	}

	inline size_t size			() const
	{
		return mData.size();
	}

	inline void clear			()
	{
		mData.clear();
	}

	inline void assign			( const size_t dim, const T& val )
	{
		mData.assign( dim, val );
	}

	inline void reserve			( const size_t dim )
	{
		mData.reserve( dim );
	}

	inline void push_back		( const T& val )
	{
		mData.push_back( val );
	}

	template<typename TYPE>
	inline void random			( const TYPE& min, const TYPE& max, const T& div )
	{
		int dim = mData.size();
		TYPE range = max - min;
		T scale = 1.0 / div;
		for( int i=0; i<dim; i++ ){
			myInt64 rnd = rand() % range + min;
			mData[ i ] = T( rnd ) * scale;
		}
	}

	// Math
	template<typename TYPE>
	sVectorList& operator+=		( const sVectorList<TYPE>& add )	
	{
		assert( mData.size() == add.mData.size() );

		size_t dim = mData.size();
		for( size_t i = 0; i<dim; i++ )
			mData[ i ] += add.mData[ i ];		

		return (*this);
	}

	template<typename TYPE>
	sVectorList& operator-=		( const sVectorList<TYPE>& minus )
	{
		assert( mData.size() == minus.mData.size() );

		size_t dim = mData.size();
		for( size_t i = 0; i<dim; i++ )
			mData[ i ] += minus.mData[ i ];		

		return (*this);
	}

	template<typename TYPE>
	sVectorList& operator*=		( const TYPE& val )
	{
		size_t dim = mData.size();
		for( size_t i=0; i<dim; i++ )
			mData[ i ] *= val;

		return (*this);
	}

	template<typename TYPE>
	sVectorList& operator/=		( const TYPE& val )
	{
		size_t dim = mData.size();
		for( size_t i=0; i<dim; i++ )
			mData[ i ] /= val;

		return (*this);
	}

	template<typename TYPE>
	sVectorList operator+		( const sVectorList<TYPE>& list )
	{
		sVectorList ret( *this );
		ret += list;
		return ret;
	}

	template<typename TYPE>
	sVectorList operator-		( const sVectorList<TYPE>& list )
	{
		sVectorList ret( *this );
		ret -= list;
		return ret;
	}

	template<typename TYPE>
	sVectorList operator*		( const TYPE& val )
	{
		sVectorList ret( *this );
		ret *= val;
		return ret;
	}

	template<typename TYPE>
	sVectorList operator/		( const TYPE& val )
	{
		sVectorList ret( *this );
		ret /= val;
		return ret;
	}

	void boundingBox			( T& bboxMin, T& bboxMax ) const
	{
		int dim = mData.size();
		if( dim == 0 )	return;

		bboxMin = bboxMax = mData[ 0 ];
		for( int i=1; i<dim; i++ ){
			const T& val = mData[ i ];

			bboxMin = bboxMin.myMin( val );
			bboxMax = bboxMax.myMax( val );
		}
	}

	inline T sum				() const
	{
		T ret = 0;

		int dim = mData.size();
		if( dim == 0 )	return ret;

		for( int i=0; i<dim; i++ )
			ret += mData[ i ];

		return ret;
	}

	inline T average			() const
	{
		T ret = 0;

		int dim = mData.size();
		if( dim == 0 )	return ret;

		ret = sum() / float( dim );
		return ret;
	}

	inline T square				() const
	{
		T ret = 0;

		int dim = mData.size();
		for( int i=0; i<dim; i++ )
			ret += mData[ i ] * mData[ i ];		

		return ret;
	}

	inline T length				() const
	{
		return sqrt( square() );
	}

	inline void normalizeLocal	()
	{
		T len = length();
		(*this) /= len;
	}

	inline sVectorList<T> normalize () const
	{
		sVectorList<T> ret = (*this);
		ret.normalizeLocal();
		return ret;
	}

	inline T dot				( const sVectorList<T>& rhs ) const
	{
		T ret = 0;

		if( mData.size() != rhs.mData.size() )	return 	ret;
		
		int dim = mData.size();
		for( int i=0; i<dim; i++ )
			ret += mData[ i ] * rhs.mData[ i ];

		return ret;
	}
};
// </Vector list>
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// <Type Rename>
typedef sVectorList<float>					sFloatList;
typedef sVectorList<double>					sDoubleList;
typedef sVectorList<myInt32>					sIntList;
typedef sVectorList<myInt64>					sLongList;

typedef sVectorList<myfloat2>				sFloat2List;
typedef sVectorList<mydouble2>				sDouble2List;
typedef sVectorList<myint2>					sInt2List;
typedef sVectorList<mylong2>				sLong2List;

typedef sVectorList<myfloat3>				sFloat3List;
typedef sVectorList<mydouble3>				sDouble3List;
typedef sVectorList<myint3>					sInt3List;
typedef sVectorList<mylong3>				sLong3List;
typedef sVectorList<myByte3>				sByte3List;

typedef sVectorList<myfloat4>				sFloat4List;
typedef sVectorList<mydouble4>				sDouble4List;
typedef sVectorList<myint4>					sInt4List;
typedef sVectorList<mylong4>				sLong4List;
typedef sVectorList<myByte4>				sByte4List;
// </Type Rename>
//////////////////////////////////////////////////////////////////////////

};

#endif
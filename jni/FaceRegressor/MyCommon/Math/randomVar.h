//	******************************************************
//	randomVar
//	Classes of random variable
//	Author: Chen Cao	22/08/2013
//	******************************************************

#ifndef RANDOM_VARIABLE_H
#define RANDOM_VARIABLE_H

#include "list.h"

namespace cc{
	template<class T>
	class sRandomVar : public sVectorList<T>
	{
	public:

		// Constructor
		sRandomVar					( int dim = 0 )
			: sVectorList<T>( dim )
		{

		}

		// ----------------------------------------
		// *Math-operators
		// Variance
		T variance					() const
		{
			int dim = mData.size();
			if( dim <= 0 ) return 0;

			T aver = average();
			T total = 0;
			for( int i=0; i<dim; i++ ){
				T diff = mData[ i ] - aver;
				total += diff * diff;
			}
			total /= T( dim );
			return total;
		}

		// Standard deviation
		T standDevi					() const
		{
			return sqrt( variance() );
		}

		// Covariance
		T covariance				( sRandomVar<T>& rhs ) const
		{
			if( mData.size() != rhs.mData.size() )	return 0;

			T muX = average();
			T muY = rhs.average();

			T total = 0;
			int dim = mData.size();
			for( int i=0; i<dim; i++ ){
				T diffX = mData[ i ];
				T diffY = rhs.mData[ i ];
				total += diffX * diffY;
			}
			total /= T( dim );
			total -= muX * muY;

			return total;
		}

		// Correlation
		T correlation				( sRandomVar<T>& rhs )
		{
			if( mData.size() != rhs.mData.size() )	return 0;

			T muX = average();
			T muY = rhs.average();

			T total = 0;
			int dim = mData.size();
			for( int i=0; i<dim; i++ ){
				T diffX = mData[ i ];
				T diffY = rhs.mData[ i ];
				total += diffX * diffY;
			}
			total /= T( dim );
			total -= muX * muY;

			T sigmaX = standDevi();
			T sigmaY = rhs.standDevi();

			T tmp = sigmaX * sigmaY;
			if( abs( tmp ) < EPSLON )	return 0;

			T corr = total / tmp;
			return corr;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// <Type Rename>
	typedef sRandomVar<double>		sRandVarD;
	typedef sRandomVar<float>		sRandVarF;
	// </Type Rename>
	//////////////////////////////////////////////////////////////////////////
}

#endif
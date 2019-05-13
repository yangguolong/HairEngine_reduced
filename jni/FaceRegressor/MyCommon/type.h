//	******************************************************
//	Type
//	Classes of different types
//	Author: Chen Cao	19/08/2013
//	******************************************************

#ifndef TYPE_H
#define TYPE_H

#pragma warning( disable: 4005 )

#define EPSLON				1e-6
#define INFITE_FLOAT		3.4e38
#define INFITE_DOUBLE		1.7e308

#ifndef M_PI
#define M_PI				3.14159265
#endif

namespace cc{
	typedef signed char			myInt8;
	typedef short int			myInt16;
	typedef int					myInt32;
	typedef long int			myInt64;
	typedef unsigned char		myByte;
}

#endif
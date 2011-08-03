// $Id$

#ifndef __SOURCEACCESSOR_H__
#define __SOURCEACCESSOR_H__

#include "SourceTransform.h"
#include "Point.h"
#include "Point2D.h"
#include "Point3D.h"
#include <math.h>


/// base class for SourceAccessors. Essentially a SourceTransform with 
/// no free parameters
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorOf 
	: public SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
public:
	SourceAccessorOf() 
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
	{}

	SourceAccessorOf(SOURCE* psource) 
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> (psource)
	{}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorOf" + LINE_
			+ this->DescribeCommon()
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg){}

	static String SerializationId()
	{
		return "SourceAccessorOf<>";
	}

	virtual void SerializeSelf(Stream& st) const
	{ 
	}
	virtual void DeserializeSelf(Stream& st)
	{
	}

};


template <class DATA, class PRECISION, class SUBSOURCE>
inline void Get(
	const SourceAccessorOf<DATA,2,PRECISION,SUBSOURCE>& src, 
	DATA& dataOut, const PRECISION& nX, const PRECISION& nY
)
{
	src.VirtualGet(dataOut,nX,nY);
}

template <class DATA, class PRECISION, class SUBSOURCE>
inline void Set(
	SourceAccessorOf<DATA,2,PRECISION,SUBSOURCE>& src, 
	const PRECISION& nX, const PRECISION& nY, const DATA& data
)
{
	src.VirtualSet(nX,nY,data);
}

template <class DATA, class PRECISION, class SUBSOURCE>
inline void Get(
	const SourceAccessorOf<DATA,3,PRECISION,SUBSOURCE>& src, DATA& dataOut,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ
)
{
	src.VirtualGet(dataOut,nX,nY,nZ);
}

template <class DATA, class PRECISION, class SUBSOURCE>
inline void Set(
	SourceAccessorOf<DATA,3,PRECISION,SUBSOURCE>& src,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ, const DATA& data
)
{
	src.VirtualSet(nX,nY,nZ,data);
}



//============================================================================
// interpolation assistants
//============================================================================
/// helper macro to make an accessor with the same DATA and DIMENSIONALITY but
/// a Real PRECISION
#define MAKE_INTERPOLANT_ASSISTANT(_name,_class)															\
	MAKE_TYPECHANGE_ASSISTANT(																									\
		_name,																																		\
		_class,																																		\
		TypeOfData(SOURCE),																												\
		TypeOfDimensionality(SOURCE),																							\
		Real,																																			\
		{}																																				\
		,																																					\
	);


#endif // __SOURCEACCESSOR_H__

// $Id$

#ifndef __SOURCEACCESSORNEARESTNEIGHBOR_H__
#define __SOURCEACCESSORNEARESTNEIGHBOR_H__

#include "SourceGenerics.h"

#include <math.h>

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorNearestNeighborOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};

template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorNearestNeighborOf<DATA,2,PRECISION,SOURCE>
	: public SourceAccessorOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorNearestNeighborOf(2D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		::Get(*(this->m_psource),dataOut,round(rX),round(rY));
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		::Set(*(this->m_psource),round(rX),round(rY),data);
	}
};



template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorNearestNeighborOf<DATA,3,PRECISION,SOURCE>
	: public SourceAccessorOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorNearestNeighborOf(3D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		::Get(*(this->m_psource),	dataOut,round(rX),round(rY),round(rZ));
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ,const DATA& data)
	{
		::Set(*(this->m_psource),round(rX),round(rY),round(rZ),data);
	}

};

MAKE_INTERPOLANT_ASSISTANT( NearestNeighbor, SourceAccessorNearestNeighborOf );

#endif // __SOURCEACCESSORNEARESTNEIGHBOR_H__

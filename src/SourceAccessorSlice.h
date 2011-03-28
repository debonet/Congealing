// $Id$

#ifndef __SOURCEACCESSORSLICE_H__
#define __SOURCEACCESSORSLICE_H__

#include "SourceGenerics.h"

#include "libBaseTypes.h"
#include <math.h>


template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorSliceOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};




template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorSliceOf<DATA,2,PRECISION,SOURCE>
	: public SourceAccessorOf<DATA, 2, PRECISION, SOURCE>
{
	DimensionType m_dim;
	PRECISION m_rAlong;

public:
	SOURCE_ACTUALS_2D;

	SourceAccessorSliceOf() 
		: SourceAccessorOf<DATA, 2, PRECISION, SOURCE> ()
		, m_dim(DimensionX)
		, m_rAlong(PRECISION(0.))
	{}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorSliceOf(2D)" + LINE_ + 
			(_INDENT + "dimension " + int(m_dim) + " at " + m_rAlong + LINE_) + 
			this->DescribeCommon()
		);
	}


	void SetSlice(DimensionType dim, PRECISION rAlong) 
	{
		m_dim=dim;
		m_rAlong=rAlong;
	}

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		switch(m_dim){
		case DimensionX: return ::Get(*(this->m_psource),dataOut,m_rAlong,rX,rY);
		case DimensionY: return ::Get(*(this->m_psource),dataOut,rX,m_rAlong,rY);
		case DimensionZ: return ::Get(*(this->m_psource),dataOut,rX,rY,m_rAlong);
		default: ERROR("Bad dimension %d", m_dim);
		}
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		switch(m_dim){
		case DimensionX: return ::Set(*(this->m_psource),m_rAlong,rX,rY,data);
		case DimensionY: return ::Set(*(this->m_psource),rX,m_rAlong,rY,data);
		case DimensionZ: return ::Set(*(this->m_psource),rX,rY,m_rAlong,data);
		default: ERROR("Bad dimension %d", m_dim);
		}
	}

	void PrepareForAccessAction() const
	{
	}


	virtual void PrepareForAccess() const
	{
		// tell source to prepare first
		this->m_psource->PrepareForAccess();

		switch(m_dim){
		case DimensionX: {
			this->m_ptSize=PointOf<2,PRECISION>(this->PSource()->Size().Y(),this->PSource()->Size().Z());
			break;
		}
		case DimensionY: {
			this->m_ptSize=PointOf<2,PRECISION>(this->PSource()->Size().X(),this->PSource()->Size().Z());
			break;
		}
		case DimensionZ: {
			this->m_ptSize=PointOf<2,PRECISION>(this->PSource()->Size().X(),this->PSource()->Size().Y());
			break;
		}
		default: {
			ERROR("Bad dimension %d", m_dim);
		}
		}

		this->PrepareForAccessAction();
	}


};



//============================================================================
// slice assistant
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	Slice,
	SourceAccessorSliceOf,
	TypeOfData(SOURCE),
	TypeOfDimensionality(SOURCE)-1,
	TypeOfPrecision(SOURCE),
	{ psrcOut->SetSlice(dt,rAlong); },
	const DimensionType& dt, const TypeOfPrecision(SOURCE)& rAlong,
);

#endif // __SOURCEACCESSORSLICE_H__

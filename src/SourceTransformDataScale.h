// $Id$

#ifndef __SOURCETRANSFORMDATASCALE_H__
#define __SOURCETRANSFORMDATASCALE_H__

#include "SourceGenerics.h"

#include <math.h>

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformDataScaleBaseOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	Parameter m_rSlope;
	Parameter m_rIntercept;

public:
	typedef PointOf<DIMENSIONALITY,PRECISION> type_point;												
	typedef DATA type_data;																											
	typedef PRECISION type_precision;																						
	static const int type_dimensionality=DIMENSIONALITY;												

	SourceTransformDataScaleBaseOf()
		: SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>()
		, m_rSlope(1.)
		, m_rIntercept(0.)
	{}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceTransformDataScaleOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			(_INDENT + "slope " + m_rSlope + " intercept " + m_rIntercept + LINE_) +
			this->DescribeCommon()
		);
	}

	Parameter Slope() const
	{
		return m_rSlope;
	}

	Parameter Intercept() const
	{
		return m_rIntercept;
	}


	void SetSlopeIntercept(
		const Real& rSlope,
		const Real& rIntercept
	){
		m_rSlope=rSlope;
		m_rIntercept=rIntercept;
	}

	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		reg.Register(&m_rSlope);
		reg.Register(&m_rIntercept);
	}

	static String SerializationId()
	{
		return String("SourceTransformDataScaleOf(") + DIMENSIONALITY +"D)";
	}

	void SerializeSelf(Stream& st) const
	{ 
		::Serialize(st,m_rSlope);
		::Serialize(st,m_rIntercept);
	}

	void DeserializeSelf(Stream &st)
	{
		::Deserialize(st,m_rSlope);
		::Deserialize(st,m_rIntercept);
	}
};

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformDataScaleOf 
	: public SourceTransformDataScaleBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};

template<class DATA, class PRECISION, class SOURCE>
class SourceTransformDataScaleOf<DATA,2,PRECISION,SOURCE>
	: public SourceTransformDataScaleBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		TypeOfData(SOURCE) dataT;
		dataOut = type_data(
			::Bound(
				RANGEPROMOTION(TypeOfData(SOURCE))(
					(	Real(dataT) * this->m_rSlope ) 
					+ ( this->m_rIntercept * Range(DATA) )
				),
				RANGEPROMOTION(TypeOfData(SOURCE))(MinValue(DATA)),
				RANGEPROMOTION(TypeOfData(SOURCE))(MaxValue(DATA))));
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		ERROR("Not implemented");
	}
};



template<class DATA, class PRECISION, class SOURCE>
class SourceTransformDataScaleOf<DATA,3,PRECISION,SOURCE>
	: public SourceTransformDataScaleBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		TypeOfData(SOURCE) dataT;
		::Get(*(this->m_psource),dataT,rX,rY,rZ);
		dataOut = type_data(
			::Bound(
				RANGEPROMOTION(TypeOfData(SOURCE))(
					(	Real(dataT) * this->m_rSlope ) 
					+ ( this->m_rIntercept * Range(DATA) )
				),
				RANGEPROMOTION(TypeOfData(SOURCE))(MinValue(DATA)),
				RANGEPROMOTION(TypeOfData(SOURCE))(MaxValue(DATA))));
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ,const DATA& data)
	{
		ERROR("Not implemented");
	}

};


#include "Pixels.h"

//============================================================================
// DataScale
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	GreyPixelDataScale,
	SourceTransformDataScaleOf,
	Pixel8BitGrey,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{ psrcOut->SetSlopeIntercept(rSlope,rIntercept); },
	const Real &rSlope,
	const Real &rIntercept,
);

//============================================================================
// DataScale
//============================================================================
MAKE_ASSISTANT(
	DataScale,
	SourceTransformDataScaleOf,
	{ psrcOut->SetSlopeIntercept(rSlope,rIntercept); },
	const Real &rSlope,
	const Real &rIntercept,
);

#endif // __SOURCETRANSFORMDATASCALE_H__

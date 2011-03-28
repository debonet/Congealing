// $Id$

#ifndef __SOURCEACCESSORRANGESCALE_H__
#define __SOURCEACCESSORRANGESCALE_H__

#include "SourceGenerics.h"

#include <math.h>

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorRangeScaleBaseOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	mutable TypeOfData(SOURCE) m_dataMean;
	mutable TypeOfData(SOURCE) m_dataRange;

public:
	typedef PointOf<DIMENSIONALITY,PRECISION> type_point;												
	typedef DATA type_data;																											
	typedef PRECISION type_precision;																						
	static const int type_dimensionality=DIMENSIONALITY;												

	void SetRangeScale(
		const TypeOfData(SOURCE)& dataMean,  
		const TypeOfData(SOURCE)& dataRange
	){
		m_dataMean=dataMean;
		m_dataRange=dataRange;
		}

	// compute the mean and range by 
	// assuming a normal distribution and picks
	// a range of 3 standard deviations about the mean
	void SetNormalRangeScale(
		const Real rSampleFraction, const TypeOfData(SOURCE)& dataThreshold
	){
		this->PrepareForAccess();

		int c=this->CSize();
		int cSamples=c*rSampleFraction;

		type_point pt;

		RANGEPROMOTION(TypeOfData(SOURCE)) dataSum(0);
		RANGEPROMOTION(TypeOfData(SOURCE)) dataSumSq(0);

		int cCounted=0;
		for (int n=0; n<cSamples; n++){
			pt=NewRandomPoint(this->m_ptSize);
			TypeOfData(SOURCE) data;
			::GetPoint(*(this->m_psource),data,pt);
			if (data > dataThreshold){
				dataSum=dataSum + data;
				dataSumSq=dataSumSq + sqr(data);
				cCounted++;
			}
		}
		cSamples=cCounted;
		m_dataMean=(dataSum/cSamples);
		m_dataRange=3*(sqrt(dataSumSq/cSamples-sqr(dataSum/cSamples)+1));
	}

	void SetMinMaxRangeScale(
		const Real rSampleFraction, const TypeOfData(SOURCE)& dataThreshold
	){
		this->PrepareForAccess();

		int c=this->CSize();
		int cSamples=c*rSampleFraction;

		type_point pt;

		RANGEPROMOTION(TypeOfData(SOURCE)) dataSum(0);
		RANGEPROMOTION(TypeOfData(SOURCE)) dataSumSq(0);

		TypeOfData(SOURCE) dataMin=MaxValue(TypeOfData(SOURCE));
		TypeOfData(SOURCE) dataMax=MinValue(TypeOfData(SOURCE));
		int cCounted=0;
		for (int n=0; n<cSamples; n++){
			pt=NewRandomValue(this->m_ptSize);
			TypeOfData(SOURCE) data;
			::GetPoint(*(this->m_psource),data,pt);
			if (data > dataMax){
				dataMax=data;
			}
			if (data < dataMin){
				dataMin=data;
			}
		}
		m_dataMean=(dataMax+dataMin)/2;
		m_dataRange=(dataMax-dataMin);
	}
};

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorRangeScaleOf 
	: public SourceAccessorRangeScaleBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};

template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorRangeScaleOf<DATA,2,PRECISION,SOURCE>
	: public SourceAccessorRangeScaleBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorRangeScaleOf(2D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		TypeOfData(SOURCE) dataT;
		::Get(*(this->m_psource),dataT,rX,rY);
		dataOut = type_data(
			::Bound(
				(
					(
						Real(dataT)
						- this->m_dataMean
					) 
					/ this->m_dataRange 
					* Range(DATA) 
					+ MidValue(DATA) 
				),
				Real(MinValue(DATA)),
				Real(MaxValue(DATA))
			)
		);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		ERROR("Not implemented");
	}
};



template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorRangeScaleOf<DATA,3,PRECISION,SOURCE>
	: public SourceAccessorRangeScaleBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorRangeScaleOf(3D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		TypeOfData(SOURCE) dataT;
		::Get(*(this->m_psource),dataT,rX,rY,rZ);
		dataOut = type_data(
			::Bound(
				RANGEPROMOTION(TypeOfData(SOURCE))(
					(
						Real(dataT)
						- this->m_dataMean
					) 
					/ this->m_dataRange 
					* Range(DATA) 
					+ MidValue(DATA) 
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
// RangeScale
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	GreyPixelRangeScale,
	SourceAccessorRangeScaleOf,
	Pixel8BitGrey,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{ psrcOut->SetRangeScale(dataMean,dataRange); },
	const TypeOfData(SOURCE)& dataMean,  
	const TypeOfData(SOURCE)& dataRange,
);

//============================================================================
// RangeScale
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	GreyPixelNormalRangeScale,
	SourceAccessorRangeScaleOf,
	Pixel8BitGrey,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{ psrcOut->SetNormalRangeScale(rSampleFraction,dataThreshold); },
	const Real& rSampleFraction, const TypeOfData(SOURCE)& dataThreshold,
);

//============================================================================
// RangeScale
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	GreyPixelMinMaxRangeScale,
	SourceAccessorRangeScaleOf,
	Pixel8BitGrey,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{ psrcOut->SetMinMaxRangeScale(rSampleFraction,dataThreshold); },
	const Real& rSampleFraction, const TypeOfData(SOURCE)& dataThreshold,
);

#endif // __SOURCEACCESSORRANGESCALE_H__

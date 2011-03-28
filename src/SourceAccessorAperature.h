// $Id$

#ifndef __SOURCEACCESSORAPERATURE_H__
#define __SOURCEACCESSORAPERATURE_H__

#include "SourceGenerics.h"
#include "libSerialize.h"

#include "Promotion.h"
#include <math.h>

//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorAperatureBaseOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	PointOf<DIMENSIONALITY,PRECISION> m_ptAperatureOffset;
	PointOf<DIMENSIONALITY,PRECISION> m_ptAperatureSize;

public:
	SourceAccessorAperatureBaseOf()
		: SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>()
		, m_ptAperatureOffset(0)
		, m_ptAperatureSize(0)
	{
	}

	void SetAperature(
		const PointOf<DIMENSIONALITY,PRECISION>& ptAperatureOffset,
		const PointOf<DIMENSIONALITY,PRECISION>& ptAperatureSize
	){
		m_ptAperatureOffset=ptAperatureOffset;
		m_ptAperatureSize=ptAperatureSize;
	}

	virtual String Describe() const
	{
		return (
			_LINE + SerializationId() + LINE_ + 
			(_INDENT + "with offset:" + m_ptAperatureOffset.Describe() + LINE_ ) + 
			(_INDENT + "and  size:" + m_ptAperatureSize.Describe() + LINE_ ) + 
			this->DescribeCommon()
		);
	}

	void PrepareForAccessAction() const
	{
		// report the aperature size as the actual size
		this->m_ptSize=m_ptAperatureSize;
	}

	static String SerializationId()
	{
		return String("SourceAccessorAperatureOf(") + DIMENSIONALITY +"D)";
	}

	void SerializeSelf(Stream &st) const
	{ 
		::Serialize(st,m_ptAperatureOffset);
		::Serialize(st,m_ptAperatureSize);
	}

	void DeserializeSelf(Stream &st)
	{
		::Deserialize(st,m_ptAperatureOffset);
		::Deserialize(st,m_ptAperatureSize);
	}

};

//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorAperatureOf 
	: public SourceAccessorAperatureBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};


//============================================================================
// 2D
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorAperatureOf<DATA, 2, PRECISION, SOURCE>
	: public SourceAccessorAperatureBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		::Get(
			*this->PSource()
			, dataOut
			, this->m_ptAperatureOffset.X()+rX
			, this->m_ptAperatureOffset.Y()+rY
		);
	}


	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		::Set(
			*this->PSource()
			, this->m_ptAperatureOffset.X()+rX
			, this->m_ptAperatureOffset.Y()+rY
			, data
		);
	}

};





//============================================================================
// 3d
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorAperatureOf<DATA, 3, PRECISION, SOURCE>
	: public SourceAccessorAperatureBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		::Get(
			*this->PSource()
			, dataOut
			, this->m_ptAperatureOffset.X()+rX
			, this->m_ptAperatureOffset.Y()+rY
			, this->m_ptAperatureOffset.Z()+rZ
		);
	}


	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		::Set(
			*this->PSource()
			, this->m_ptAperatureOffset.X()+rX
			, this->m_ptAperatureOffset.Y()+rY
			, this->m_ptAperatureOffset.Z()+rZ
			, data
		);
	}

};



//============================================================================
// aperature assistant
//============================================================================
MAKE_ASSISTANT(
	Aperature,
	SourceAccessorAperatureOf,
	{ psrcOut->SetAperature(ptAperatureOffset, ptAperatureSize); }, 
	const PointOf<TypeOfDimensionality(SOURCE),TypeOfPrecision(SOURCE)>& ptAperatureOffset,
	const PointOf<TypeOfDimensionality(SOURCE),TypeOfPrecision(SOURCE)>& ptAperatureSize,
);



#endif // __SOURCEACCESSORAPERATURE_H__

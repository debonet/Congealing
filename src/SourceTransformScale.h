// $Id$

#ifndef __SOURCETRANSFORMSCALE_H__
#define __SOURCETRANSFORMSCALE_H__

#include "SourceGenerics.h"
#include <math.h>


//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformScaleBaseOf 
	: public SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	typedef PointOf<DIMENSIONALITY, Real> POINT;
	POINT m_ptScale;

public:
	SourceTransformScaleBaseOf()
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
		, m_ptScale(1.)
	{	}

	virtual String Describe() const
	{
		return (
			_LINE + SerializationId() + LINE_ + 
			(_INDENT + "scale factors: " + m_ptScale.Describe() + LINE_) + 
			this->DescribeCommon()
		);
	}


	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		RegisterPointAsParameter(reg,m_ptScale);
	}

	void SetScale(const POINT& pt)
	{
		m_ptScale=pt;
	}

	const POINT& Scale() const
	{
		return m_ptScale;
	}


	void PrepareForAccessAction() const
	{
		this->m_ptSize=this->m_psource->Size() * m_ptScale;
	}


	static String SerializationId()
	{
		return String("SourceTransformScale(") + DIMENSIONALITY +"D)";
	}

	void SerializeSelf(Stream& st) const
	{ 
		::Serialize(st,m_ptScale);
	}

	void DeserializeSelf(Stream &st)
	{
		::Deserialize(st,m_ptScale);
	}
};



//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformScaleOf 
	: public SourceTransformScaleBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};




//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformScaleOf<DATA,2,PRECISION,SOURCE>
	: public SourceTransformScaleBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:

	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		::Get(
			*(this->m_psource),
			dataOut, 
			(rX/this->m_ptScale.X()),
			(rY/this->m_ptScale.Y())
		);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		::Set(
			*(this->m_psource),
			(rX/this->m_ptScale.X()),
			(rY/this->m_ptScale.Y()),
			data);
	}
	
};





//============================================================================
// 3d
//============================================================================
//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformScaleOf<DATA,3,PRECISION,SOURCE>
	: public SourceTransformScaleBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		::Get(
			*(this->m_psource),
			dataOut, 
			(rX/this->m_ptScale.X()),
			(rY/this->m_ptScale.Y()),
			(rZ/this->m_ptScale.Z())
		);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		::Set(
			*(this->m_psource),
			(rX/this->m_ptScale.X()),
			(rY/this->m_ptScale.Y()),
			(rZ/this->m_ptScale.Z()),
			data);
	}
};




//============================================================================
// scale assistants
//============================================================================
MAKE_ASSISTANT(
	ScaleBy,
	SourceTransformScaleOf, 
	{ psrcOut->SetScale(pt); }, 
	const PointOf<TypeOfDimensionality(SOURCE),Real>& pt,
);

MAKE_ASSISTANT(
	ScaleTo,
	SourceTransformScaleOf, 
	{ 
		psrcIn->PrepareForAccess();
		psrcOut->SetScale(pt/psrcIn->Size()); 
	}, 
	const PointOf<TypeOfDimensionality(SOURCE),TypeOfPrecision(SOURCE)>& pt,
);

MAKE_ASSISTANT(
	ScaleUpto,
	SourceTransformScaleOf, 
	{ 
		psrcIn->PrepareForAccess();
		int nDim=(pt/psrcIn->Size()).MinDimension(); 
		psrcOut->SetScale(
			PointOf<TypeOfDimensionality(SOURCE),TypeOfPrecision(SOURCE)>(
				(pt/psrcIn->Size()).Dim(nDim)
			)
		);
	}, 
	const PointOf<TypeOfDimensionality(SOURCE),TypeOfPrecision(SOURCE)>& pt,
);

#endif // __SOURCETRANSFORMSCALE_H__

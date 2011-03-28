// $Id$

#ifndef __SOURCETRANSFORMTRANSLATE_H__
#define __SOURCETRANSFORMTRANSLATE_H__

#include "SourceGenerics.h"
#include <math.h>


//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformTranslateBaseOf 
	: public SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	typedef PointOf<DIMENSIONALITY, Real> POINT;
	POINT m_dpt;

public:
	SourceTransformTranslateBaseOf()
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
		, m_dpt(0.)
	{	}

	virtual String Describe() const
	{
		return (
			_LINE + SerializationId() + LINE_ + 
			(_INDENT + "translation by: " + m_dpt.Describe() + LINE_) +
			this->DescribeCommon()
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		RegisterPointAsParameter(reg,m_dpt);
	}

	POINT& Translate()
	{
		return m_dpt;
	}

	const POINT& Translate() const
	{
		return m_dpt;
	}

	static String SerializationId()
	{
		return String("SourceTransformTranslate(") + DIMENSIONALITY +"D)";
	}


	void SerializeSelf(Stream &st) const
	{ 
		::Serialize(st,m_dpt);
	}

	void DeserializeSelf(Stream &st)
	{
		::Deserialize(st,m_dpt);
	}
};



//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformTranslateOf 
	: public SourceTransformTranslateBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
	SourceTransformTranslateOf()
		: SourceTransformTranslateBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
	{}
	SourceTransformTranslateOf(SOURCE* psource) 
		: SourceTransformTranslateBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> (psource)
	{}

	// TODO
};




//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformTranslateOf<DATA,2,PRECISION,SOURCE>
	: public SourceTransformTranslateBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

	inline void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		::Get(
			*(this->m_psource),
			dataOut, 
			(rX-this->m_dpt.X()*this->m_ptSize.X()), 
			(rY-this->m_dpt.Y()*this->m_ptSize.Y())
		);
	}

	inline void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		::Set(
			*(this->m_psource),
			(rX-this->m_dpt.X()*this->m_ptSize.X()), 
			(rY-this->m_dpt.Y()*this->m_ptSize.Y()),
			data
		);
	}
	
};






//============================================================================
// 3d
//============================================================================
//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformTranslateOf<DATA,3,PRECISION,SOURCE>
	: public SourceTransformTranslateBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	inline void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		::Get(
			*(this->m_psource),
			dataOut, 
			(rX-this->m_dpt.X()*this->m_ptSize.X()), 
			(rY-this->m_dpt.Y()*this->m_ptSize.Y()),
			(rZ-this->m_dpt.Z()*this->m_ptSize.Z())
		);
	}

	inline void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		::Set(
			*(this->m_psource),
			(rX-this->m_dpt.X()*this->m_ptSize.X()), 
			(rY-this->m_dpt.Y()*this->m_ptSize.Y()),
			(rZ-this->m_dpt.Z()*this->m_ptSize.Z()),
			data
		);
	}
};


//============================================================================
// translate assistant
//============================================================================
MAKE_ASSISTANT(
	Translate, 
	SourceTransformTranslateOf, 
	{ psrcOut->Translate()=pt; }, 
	const PointOf<TypeOfDimensionality(SOURCE) COMMA TypeOfPrecision(SOURCE)> &pt,
);


#endif // __SOURCETRANSFORMTRANSLATE_H__

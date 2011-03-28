// $Id$

#ifndef __SOURCETRANSFORMCONTRAST_H__
#define __SOURCETRANSFORMCONTRAST_H__

#include "SourceGenerics.h"
#include <math.h>


// IDEA: 
//   consider using sigmoidal conversion
//
// formula from
//  
//    http://www.imagemagick.org/Usage/color/#sigmoidal
//
//	(1/(1+Exp(B(A-U))) - 1/(1+Exp(B))) / (1/(1+Exp(B(A-U))/(1+Exp(B))))


//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformContrastOf 
	: public SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
	SourceTransformContrastOf()
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
	{}
	SourceTransformContrastOf(SOURCE* psource) 
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> (psource)
	{}

	// TODO
};




//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformContrastOf<DATA,2,PRECISION,SOURCE>
	: public SourceTransformOf<DATA, 2, PRECISION, SOURCE>
{
	typedef PROMOTION(DATA,Real) FACTOR ;
	FACTOR m_rFactor;
	mutable FACTOR m_rExpFactor;

public:
	SOURCE_ACTUALS_2D;

	SourceTransformContrastOf()
		: SourceTransformOf<DATA, 2, PRECISION, SOURCE> ()
		, m_rFactor(FACTOR(0))
	{	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceTransformContrastOf(2D)" + LINE_ + 
			(_INDENT + "factors " + m_rFactor + LINE_) + 
			this->DescribeCommon()
		);
	}


	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		// HACK: need to add some sort of Registration functionality
		reg.Register(&m_rFactor);
	}

	void SetContrast(FACTOR factor)
	{
		m_rFactor=factor;
	}

	const FACTOR& GetContrast() const
	{
		return m_rFactor;
	}

	void Get(DATA& dataOut, PRECISION rX, PRECISION rY) const
	{
		::Get(this->m_psource,dataOut, rX,rY);

		int z = SignOfDiff(dataOut,MidValue(DATA));
		DATA d = AbsDiff(dataOut,MidValue(DATA));

		dataOut = z	* pow(d,m_rExpFactor) + MidValue(DATA);
	}

	void Set(PRECISION rX, PRECISION rY, const DATA& data)
	{
		int z = SignOfDiff(data,MidValue(DATA));
		DATA d = AbsDiff(data,MidValue(DATA));

		DATA dataOut = z	* pow(d,-m_rExpFactor) + MidValue(DATA);

		::Set(this->m_psource,rX, rY, dataOut);
	}

	void PrepareForAccessAction() const
	{
		this->m_ptSize=this->PSource()->Size();
		m_rExpFactor=exp(m_rFactor);
	}

};






//============================================================================
// 3d
//============================================================================
//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformContrastOf<DATA,3,PRECISION,SOURCE>
	: public SourceTransformOf<DATA, 3, PRECISION, SOURCE>
{
	typedef PROMOTION(DATA,Real) FACTOR ;
	FACTOR m_rFactor;
	mutable FACTOR m_rExpFactor;

public:
	SOURCE_ACTUALS_3D;

	SourceTransformContrastOf()
		: SourceTransformOf<DATA, 3, PRECISION, SOURCE> ()
		, m_rFactor(FACTOR(0))
	{	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceTransformContrastOf(3D)" + LINE_ + 
			(_INDENT + "factors " + m_rFactor + LINE_) + 
			this->DescribeCommon()
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		reg.Register(&m_rFactor);
	}

	void SetContrast(FACTOR factor)
	{
		m_rFactor=factor;
	}

	const FACTOR& GetContrast() const
	{
		return m_rFactor;
	}

	void Get(DATA& dataOut, PRECISION rX, PRECISION rY, PRECISION rZ) const
	{
		::Get(this->m_psource,dataOut, rX,rY,rZ);

		PROMOTION(DATA,Real) d = (
			2.
			 * (1. * dataOut - MidValue(DATA))
			/ Range(DATA)
		);
		int z = sign(d);


		dataOut = z	* pow(z*d,m_rExpFactor) * Range(DATA) / 2 + MidValue(DATA);

	}

	void Set(PRECISION rX, PRECISION rY, PRECISION rZ, const DATA& data)
	{
		PROMOTION(DATA,Real) d = (
			2.
			 * (1. * data - MidValue(DATA))
			/ Range(DATA)
		);
		int z = sign(d);

		DATA dataOut = z	* pow(z*d,-m_rExpFactor) * Range(DATA) / 2 + MidValue(DATA);

		::Set(this->m_psource,rX, rY, rZ, dataOut);
	}

	void PrepareForAccessAction() const
	{
		this->m_ptSize=this->PSource()->Size();
		m_rExpFactor=exp(-m_rFactor);
	}
};

#endif // __SOURCETRANSFORMCONTRAST_H__

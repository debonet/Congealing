// $Id$

#ifndef __SOURCEMASKCOMBINESELECT_H__
#define __SOURCEMASKCOMBINESELECT_H__

#include "SourceGenerics.h"
#include "SourceMaskCombine.h"
#include "Promotion.h"
#include "libUtility.h"
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
class SourceMaskCombineSelectOf
	: public SourceMaskCombineOf<DATA, DIMENSIONALITY, PRECISION, SOURCEMASK,SOURCECOMBINE>
{
}

//============================================================================
// 2D
//============================================================================
template<class DATA, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
class SourceMaskCombineSelectOf<DATA, 2, PRECISION, SOURCEMASK,SOURCECOMBINE>
	: public SourceMaskCombineOf<DATA, 2, PRECISION, SOURCEMASK,SOURCECOMBINE>
{
public:
	virtual String Describe() const
	{
		return (
			_LINE + "SourceMaskCombineSelectOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	SOURCEMASK,SOURCECOMBINE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		TypeOfData(SOURCEMASK) dataMask;
		::Get(*this->psourceMask, dataMask, rX, rY);

		::Get(*this->PSourceCombine(dataMask), dataOut, rX, rY);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		TypeOfData(SOURCEMASK) dataMask;
		::Get(*this->psourceMask, dataMask, rX, rY);

		::Set(*this->PSourceCombine(dataMask), rX, rY, data);
	}
};


//============================================================================
// 3d
//============================================================================
template<class DATA, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
class SourceMaskCombineSelectOf<DATA, 3, PRECISION, SOURCEMASK,SOURCECOMBINE>
	: public SourceMaskCombineOf<TypeOfData(DATA), 3, PRECISION, SOURCEMASK,SOURCECOMBINE>
{
	typedef TypeOfData(DATA) DATA;
public:
	SOURCEMASK,SOURCECOMBINE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		TypeOfData(SOURCEMASK) dataMask;
		::Get(*this->psourceMask, dataMask, rX, rY, rZ);

		::Get(*this->PSourceCombine(dataMask), dataOut, rX, rY, rZ);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		TypeOfData(SOURCEMASK) dataMask;
		::Get(*this->psourceMask, dataMask, rX, rY, rZ);

		::Set(*this->PSourceCombine(dataMask), rX, rY, rZ, data);
	}

};


#endif




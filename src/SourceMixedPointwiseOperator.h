// $Id$

#ifndef __SOURCEMIXEDPOINTWISEOPERATOR_H__
#define __SOURCEMIXEDPOINTWISEOPERATOR_H__

#include "SourceGenerics.h"
#include "Promotion.h"



//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE1, class SOURCE2>
class SourceMixedPointwiseOperatorBaseOf 
	: public SourceMixedOf<DATA, DIMENSIONALITY, PRECISION, SOURCE1, SOURCE2>
{
protected:
public:
	virtual String Describe() const
	{
		return (
			_LINE + "SourceMixedPointwiseOperatorOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			this->DescribeCommon()
		);
	}
};

//============================================================================
//============================================================================
template<class OPERATOR, int DIMENSIONALITY, class PRECISION, class SOURCE1, class SOURCE2>
class SourceMixedPointwiseOperatorOf 
	: public SourceMixedOf<typename OPERATOR::type_data, DIMENSIONALITY, PRECISION, SOURCE1, SOURCE2>
{
};


//============================================================================
// 2D
//============================================================================
template<class OPERATOR, class PRECISION, class SOURCE1, class SOURCE2>
class SourceMixedPointwiseOperatorOf<OPERATOR, 2, PRECISION, SOURCE1, SOURCE2>
	: public SourceMixedPointwiseOperatorBaseOf<typename OPERATOR::type_data, 2, PRECISION, SOURCE1, SOURCE2>
{
	typedef typename OPERATOR::type_data DATA;
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		TypeOfData(SOURCE1) dataSource1;
		TypeOfData(SOURCE2) dataSource2;

		::Get(*this->PSource1(), dataSource1, rX, rY);
		::Get(*this->PSource2(), dataSource2, rX, rY);

		OPERATOR::FromSource(dataOut, dataSource1, dataSource2);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		TypeOfData(SOURCE1) dataSource1;
		TypeOfData(SOURCE2) dataSource2;

		OPERATOR::ToSource(dataSource1, dataSource2, data);

		::Set(*this->PSource1(),rX, rY, dataSource1);
		::Set(*this->PSource2(),rX, rY, dataSource2);
	}
};


//============================================================================
// 3d
//============================================================================
template<class OPERATOR, class PRECISION, class SOURCE>
class SourceMixedPointwiseOperatorOf<OPERATOR, 3, PRECISION, SOURCE>
	: public SourceMixedPointwiseOperatorBaseOf<typename OPERATOR::type_data, 3, PRECISION, SOURCE>
{
	typedef typename OPERATOR::type_data DATA;

public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		TypeOfData(SOURCE1) dataSource1;
		TypeOfData(SOURCE2) dataSource2;

		::Get(*this->PSource1(), dataSource1, rX, rY, rZ);
		::Get(*this->PSource2(), dataSource2, rX, rY, rZ);

		OPERATOR::FromSource(dataOut, dataSource1, dataSource2);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		TypeOfData(SOURCE1) dataSource1;
		TypeOfData(SOURCE2) dataSource2;

		OPERATOR::ToSource(dataSource1, dataSource2, data);

		::Set(*this->PSource1(),rX, rY, rZ, dataSource1);
		::Set(*this->PSource2(),rX, rY, rZ, dataSource2);
	}

};






//============================================================================
// OperatorLookupDimensionOf
//============================================================================
// the value in source1 is used to select the dimension in source2
template <class T1, class T2>
class OperatorLookupDimensionOf
{
public:
};

//----------------------------------------------------------------------------
// OperatorMaximumDimension for Point256DReal -> Pixel8BitGrey
//----------------------------------------------------------------------------
template <>
class OperatorLookupDimensionOf<Pixel8BitGrey, Point256DReal>
{
public:
	typedef PixelRealGrey type_data;

	static void FromSource(PixelRealGrey& dataOut, const Pixel8BitGrey& dataSource1,  const Point256DReal& dataSource2){
		dataOut=dataSource2.GetDimension(dataSource1);
	}
	static void ToSource(Pixel8BitGrey& dataSource1,  Point256DReal& dataSource2,const PixelRealGrey& dataOut){
		// one might contemplate searching through the dimensions for a matching, or closest matching value
		ERROR("NOT IMPLEMENTED");
	}
};


template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE1, class SOURCE2>
class TypeSourceMixedLookupDimensionOf
{
public: 
	typedef SourceMixedPointwiseOperatorOf<
		OperatorLookupDimensionOf<TypeOfData(SOURCE1), TyoeOfData(SOURCE2)>::type,
		DIMENSIONALITY,PRECISION,SOURCE1, SOURCE2
	> type;
};

#define SourceMixedLookupDimensionOf(_data,_dims,_precision,_src1,_src2)			\
	typename TypeSourceMixedLookupDimensionOf<_data,_dims,_precision,_src1,_src2>::type





#endif 



// $Id$

#ifndef __SOURCEACCESSORPOINTWISEOPERATOR_H__
#define __SOURCEACCESSORPOINTWISEOPERATOR_H__

#include "SourceGenerics.h"
#include "Promotion.h"



//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorPointwiseOperatorBaseOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
public:
	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorPointwiseOperatorOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			this->DescribeCommon()
		);
	}
};

//============================================================================
//============================================================================
template<class OPERATOR, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorPointwiseOperatorOf 
	: public SourceAccessorOf<typename OPERATOR::type_data, DIMENSIONALITY, PRECISION, SOURCE>
{
};


//============================================================================
// 2D
//============================================================================
template<class OPERATOR, class PRECISION, class SOURCE>
class SourceAccessorPointwiseOperatorOf<OPERATOR, 2, PRECISION, SOURCE>
	: public SourceAccessorPointwiseOperatorBaseOf<typename OPERATOR::type_data, 2, PRECISION, SOURCE>
{
	typedef typename OPERATOR::type_data DATA;
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		typename SOURCE::type_data dataSource;

		::Get(*this->PSource(), dataSource, rX, rY);

		OPERATOR::FromSource(dataOut, dataSource);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		typename SOURCE::type_data dataSource;

		OPERATOR::ToSource(dataSource, data);

		::Set(*this->PSource(),rX, rY, dataSource);
	}
};


//============================================================================
// 3d
//============================================================================
template<class OPERATOR, class PRECISION, class SOURCE>
class SourceAccessorPointwiseOperatorOf<OPERATOR, 3, PRECISION, SOURCE>
	: public SourceAccessorPointwiseOperatorBaseOf<typename OPERATOR::type_data, 3, PRECISION, SOURCE>
{
	typedef typename OPERATOR::type_data DATA;

public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		typename SOURCE::type_data dataSource;

		::Get(*this->PSource(), dataSource, rX, rY, rZ);

		OPERATOR::FromSource(dataOut, dataSource);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		typename SOURCE::type_data dataSource;

		OPERATOR::ToSource(dataSource, data);

		::Set(*this->PSource(), rX, rY, rZ, dataSource);
	}

};






//============================================================================
// OperatorMaximumDimension
//============================================================================
// controls how a pixel of some random type is converted into a 
// distribution field
template <class T>
class OperatorMaximumDimensionOf
{
public:
};



//============================================================================
// OperatorMaximumDimension for Point256DReal -> Pixel8BitGrey
//============================================================================
template <>
class OperatorMaximumDimensionOf<Point256DReal>
{
public:
	typedef Pixel8BitGrey type_data;

	static void FromSource(Pixel8BitGrey& dataOut, const Point256DReal& dataSource){
		dataOut=dataSource.MaxDimension();
	}
	static void ToSource(Point256DReal& dataSource, const Pixel8BitGrey& dataOut){
		ERROR("NOT IMPLEMENTED");
	}
};


//============================================================================
// MaximumDimension
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	MaximumDimension,
	SourceAccessorPointwiseOperatorOf,
	OperatorMaximumDimensionOf<TypeOfData(SOURCE)>,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{}
);

#endif 



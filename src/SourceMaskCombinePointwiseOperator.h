// $Id$

#ifndef __SOURCEMASKCOMBINEPOINTWISEOPERATOR_H__
#define __SOURCEMASKCOMBINEPOINTWISEOPERATOR_H__

#include "SourceGenerics.h"
#include "SourceMaskCombine.h"
#include "Promotion.h"
#include "libUtility.h"


//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
class SourceMaskCombinePointwiseOperatorBaseOf 
	: public SourceMaskCombineOf<DATA, DIMENSIONALITY, PRECISION, SOURCEMASK, SOURCECOMBINE>
{
protected:
	mutable TypeOfData(SOURCECOMBINE)* m_vdata;
	mutable int m_cdata;
public:
	SourceMaskCombinePointwiseOperatorBaseOf()
		: SourceMaskCombineOf<DATA, DIMENSIONALITY, PRECISION, SOURCEMASK,SOURCECOMBINE>()
	{
		m_vdata=NULL;
		m_cdata=0;
	}

	virtual ~SourceMaskCombinePointwiseOperatorBaseOf()
	{
		SafeDeleteArray(m_vdata);
	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceMaskCombinePointwiseOperatorOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	virtual void PrepareForAccessAction() const
	{
 		SourceMaskCombineOf<DATA, DIMENSIONALITY, PRECISION, SOURCEMASK,SOURCECOMBINE>::PrepareForAccessAction();
		if (m_cdata!=this->CSourceCombine()){
			SafeDeleteArray(m_vdata);
			m_cdata=this->CSourceCombine();
			m_vdata=new TypeOfData(SOURCECOMBINE)[m_cdata];
		}
	}

};

//============================================================================
//============================================================================
template<class OPERATOR, int DIMENSIONALITY, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
class SourceMaskCombinePointwiseOperatorOf 
	: public SourceMaskCombineOf<TypeOfData(OPERATOR), DIMENSIONALITY, PRECISION, SOURCEMASK,SOURCECOMBINE>
{
};


//============================================================================
// 2D
//============================================================================
template<class OPERATOR, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
class SourceMaskCombinePointwiseOperatorOf<OPERATOR, 2, PRECISION, SOURCEMASK,SOURCECOMBINE>
	: public SourceMaskCombinePointwiseOperatorBaseOf<TypeOfData(OPERATOR), 2, PRECISION, SOURCEMASK,SOURCECOMBINE>
{
	typedef TypeOfData(OPERATOR) DATA;
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		TypeOfData(SOURCEMASK) dataMask;
		::Get(*this->PSourceMask(), dataMask, rX, rY);
		
		for (int n=0; n<this->m_cdata; n++){
			::Get(*this->PSourceCombine(n), this->m_vdata[n], rX, rY);
		}

		OPERATOR::FromSource(dataOut, dataMask, this->m_vdata, this->m_cdata);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		TypeOfData(SOURCEMASK) dataMask;
		OPERATOR::ToSource(dataMask, this->m_vdata, this->m_cdata, data);

		::Set(*this->PSourceMask(),rX, rY, dataMask);
		for (int n=0; n<this->m_cdata; n++){
			::Set(*this->PSourceCombine(n),rX, rY, this->m_vdata[n]);
		}
	}
};


//============================================================================
// 3d
//============================================================================
template<class OPERATOR, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
class SourceMaskCombinePointwiseOperatorOf<OPERATOR, 3, PRECISION, SOURCEMASK,SOURCECOMBINE>
	: public SourceMaskCombinePointwiseOperatorBaseOf<TypeOfData(OPERATOR), 3, PRECISION, SOURCEMASK,SOURCECOMBINE>
{
	typedef TypeOfData(OPERATOR) DATA;
public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		TypeOfData(SOURCEMASK) dataMask;
		::Get(*this->PSourceMask(), dataMask, rX, rY, rZ);
		
		for (int n=0; n<this->m_cdata; n++){
			::Get(*this->PSourceCombine(n), this->m_vdata[n], rX, rY, rZ);
		}

		OPERATOR::FromSource(dataOut, dataMask, this->m_vdata, this->m_cdata);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		TypeOfData(SOURCEMASK) dataMask;
		OPERATOR::ToSource(dataMask, this->m_vdata, this->m_cdata, data);

		::Set(*this->PSourceMask(),rX, rY, rZ, dataMask);
		for (int n=0; n<this->m_cdata; n++){
			::Set(*this->PSourceCombine(n),rX, rY, rZ, this->m_vdata[n]);
		}
	}
};


//============================================================================
// OperatorWeightedAverageOf
//============================================================================
// the value in source1 is used to select the dimension in source2
template <class T1, class T2>
class OperatorWeightedAverageOf
{
};

template <>
class OperatorWeightedAverageOf<Point256DReal, Pixel8BitGrey>
{
public:
	typedef Pixel8BitGrey type_data;

	static void FromSource(
		Pixel8BitGrey& dataOut, 
		const Point256DReal& dataMask,  const Pixel8BitGrey* vdata, const int& cdata
	){
		PromotionOf<Pixel8BitGrey,Real>::type dataT(0.);
		Real rT=0.;

		for (int n=0; n<cdata; n++){
			const Real& rWeight=dataMask.Dim(vdata[n]);
			rT+=rWeight;
			dataT = dataT + vdata[n] * rWeight;
		}
		dataOut=dataT/rT;
	}

	static void ToSource(
		Point256DReal& dataMask,  Pixel8BitGrey* vdata, const int& cdata,
		const Pixel8BitGrey& dataOut
	){
		ERROR("not implemented");
	}

};


template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
class TypeSourceMaskCombineWeightedAverageOf
{
public: 
	typedef SourceMaskCombinePointwiseOperatorOf<
		typename OperatorWeightedAverageOf<TypeOfData(SOURCEMASK), TypeOfData(SOURCECOMBINE)>::type_data,
		DIMENSIONALITY,PRECISION,SOURCEMASK, SOURCECOMBINE
	> type;
};

#define SourceMaskCombineWeightedAverageOf(_data,_dims,_precision,_src1,_src2)			\
	typename TypeSourceMaskCombineWeightedAverageOf<_data,_dims,_precision,_src1,_src2>::type


//============================================================================
// WeightedAverage
//============================================================================
MAKE_MASKCOMBINE_ASSISTANT(
	WeightedAverage,
	SourceMaskCombinePointwiseOperatorOf,
	OperatorWeightedAverageOf<TypeOfData(SOURCEMASK) COMMA TypeOfData(SOURCECOMBINE)>,
	TypeOfDimensionality(SOURCEMASK),
	TypeOfPrecision(SOURCEMASK),
	{}
);


#endif






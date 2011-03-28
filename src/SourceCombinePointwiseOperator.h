// $Id$

#ifndef __SOURCECOMBINEPOINTWISEOPERATOR_H__
#define __SOURCECOMBINEPOINTWISEOPERATOR_H__

#include "SourceGenerics.h"
#include "SourceCombine.h"
#include "Promotion.h"
#include "libUtility.h"


//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceCombinePointwiseOperatorBaseOf 
	: public SourceCombineOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	typedef typename SOURCE::type_data DATA_SOURCE;
	
	mutable DATA_SOURCE* m_vdata;
	mutable int m_cdata;
public:
	SourceCombinePointwiseOperatorBaseOf()
		: SourceCombineOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>()
	{
		m_vdata=NULL;
		m_cdata=0;
	}

	virtual ~SourceCombinePointwiseOperatorBaseOf()
	{
		SafeDeleteArray(m_vdata);
	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceCombinePointwiseOperatorOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	virtual void PrepareForAccessAction() const
	{
 		SourceCombineOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>::PrepareForAccessAction();
		if (m_cdata!=this->CSources()){
			SafeDeleteArray(m_vdata);
			m_cdata=this->CSources();
			m_vdata=new DATA_SOURCE[m_cdata];
		}
	}

};

//============================================================================
//============================================================================
template<class OPERATOR, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceCombinePointwiseOperatorOf 
	: public SourceCombineOf<typename OPERATOR::type_data, DIMENSIONALITY, PRECISION, SOURCE>
{
};


//============================================================================
// 2D
//============================================================================
template<class OPERATOR, class PRECISION, class SOURCE>
class SourceCombinePointwiseOperatorOf<OPERATOR, 2, PRECISION, SOURCE>
	: public SourceCombinePointwiseOperatorBaseOf<typename OPERATOR::type_data, 2, PRECISION, SOURCE>
{
	typedef typename OPERATOR::type_data DATA;
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		for (int n=0; n<this->m_cdata; n++){
			::Get(*this->PSource(n), this->m_vdata[n], rX, rY);
		}

		OPERATOR::FromSource(dataOut, this->m_vdata, this->m_cdata);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		OPERATOR::ToSource(this->m_vdata, this->m_cdata, data);

		for (int n=0; n<this->m_cdata; n++){
			::Set(*this->PSource(n),rX, rY, this->m_vdata[n]);
		}
	}
};


//============================================================================
// 3d
//============================================================================
template<class OPERATOR, class PRECISION, class SOURCE>
class SourceCombinePointwiseOperatorOf<OPERATOR, 3, PRECISION, SOURCE>
	: public SourceCombinePointwiseOperatorBaseOf<typename OPERATOR::type_data, 3, PRECISION, SOURCE>
{
	typedef typename OPERATOR::type_data DATA;
public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		for (int n=0; n<this->m_cdata; n++){
			::Get(*this->PSource(n), this->m_vdata[n], rX, rY, rZ);
		}

		OPERATOR::FromSource(dataOut, this->m_vdata, this->m_cdata);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		OPERATOR::ToSource(this->m_vdata, this->m_cdata, data);

		for (int n=0; n<this->m_cdata; n++){
			::Set(*this->PSource(n),rX, rY, rZ, this->m_vdata[n]);
		}
	}

};





//============================================================================
// OperatorAverage
//============================================================================
template <class DATA_OUT, class DATA_SOURCE>
class OperatorAverageOf
{
public:
	typedef DATA_OUT type_data;

	static void FromSource(DATA_OUT& dataOut, const DATA_SOURCE* vdata, const int& cdata){
		RANGEPROMOTION(DATA_SOURCE) dataT(0);

		for (int n=0; n<cdata; n++){
			dataT = dataT + vdata[n];
		}
		dataOut=dataT/cdata;
	}

	static void ToSource(DATA_SOURCE* vdata, const int& cdata, const DATA_OUT& dataOut){
		for (int n=0; n<cdata; n++){
			vdata[n] = dataOut;
		}
	}
};


template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class TypeSourceCombineAverageOf
{
public: 
	typedef SourceCombinePointwiseOperatorOf<
		OperatorAverageOf<DATA, typename SOURCE::type_data>,
		DIMENSIONALITY,PRECISION,SOURCE
	> type;
};

#define SourceCombineAverageOf(_data,_dims,_precision,_src) \
	typename TypeSourceCombineAverageOf<_data,_dims,_precision,_src>::type
	


//============================================================================
// OperatorSum
//============================================================================
template <class DATA_OUT, class DATA_SOURCE>
class OperatorSumOf
{
public:
	typedef DATA_OUT type_data;

	static void FromSource(DATA_OUT& dataOut, const DATA_SOURCE* vdata, const int& cdata){
		dataOut=0;

		for (int n=0; n<cdata; n++){
			dataOut = dataOut + vdata[n];
		}
	}

	static void ToSource(DATA_SOURCE* vdata, const int& cdata, const DATA_OUT& dataOut){
		for (int n=0; n<cdata; n++){
			vdata[n] = dataOut/cdata;
		}
	}
};


template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class TypeSourceCombineSumOf
{
public: 
	typedef SourceCombinePointwiseOperatorOf<
		OperatorSumOf<DATA, typename SOURCE::type_data>,
		DIMENSIONALITY,PRECISION,SOURCE
	> type;
};

#define SourceCombineSumOf(_data,_dims,_precision,_src) \
	typename TypeSourceCombineSumOf<_data,_dims,_precision,_src>::type
	



//============================================================================
// OperatorDifference
//============================================================================
template <class DATA_OUT, class DATA_SOURCE>
class OperatorDifferenceOf
{
public:
	typedef DATA_OUT type_data;

	static void FromSource(DATA_OUT& dataOut, const DATA_SOURCE* vdata, const int& cdata){
		dataOut=vdata[0];

		for (int n=1; n<cdata; n++){
			dataOut = dataOut - vdata[n];
		}
		dataOut = dataOut + MidValue(DATA_SOURCE);
	}

	static void ToSource(DATA_SOURCE* vdata, const int& cdata, const DATA_OUT& dataOut){
		vdata[0]=dataOut;
		for (int n=1; n<cdata; n++){
			vdata[n] = MidValue(DATA_SOURCE)/cdata;
		}
	}
};


template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class TypeSourceCombineDifferenceOf
{
public: 
	typedef SourceCombinePointwiseOperatorOf<
		OperatorDifferenceOf<DATA, typename SOURCE::type_data>,
		DIMENSIONALITY,PRECISION,SOURCE
	> type;
};

#define SourceCombineDifferenceOf(_data,_dims,_precision,_src) \
	typename TypeSourceCombineDifferenceOf<_data,_dims,_precision,_src>::type
	


//============================================================================
// OperatorIndexOfMaximum
//============================================================================
template <class DATA_OUT, class DATA_SOURCE>
class OperatorIndexOfMaximumOf
{
public:
	typedef DATA_OUT type_data;

	static void FromSource(DATA_OUT& dataOut, const DATA_SOURCE* vdata, const int& cdata){
		DATA_SOURCE dataT=vdata[0];
		dataOut=0;

		for (int n=1; n<cdata; n++){
			if (dataT < vdata[n]){
				dataOut=n;
				dataT=vdata[n];
			}
		}
	}

	static void ToSource(DATA_SOURCE* vdata, const int& cdata, const DATA_OUT& dataOut){
		ERROR("not implemented");
	}
};


template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class TypeSourceCombineIndexOfMaximumOf
{
public: 
	typedef SourceCombinePointwiseOperatorOf<
		OperatorIndexOfMaximumOf<DATA, typename SOURCE::type_data>,
		DIMENSIONALITY,PRECISION,SOURCE
	> type;
};

#define SourceCombineIndexOfMaximumOf(_data,_dims,_precision,_src) \
	typename TypeSourceCombineIndexOfMaximumOf<_data,_dims,_precision,_src>::type
	



#define COMMA ,
//============================================================================
// Average
//============================================================================
MAKE_COMBINE_ASSISTANT(
	Average,
	SourceCombinePointwiseOperatorOf,
	OperatorAverageOf<TypeOfData(SOURCE) COMMA TypeOfData(SOURCE)>,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{}
);

//============================================================================
// Sum
//============================================================================
MAKE_COMBINE_ASSISTANT(
	Sum,
	SourceCombinePointwiseOperatorOf,
	OperatorSumOf<TypeOfData(SOURCE) COMMA TypeOfData(SOURCE)>,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{}
);


//============================================================================
// Difference
//============================================================================
MAKE_COMBINE_ASSISTANT(
	Difference,
	SourceCombinePointwiseOperatorOf,
	OperatorDifferenceOf<TypeOfData(SOURCE) COMMA TypeOfData(SOURCE)>,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{}
);






#endif //__SOURCECOMBINEPOINTWISEOPERATOR_H__





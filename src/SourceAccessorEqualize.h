// $Id$

#ifndef __SOURCEACCESSOREQUALIZE_H__
#define __SOURCEACCESSOREQUALIZE_H__

#include "SourceGenerics.h"

#include <math.h>

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorEqualizeBaseOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	mutable DATA m_vdataRemap[Range( TypeOfData(SOURCE) )];

public:
	typedef PointOf<DIMENSIONALITY,PRECISION> type_point;												
	typedef DATA type_data;																											
	typedef PRECISION type_precision;																						
	static const int type_dimensionality=DIMENSIONALITY;												

	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorEqualizeOf(" + DIMENSIONALITY + ")" + LINE_ + 
			this->DescribeCommon()
		);
	}

	void PrepareForAccessAction() const
	{
		this->m_ptSize=this->m_psource->Size();
		const int c=Range(TypeOfData(SOURCE));
		int vcdata[c];

		for (int n=0;n<c; n++){
			vcdata[n]=0;
		}
		
		forpoint(type_point,pt,0,this->Size()){
			TypeOfData(SOURCE) data;
			::GetPoint(*(this->m_psource),data,pt);
			vcdata[data]++;
		}

		// make CDF
		for (int n=1;n<c; n++){
			vcdata[n]=vcdata[n-1];
		}

		int cdataPerBin=this->CSize()/Range(DATA);

		for (int n=0;n<c; n++){
			m_vdataRemap[n]=(vcdata[n]+(n>0?vcdata[n-1]:0))/2 / cdataPerBin;
		}
	}
};

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorEqualizeOf 
	: public SourceAccessorEqualizeBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};

template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorEqualizeOf<DATA,2,PRECISION,SOURCE>
	: public SourceAccessorEqualizeBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		DATA dataT;
		::Get(*(this->m_psource),dataT,rX,rY);
		dataOut=this->m_vdataRemap[dataT];
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		ERROR("Not implemented");
	}
};



template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorEqualizeOf<DATA,3,PRECISION,SOURCE>
	: public SourceAccessorEqualizeBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		TypeOfData(SOURCE) dataT;
		::Get(*(this->m_psource),dataT,rX,rY,rZ);
		dataOut=this->m_vdataRemap[dataT];
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ,const DATA& data)
	{
		ERROR("Not implemented");
	}

};


//============================================================================
// assistant
//============================================================================
MAKE_ASSISTANT(
	Equalize,
	SourceAccessorEqualizeOf,
	{}
);

//============================================================================
// assistant
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	GreyPixelEqualize,
	SourceAccessorEqualizeOf,
	Pixel8BitGrey,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{}
);

#endif // __SOURCEACCESSOREQUALIZE_H__

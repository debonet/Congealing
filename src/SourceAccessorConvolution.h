// $Id$

#ifndef __SOURCEACCESSORCONVOLUTION_H__
#define __SOURCEACCESSORCONVOLUTION_H__

#include "SourceGenerics.h"

#include "Promotion.h"
#include <math.h>

//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorConvolutionBaseOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	SourceMemoryOf<Real,DIMENSIONALITY>* m_psrcFilter;
	mutable PointOf<DIMENSIONALITY,int> m_ptFilterOffset;

public:
	SourceAccessorConvolutionBaseOf()
		: SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>()
		, m_psrcFilter(NULL)
	{
	}

	virtual ~SourceAccessorConvolutionBaseOf() 
	{
		ReleasePointer(m_psrcFilter);
	}

	virtual String Describe() const
	{
		return (
			_LINE + SerializationId() + LINE_ + 
			(_INDENT + "with filter:" + LINE_ ) + 
			(_INDENT + (_INDENT + this->m_psrcFilter->Describe() + LINE_)) +
			this->DescribeCommon()
		);
	}

	void SetFilter(	SourceMemoryOf<Real,DIMENSIONALITY>* psrcFilter){
		ChangePointer(m_psrcFilter,psrcFilter);
	}

	void PrepareForAccessAction() const
	{
		this->m_ptSize=this->PSource()->Size();
		this->m_ptFilterOffset=-1*(this->m_psrcFilter->Size()/2);
	}

	static String SerializationId()
	{
		return String("SourceAccessorConvolutionOf(") + DIMENSIONALITY + "D)";
	}

	void SerializeSelf(Stream& st) const
	{ 
		::SerializePointer(st,m_psrcFilter);
		::Serialize(st,m_ptFilterOffset);
	}

	void DeserializeSelf(Stream& st)
	{
		::DeserializePointer(st,m_psrcFilter);
		::Deserialize(st,m_ptFilterOffset);
	}
};

//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorConvolutionOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};


//============================================================================
// 2D
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorConvolutionOf<DATA, 2, PRECISION, SOURCE>
	: public SourceAccessorConvolutionBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		dataOut=DATA(0.);
		typedef PointOf<2,int> POINT;
		
		forpoint(POINT,pt,0,this->m_psrcFilter->Size()){
			Real rFilterAtPoint;
			::GetPoint(*this->m_psrcFilter, rFilterAtPoint, pt);

			DATA dataAtPoint;

			::Get(
				*this->PSource()
				, dataAtPoint
				, pt.X()+this->m_ptFilterOffset.X() + rX
				, pt.Y()+this->m_ptFilterOffset.Y() + rY
			);

			dataOut = dataOut + (dataAtPoint * rFilterAtPoint);
		}
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		CONGEAL_ERROR("NOT IMPLEMENTED");
	}

};





//============================================================================
// 3d
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorConvolutionOf<DATA, 3, PRECISION, SOURCE>
	: public SourceAccessorConvolutionBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		dataOut=DATA(0);
		typedef PointOf<3,int> POINT;

		forpoint(POINT,pt,0,this->m_psrcFilter->Size()){
			Real rFilterAtPoint;
			::GetPoint(*this->m_psrcFilter, rFilterAtPoint, pt);

			DATA dataAtPoint;

			::Get(
				*this->PSource()
				, dataAtPoint
				, pt.X()+this->m_ptFilterOffset.X() + rX
				, pt.Y()+this->m_ptFilterOffset.Y() + rY
				, pt.Z()+this->m_ptFilterOffset.Z() + rZ
			);

			dataOut = dataOut + (dataAtPoint * rFilterAtPoint);
		}
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		CONGEAL_ERROR("NOT IMPLEMENTED");
	}

};




//============================================================================
// Convolution assistant
//============================================================================
MAKE_ASSISTANT(
	Convolution,
	SourceAccessorConvolutionOf,
	{ psrcOut->SetFilter(psrcFilter); },
	SourceMemoryOf<Real,TypeOfDimensionality(SOURCE)>* psrcFilter,
);




#endif // __SOURCEACCESSORCONVOLUTION_H__

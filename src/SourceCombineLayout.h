// $Id$

#ifndef __SOURCECOMBINELAYOUT_H__
#define __SOURCECOMBINELAYOUT_H__

#include "SourceGenerics.h"
#include "SourceCombine.h"
#include "Promotion.h"
#include "libUtility.h"
#include <math.h>


//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceCombineLayoutBaseOf 
	: public SourceCombineOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
	
	
protected:
	typedef PointOf<DIMENSIONALITY,PRECISION> type_point;
	VectorOf<type_point> m_vecpt;

public:
	void AddSourcePosition(type_point& pt)
	{
		m_vecpt.Append(pt);
	}

	virtual void SetSourcePosition(int n,type_point& pt)
	{
		m_vecpt[n]=pt;
	}

	// auto arranges in a grid
	virtual void AutoArrange()
	{
		int c=this->m_vecpsrc.C();

		type_point ptMax(0.);

		for (int n=0; n<c; n++){
			this->m_vecpsrc[n]->PrepareForAccess();
			ptMax=congeal_max(ptMax,this->m_vecpsrc[n]->Size());
		}
		
		typedef PointOf<DIMENSIONALITY,int> POINT;

		POINT ptEnd;
		if (ptMax.Dim(0) > ptMax.Dim(1) * 1.2){
			ptEnd=POINT(floor(pow(c,1./DIMENSIONALITY)));
			
			for (int n=DIMENSIONALITY-1 && c > ptEnd.CVolume(); n>=0; n--){
				ptEnd.Dim(n)++;
			}
		}
		else{
			ptEnd=POINT(ceil(pow(c,1./DIMENSIONALITY)));
		}

		m_vecpt.SetSize(c);

		int n=0;
		forpoint(POINT,npt,0,ptEnd){
			m_vecpt[n]=ptMax*npt;

			n++;
			if (n>=c){
				break;
			}
		}
	}

	virtual String Describe() const
	{
		String s;
		for (int n=0; n<m_vecpt.C(); n++){
			s += _LINE + "Offset[" + n + "]" + m_vecpt[n].Describe() + LINE_;
		}

		return (
			_LINE + "SourceCombineOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			+ (_INDENT + s) + 
			this->DescribeCommon()
		);
	}

	virtual void PrepareForAccessAction() const
	{
		this->m_ptSize=m_vecpt[0]+this->m_vecpsrc[0]->Size();

		for (int n=1; n<m_vecpt.C(); n++){
			this->m_ptSize=congeal_max(this->m_ptSize, m_vecpt[n]+this->m_vecpsrc[n]->Size());
		}
	}


};



//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceCombineLayoutOf 
	: public SourceCombineOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};


//============================================================================
// 2D
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceCombineLayoutOf<DATA, 2, PRECISION, SOURCE>
	: public SourceCombineLayoutBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		const int& c=this->m_vecpt.C();
		type_point pt=type_point(rX,rY);
		for (int n=c-1; n>=0; n--){
			type_point dpt=pt-this->m_vecpt[n];
			
			if  ( (!(dpt < 0)) &&  (!(dpt > this->PSource(n)->Size())) ){
				if  ( (dpt < 1) ||  (dpt > (this->PSource(n)->Size()-1)) ){
					dataOut=MaxValue(DATA);
				}
				else{
					::GetPoint(*this->PSource(n), dataOut, dpt);
				}
				return;
			}
		}
		dataOut=MidValue(DATA);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		const int& c=this->m_vecpt.C();
		type_point pt=type_point(rX,rY);
		for (int n=c-1; n>=0; n--){
			type_point dpt=pt-this->m_vecpt[n];
			
			if  ( (!(dpt < 0)) &&  (!(dpt > this->PSource(n)->Size())) ){
				::SetPoint(*this->PSource(n), dpt, data);
				return;
			}
		}
	}
};


//============================================================================
// 3d
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceCombineLayoutOf<DATA, 3, PRECISION, SOURCE>
	: public SourceCombineLayoutBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		const int& c=this->m_vecpt.C();
		type_point pt=type_point(rX,rY,rZ);
		for (int n=c-1; n>=0; n--){
			type_point dpt=pt-this->m_vecpt[n];
			
			if  ( (!(dpt < 0)) &&  (!(dpt > this->PSource(n)->Size())) ){
				::GetPoint(*this->PSource(n), dataOut, dpt);
				return;
			}
		}
		dataOut=DATA(0);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		const int& c=this->m_vecpt.C();
		type_point pt=type_point(rX,rY,rZ);
		for (int n=c-1; n>=0; n--){
			type_point dpt=pt-this->m_vecpt[n];
			
			if  ( (!(dpt < 0)) &&  (!(dpt > this->PSource(n)->Size())) ){
				::SetPoint(*this->PSource(n), dpt, data);
				return;
			}
		}
	}
};









template<class DATA, int DIMENSIONALITY, class PRECISION>
class TypeSourceCombineLayoutGenericOf
{
	public: 
	typedef SourceCombineLayoutOf<
	DATA, DIMENSIONALITY,PRECISION,SourceOf<DATA,DIMENSIONALITY,PRECISION>
	> type;
};

#define SourceCombineLayoutGeneric(_data,_dims,_precision)										\
	TypeSourceCombineLayoutGenericOf<_data,_dims,_precision>::type

typedef SourceCombineLayoutGeneric(Pixel8BitGrey,3,Real) GenericLayout_G3R;
typedef SourceCombineLayoutGeneric(Pixel8BitGrey,3,int) GenericLayout_G3I;
typedef SourceCombineLayoutGeneric(Pixel8BitGrey,2,Real) GenericLayout_G2R;
typedef SourceCombineLayoutGeneric(Pixel8BitGrey,2,int) GenericLayout_G2I;

#endif 
	

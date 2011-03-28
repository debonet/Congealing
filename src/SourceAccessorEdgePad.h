// $Id$

#ifndef __SOURCEACCESSOREDGEPAD_H__
#define __SOURCEACCESSOREDGEPAD_H__

#include "SourceGenerics.h"

#include "Point.h"
#include "Point2D.h"
#include <math.h>

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorEdgePadOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
public:
};


template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorEdgePadOf <DATA, 2, PRECISION, SOURCE>
	: public SourceAccessorOf<DATA, 2, PRECISION, SOURCE>
{
	DATA m_dataPad;

public:
	SourceAccessorEdgePadOf() 
		:  SourceAccessorOf<DATA, 2, PRECISION, SOURCE>()
		, m_dataPad(0)
	{
	}

	virtual void Describe() const
	{
		return (
			_LINE + "SourceAccessorEdgePadOf(2D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	void Get(DATA& dataOut, PRECISION rX, PRECISION rY) const
	{
		PRECISION cX=this->m_ptSize.X();
		PRECISION cY=this->m_ptSize.Y();

		if (rX<0 || rY<0 || rX>=cX || rY>=cY){
			dataOut=DATA(m_dataPad);
		}
		else{
			::Get(this->m_psource,dataOut,rX,rY);
		}
	}

	void Set(PRECISION rX, PRECISION rY, const DATA& data)
	{
		PRECISION cX=this->m_ptSize.X();
		PRECISION cY=this->m_ptSize.Y();

		if (rX<0 || rY<0 || rX>=cX || rY>=cY){
			return;
		}

		::Set(this->m_psource,rX,rY,data);
	}

	void SetPadding(const DATA& data)
	{
		m_dataPad=data;
	}

};

#endif // __SOURCEACCESSOREDGEPAD_H__

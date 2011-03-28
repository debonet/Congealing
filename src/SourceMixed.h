#ifndef __SOURCEMIXED_H__
#define __SOURCEMIXED_H__

// $Id$

#include "Source.h"
#include "Registry.h"
#include "libRefCount.h"

#undef DEBUG 
#define DEBUG 0
#include "libDebug.h"

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE1, class SOURCE2>
class SourceMixedOf 
	: public SourceOf<DATA, DIMENSIONALITY, PRECISION>
{
protected:
	SOURCE1* m_psource1;
	SOURCE2* m_psource2;
	mutable PointOf<DIMENSIONALITY, PRECISION> m_ptSize;

protected:
	SourceMixedOf()
		: SourceOf<DATA, DIMENSIONALITY, PRECISION>()
		,	m_psource1(NULL)
		,	m_psource2(NULL)

	{
	}

	SourceMixedOf(SourceMixedOf<DATA,DIMENSIONALITY,PRECISION,SOURCE1,SOURCE2>&src)
		: SourceOf<DATA,DIMENSIONALITY,PRECISION>(src)
	{
		m_psource1=src.m_psource1;
		ClaimPointer(m_psource1);
		m_psource2=src.m_psource2;
		ClaimPointer(m_psource2);
	}

	virtual ~SourceMixedOf()
	{
		ReleasePointer(m_psource1);
		ReleasePointer(m_psource2);
	}

public:

	SOURCE1* PSource1() 
	{
		return m_psource1;
	}

	const SOURCE1* PSource1() const
	{
		return m_psource1;
	}

	SOURCE2* PSource2() 
	{
		return m_psource2;
	}

	const SOURCE2* PSource2() const
	{
		return m_psource2;
	}

	void SetSources(SOURCE1* psource1,SOURCE2* psource2)
	{
		ChangePointer(m_psource1,psource1);
		ChangePointer(m_psource2,psource2);
	}

	void WipeSource()
	{
		m_psource1=NULL;
		m_psource2=NULL;
	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceMixedOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	virtual String DescribeCommon() const
	{
		return _INDENT + (
			_LINE + "size: " + this->Size().Describe() + LINE_ + 
			( _INDENT + "source 1:" + LINE_ + 
				this->m_psource1->Describe() + 
				this->m_psource2->Describe() 
			)
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg){}

	virtual void RegisterDataAsParameters(RegistryOfParameters& reg) {}

	const PointOf<DIMENSIONALITY,PRECISION>& Size() const
	{
		return m_ptSize;
	}

	virtual PointOf<DIMENSIONALITY,PRECISION> Bound(const PointOf<DIMENSIONALITY,PRECISION>& pt) const{
		return pt.Bound(
			PointOf<DIMENSIONALITY,PRECISION> (PRECISION(0)),
			this->Size()
		);
	}

	virtual void PrepareForAccessAction() const
	{
 		this->m_ptSize=this->PSource1()->Size();
	}

	virtual void PrepareForAccess() const
	{
		// tell sources to prepare first
		m_psource1->PrepareForAccess();
		m_psource2->PrepareForAccess();

		// the call the specific mixed's AccessPreparation function
		this->PrepareForAccessAction();
	}

};




template <class DATA, class PRECISION, class SOURCE1, class SOURCE2>
inline void Get(
	const SourceMixedOf<DATA,3,PRECISION,SOURCE1,SOURCE2>& src, 
	DATA& dataOut, const PRECISION& nX, const PRECISION& nY
)
{
	src.VirtualGet(dataOut,nX,nY);
}


template <class DATA, class PRECISION, class SOURCE1, class SOURCE2>
inline void Set(
	SourceMixedOf<DATA,3,PRECISION,SOURCE1,SOURCE2>& src,  
	const PRECISION& nX, const PRECISION& nY, const DATA& data
)
{
	src.VirtualSet(nX,nY,data);
}


template <class DATA, class PRECISION, class SOURCE1, class SOURCE2>
inline void Get(
	const SourceMixedOf<DATA,3,PRECISION,SOURCE1,SOURCE2>& src, DATA& dataOut,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ
)
{
	src.VirtualGet(dataOut,nX,nY,nZ);
}

template <class DATA, class PRECISION, class SOURCE1, class SOURCE2>
inline void Set(
	SourceMixedOf<DATA,3,PRECISION,SOURCE1,SOURCE2>& src,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ, const DATA& data
)
{
	src.VirtualSet(nX,nY,nZ,data);
}


#endif // __SOURCEMIXED_H__

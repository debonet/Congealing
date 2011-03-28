// $Id$

#ifndef __SOURCETRANSFORMREMAP_H__
#define __SOURCETRANSFORMREMAP_H__

#include "SourceGenerics.h"
#include <math.h>


//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformRemapOf 
	: public SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
	SourceTransformRemapOf()
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
	{}
	// TODO
};




//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformRemapOf<DATA,2,PRECISION,SOURCE>
	: public SourceTransformOf<DATA, 2, PRECISION, SOURCE>
{
	Real *m_vrRemap;

public:
	SOURCE_ACTUALS_2D;

	SourceTransformRemapOf()
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
	{	
		int cRange=Range(DATA);
		m_vrRemap=new Real[cRange];
		for (int n=0; n<cRange; n++){
			m_vrRemap[n]=Real(n)/cRange;
		}
	}

	virtual ~SourceTransformRemapOf(){
		if (Counted::AboutToDie()){
			delete [] m_vrRemap;
		}
	}

	virtual String Describe() const
	{
		String s;
		int cRange=Range(DATA);
		for (int n=0; n<cRange; n+=10){
			for (int d=0; d<10 && (n+d)<cRange; d++){
				s=s+ _LINE + (m_vrRemap[n+d]*cRange) + LINE_;
			}
		}

		return (
			_LINE + "SourceTransformRemapOf(2D)" + LINE_ + 
			(_INDENT + s + LINE_) + 
			this->DescribeCommon()
		);
	}


	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		int c=Range(DATA);
		for (int n=0; n<c; n++){
			reg.Register(&m_vrRemap[n]);
		}
	}

	void SetRemap(int n, Real rRemap)
	{
		m_vrRemap[n]=rRemap;
	}

	const Real& GetRemap(int n) const
	{
		return m_vrRemap[n];
	}

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		::Get(*(this->m_psource),	dataOut,rX,rY);
		dataOut=DATA(m_vrRemap[(int)(dataOut)]*Range(DATA));
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		ERROR("not implemeneted (but could be)");
	}

};






//============================================================================
// 3d
//============================================================================
//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformRemapOf<DATA,3,PRECISION,SOURCE>
	: public SourceTransformOf<DATA, 3, PRECISION, SOURCE>
{
	Real *m_vrRemap;

public:
	SOURCE_ACTUALS_3D;

	SourceTransformRemapOf()
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
	{	
		int cRange=Range(DATA);
		m_vrRemap=new Real[cRange];
		for (int n=0; n<cRange; n++){
			m_vrRemap[n]=Real(n)/cRange;
		}
	}

	virtual ~SourceTransformRemapOf(){
		if (Counted::AboutToDie()){
			delete [] m_vrRemap;
		}
	}

	virtual String Describe() const
	{
		String s;
		int cRange=Range(DATA);
		for (int n=0; n<cRange; n+=10){
			for (int d=0; d<10 && (n+d)<cRange; d++){
				s=s+ _LINE + (m_vrRemap[n+d]*cRange) + LINE_;
			}
		}

		return (
			_LINE + "SourceTransformRemapOf(3D)" + LINE_ + 
			(_INDENT + s + LINE_) + 
			this->DescribeCommon()
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		int c=Range(DATA);
		for (int n=0; n<c; n++){
			reg.Register(&m_vrRemap[n]);
		}
	}

	void SetRemap(int n, Real rRemap)
	{
		m_vrRemap[n]=rRemap;
	}

	const Real& GetRemap(int n) const
	{
		return m_vrRemap[n];
	}

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		::Get(*(this->m_psource),	dataOut,rX,rY,rZ);
		dataOut=DATA(m_vrRemap[(int)(dataOut)]*Range(DATA));
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		ERROR("not implemeneted (but could be)");
	}

};



#endif // __SOURCETRANSFORMREMAP_H__

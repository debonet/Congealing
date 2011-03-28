// $Id$

#ifndef __SOURCEMEMORY_H__
#define __SOURCEMEMORY_H__

#include <stdlib.h>
#include "Source.h"
#include "Pixels.h"

#undef DEBUG
#define DEBUG 0
#include "libDebug.h"

//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY>
class SourceMemoryBaseOf 
	: public SourceOf<DATA, DIMENSIONALITY, int>
{
	typedef int PRECISION;
	typedef PointOf<DIMENSIONALITY,int> POINT;

protected:
	typedef PointOf<DIMENSIONALITY,int> type_point;

	PointOf<DIMENSIONALITY,int> m_ptSize;
	DATA *m_vdata;

public:
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	SourceMemoryBaseOf() 
		: SourceOf<DATA,DIMENSIONALITY,int>()
		, m_ptSize(0)
		, m_vdata(NULL)
	{}


	virtual ~SourceMemoryBaseOf()
	{
		if (this->AboutToDie()){
			Free();
		}
	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceMemory(" + DIMENSIONALITY + "D)" + LINE_ +
			this->DescribeCommon()
		);
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	const PointOf<DIMENSIONALITY,int>& Size() const
	{
		return m_ptSize;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	PointOf<DIMENSIONALITY,int> Bound(const PointOf<DIMENSIONALITY,int>& pt) const
	{
		return pt.Bound(
			PointOf<DIMENSIONALITY,int>(0),
			m_ptSize-1
		);
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	const DATA *ReadData() const
	{
		return m_vdata;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	DATA *WriteData() 
	{
		return m_vdata;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	virtual void Allocate(const PointOf<DIMENSIONALITY,int>& ptSize)
	{
		ASSERTf(Counted::References()==1,"Shared object being reallocated?");
		if (m_ptSize.CVolume() == ptSize.CVolume()){
			m_ptSize=ptSize;
			D1("KEEPING MEMORY %ld", sizeof(DATA)*this->CSize());
		}
		else{
			Free();
			m_ptSize=ptSize;
			D1("ALLOCATING MEMORY %ld", sizeof(DATA)*this->CSize());
			m_vdata=new DATA[this->CSize()];
		}
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void Free()
	{
		if (m_vdata!=NULL){
			D1("DELETEING MEMORY %ld", sizeof(DATA)*this->CSize());
			delete [] m_vdata;
		}
	}


	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	virtual void Fill(const DATA& data){
		int c=this->CSize();
		for (int n=0; n<c; n++){
			m_vdata[n]=data;
		}
	}

/*
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	// each point dimensionality will probably want to override this,
	// but a default implementation is possible
	void GetPoint(DATA& dataOut, const PointOf<DIMENSIONALITY,int>& ptIn) const
	{
		WARNING("probably dont want to use this");
		PointOf<DIMENSIONALITY,int> pt=ptIn.Bound(
			PointOf<DIMENSIONALITY,int>(0),this->m_ptSize-1
		);

		int cDim=DIMENSIONALITY;

		int nLoc=0;
		for (int nDim=0; nDim<cDim; nDim++){
			nLoc=int(nLoc*this->CSize(nDim) + pt.Dim(nDim));
		}

		dataOut=this->m_vdata[nLoc];
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	// each point dimensionality will probably want to override this,
	// but a default implementation is possible
	void SetPoint(const PointOf<DIMENSIONALITY,int>& ptIn, const DATA& data)
	{
		WARNING("probably dont want to use this");
		PointOf<DIMENSIONALITY,int> pt=ptIn.Bound(
			PointOf<DIMENSIONALITY,int>(0),this->m_ptSize-1
		);

		int cDim=DIMENSIONALITY;

		int nLoc=0;
		for (int nDim=0; nDim<cDim; nDim++){
			nLoc=int(nLoc*this->CSize(nDim) + pt.Dim(nDim));
		}

		this->m_vdata[nLoc]=data;
	}
*/

	static String SerializationId()
	{
		return String("SourceMemory(") + DIMENSIONALITY +"D)";
	}

	void Serialize(Stream& st) const
	{ 
		::Serialize(st,m_ptSize);
		st.Append((byte*)m_vdata,this->CSize()*sizeof(DATA));
	}

	void Deserialize(Stream& st)
	{
		type_point pt;
		::Deserialize(st,pt);
		Allocate(pt);
		st.Read((byte*)m_vdata,this->CSize()*sizeof(DATA));
	}
};








//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY>
class SourceMemoryOf
	: public SourceMemoryBaseOf<DATA, DIMENSIONALITY>
{
	void RegisterDataAsParameters(RegistryOfParameters& reg){
		ERROR("NOT IMPLEMENTED");
	}
};







//============================================================================
//============================================================================
template<class DATA>
class SourceMemoryOf<DATA,2> : public SourceMemoryBaseOf<DATA, 2>
{
	typedef int PRECISION;
public:
	SOURCE_ACTUALS_2D;

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void Get(DATA& dataOut, const int& nX, const int& nY) const
	{
		const int& cX=this->m_ptSize.X();

		dataOut= this->m_vdata[nY*cX + nX];
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void Set(const int& nX, const int& nY, const DATA& data)
	{
		const int& cX=this->m_ptSize.X();

		this->m_vdata[nY*cX + nX]=data;
	}

	void RegisterDataAsParameters(RegistryOfParameters& reg);

};




//============================================================================
//============================================================================
template<class DATA>
class SourceMemoryOf<DATA,3> : public SourceMemoryBaseOf<DATA, 3>
{
	typedef int PRECISION;
public:
	SOURCE_ACTUALS_3D;

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void Get(DATA& dataOut, const int& nX, const int& nY, const int& nZ) const
	{
		const int& cX=this->m_ptSize.X();
		const int& cY=this->m_ptSize.Y();

		dataOut= this->m_vdata[(nZ*cY+nY)*cX + nX];
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void Set(const int& nX, const int& nY, const int& nZ, const DATA& data)
	{
		const int& cX=this->m_ptSize.X();
		const int& cY=this->m_ptSize.Y();

		this->m_vdata[(nZ*cY+nY)*cX + nX]=data;
	}

	void RegisterDataAsParameters(RegistryOfParameters& reg);
};


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
template <>
inline void SourceMemoryOf<Point2DReal,2>::RegisterDataAsParameters(RegistryOfParameters& reg) 
{
	int c=this->CSize();
	for (int n=0; n<c; n++){
		reg.Register(&(this->m_vdata[n].X()));
		reg.Register(&(this->m_vdata[n].Y()));
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
template <>
inline void SourceMemoryOf<Point3DReal,3>::RegisterDataAsParameters(RegistryOfParameters& reg) 
{
	int c=this->CSize();
	for (int n=0; n<c; n++){
		reg.Register(&(this->m_vdata[n].X()));
		reg.Register(&(this->m_vdata[n].Y()));
		reg.Register(&(this->m_vdata[n].Z()));
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
template <>
inline void SourceMemoryOf<Real,2>::RegisterDataAsParameters(RegistryOfParameters& reg) 
{
	int c=this->CSize();
	for (int n=0; n<c; n++){
		reg.Register(&(this->m_vdata[n]));
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
template <>
inline void SourceMemoryOf<Real,3>::RegisterDataAsParameters(RegistryOfParameters& reg) 
{
	int c=this->CSize();
	for (int n=0; n<c; n++){
		reg.Register(&(this->m_vdata[n]));
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
template <class DATA>
inline void SourceMemoryOf<DATA,2>::RegisterDataAsParameters(RegistryOfParameters& reg) 
{
	ERROR("NOT IMPLEMENTED");
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
template <class DATA>
inline void SourceMemoryOf<DATA,3>::RegisterDataAsParameters(RegistryOfParameters& reg) 
{
	ERROR("NOT IMPLEMENTED");
}



//==========================================================================
//==========================================================================
// HELPERS (ASSISTANTS)
//==========================================================================
//==========================================================================

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
template<class SOURCE, int DIMENSIONALITY>
SOURCE* Allocate(
	const PointOf<DIMENSIONALITY,int>& ptSize,
	SOURCE* psm
)
{
	psm->Allocate(ptSize);
	return psm;
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
template<class SOURCE, class DATA>
SOURCE* Fill(
	const DATA& data,
	SOURCE *psm
)
{
	psm->Fill(data);
	return psm;
}



#endif // __SOURCEMEMORY_H__

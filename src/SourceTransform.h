#ifndef __SOURCETRANSFORM_H__
#define __SOURCETRANSFORM_H__

// $Id$

#include "Source.h"
#include "Registry.h"
#include "libRefCount.h"

#undef DEBUG 
#define DEBUG 0
#include "libDebug.h"


typedef RegistryOf<Parameter> RegistryOfInitialSteps;

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformOf 
	: public SourceOf<DATA, DIMENSIONALITY, PRECISION>
{
protected:
	SOURCE* m_psource;
	mutable PointOf<DIMENSIONALITY, PRECISION> m_ptSize;

protected:
	SourceTransformOf()
		: SourceOf<DATA, DIMENSIONALITY, PRECISION>()
	{
		m_psource=NULL;
	}

	SourceTransformOf(const SourceTransformOf<DATA,DIMENSIONALITY,PRECISION,SOURCE>&src)
		: SourceOf<DATA,DIMENSIONALITY,PRECISION>(src)
	{
		m_psource=src.m_psource;
		ClaimPointer(m_psource);
	}

	virtual ~SourceTransformOf() __attribute__((always_inline))
	{
		ReleasePointer(m_psource);
	}

public:

	SOURCE* PSource() 
	{
		return m_psource;
	}

	const SOURCE* PSource() const
	{
		return m_psource;
	}

	void SetSource(SOURCE* psource)
	{
		ChangePointer(m_psource,psource);
	}

	void WipeSource()
	{
		m_psource=NULL;
	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceTransformOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	virtual String DescribeCommon() const
	{
		return _INDENT + (
			_LINE + "size: " + this->Size().Describe() + LINE_ + 
			this->m_psource->Describe() 
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg)=0;

	virtual void RegisterDataAsParameters(RegistryOfParameters& reg) 
	{
		m_psource->RegisterDataAsParameters(reg);
	}

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
 		this->m_ptSize=this->PSource()->Size();
	}

	virtual void PrepareForAccess() const
	{
		// tell source to prepare first
		m_psource->PrepareForAccess();

		// the call the specific transform's AccessPreparation function
		this->PrepareForAccessAction();
	}

	virtual void SerializeSelf(Stream& st) const=0; 

	virtual void DeserializeSelf(Stream& st)=0;

	void Serialize(Stream& st) const
	{ 
		SerializeSelf(st);
		::SerializePointer(st,this->m_psource);
	}

	void Deserialize(Stream &st)
	{
		DeserializeSelf(st);
		::DeserializePointer(st,this->m_psource);
	}

};





template <class DATA, class PRECISION, class SUBSOURCE>
inline void Get(
	const SourceTransformOf<DATA,3,PRECISION,SUBSOURCE>& src, 
	DATA& dataOut, const PRECISION& nX, const PRECISION& nY
)
{
	src.VirtualGet(dataOut,nX,nY);
}


template <class DATA, class PRECISION, class SUBSOURCE>
inline void Set(
	SourceTransformOf<DATA,3,PRECISION,SUBSOURCE>& src,  
	const PRECISION& nX, const PRECISION& nY, const DATA& data
)
{
	src.VirtualSet(nX,nY,data);
}


template <class DATA, class PRECISION, class SUBSOURCE>
inline void Get(
	const SourceTransformOf<DATA,3,PRECISION,SUBSOURCE>& src, DATA& dataOut,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ
)
{
	src.VirtualGet(dataOut,nX,nY,nZ);
}

template <class DATA, class PRECISION, class SUBSOURCE>
inline void Set(
	SourceTransformOf<DATA,3,PRECISION,SUBSOURCE>& src,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ, const DATA& data
)
{
	src.VirtualSet(nX,nY,nZ,data);
}



//============================================================================
// ASSISTANTS
//   NOTE: calls to these macros must end with a trailing comma!
//============================================================================
#define COMMA ,

#define MAKE_TYPECHANGE_ASSISTANT(																						\
	_name,_class,_data,_dims,_precision,_code,...																\
)																																							\
	template <class SOURCE>																											\
	inline _class<_data,_dims,_precision,SOURCE> *															\
	_name(__VA_ARGS__ SOURCE* psrcIn)																						\
	{																																						\
		typedef _class<_data,_dims,_precision,SOURCE> SRC_OUT;										\
																																							\
		SRC_OUT *psrcOut=new SRC_OUT;																							\
		psrcOut->SetSource(psrcIn);																								\
		_code;																																		\
	 	HandoffPointer(psrcOut);																									\
		return psrcOut;																														\
	}

#define MAKE_ASSISTANT(_name,_class,_code,...)																\
	MAKE_TYPECHANGE_ASSISTANT(																									\
		_name,																																		\
		_class,																																		\
		TypeOfData(SOURCE),																												\
		TypeOfDimensionality(SOURCE),																							\
		TypeOfPrecision(SOURCE),																									\
		_code,																																		\
		##__VA_ARGS__																															\
	);



#endif // __SOURCETRANSFORM_H__

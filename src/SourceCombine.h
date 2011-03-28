#ifndef __SOURCECOMBINE_H__
#define __SOURCECOMBINE_H__

// $Id$

#include "Source.h"
#include "Registry.h"
#include "libRefCount.h"
#include "Vector.h"
#include "String.h"

#undef DEBUG 
#define DEBUG 0
#include "libDebug.h"

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceCombineOf 
	: public SourceOf<DATA, DIMENSIONALITY, PRECISION>
{
	typedef VectorOf<SOURCE*> VECPSRC;
protected:
	VECPSRC m_vecpsrc;
	mutable PointOf<DIMENSIONALITY, PRECISION> m_ptSize;

protected:
	SourceCombineOf()
		: SourceOf<DATA, DIMENSIONALITY, PRECISION>()
		, m_vecpsrc()
	{
	}

	SourceCombineOf(SourceCombineOf<DATA,DIMENSIONALITY,PRECISION,SOURCE>&src)
		: SourceOf<DATA,DIMENSIONALITY,PRECISION>(src)
		, m_vecpsrc(src.m_vecpsrc)
	{
	}

	virtual ~SourceCombineOf()
	{
		if (m_vecpsrc.AboutToDie()){
			for (int n=0; n<m_vecpsrc.C(); n++){
				ReleasePointer(m_vecpsrc[n]);
			}
		}
	}

public:

	SOURCE* PSource(int n)
	{
		return m_vecpsrc[n];
	}

	const SOURCE* PSource(int n) const
	{
		return m_vecpsrc[n];
	}

	SOURCE** VSources()
	{
		return m_vecpsrc.V();
	}

	const SOURCE** VSources() const
	{
		return m_vecpsrc.V();
	}

	virtual void AddSource(SOURCE* psource)
	{
		ClaimPointer(psource);
		m_vecpsrc.Append(psource);
	}

	const int& CSources() const
	{
		return m_vecpsrc.C();
	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceCombineOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	virtual String DescribeCommon() const
	{
		String s;
		for (int n=0; n<m_vecpsrc.C(); n++){
			s = s + ( 
				_INDENT + (
					_LINE + "source: " + n + LINE_ +
					(_INDENT + this->m_vecpsrc[n]->Describe() + LINE_)
				)
			);
		}

		return _INDENT + (
			_LINE + "size: " + this->Size().Describe() + LINE_ + 
			s
		);
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
 		m_ptSize=PSource(0)->Size();
	}

	virtual void PrepareForAccess() const
	{
		// tell each source to prepare first
		for (int n=0; n<m_vecpsrc.C(); n++){
			m_vecpsrc[n]->PrepareForAccess();
		}

		// the call the specific Combine's AccessPreparation function
		this->PrepareForAccessAction();
	}

	virtual void RegisterDataAsParameters(RegistryOfParameters& reg) 
	{}

	virtual void RegisterParameters(RegistryOfParameters& reg)
	{}

	virtual void SerializeSelf(Stream& st) const 
	{}

	virtual void DeserializeSelf(Stream& st) 
	{}


	void Serialize(Stream &st) const
	{ 
		SerializeSelf(st);
		for (int n=0; n<m_vecpsrc.C(); n++){
			::SerializePointer(st,this->m_vecpsrc[n]);
		}
	}

	void Deserialize(Stream &st)
	{
		DeserializeSelf(st);
		for (int n=0; n<m_vecpsrc.C(); n++){
			::DeserializePointer(st,this->m_vecpsrc[n]);
		}
	}



};





template <class DATA, class PRECISION, class SUBSOURCE>
inline void Get(
	const SourceCombineOf<DATA,3,PRECISION,SUBSOURCE>& src, 
	DATA& dataOut, const PRECISION& nX, const PRECISION& nY
)
{
	src.VirtualGet(dataOut,nX,nY);
}


template <class DATA, class PRECISION, class SUBSOURCE>
inline void Set(
	SourceCombineOf<DATA,3,PRECISION,SUBSOURCE>& src,  
	const PRECISION& nX, const PRECISION& nY, const DATA& data
)
{
	src.VirtualSet(nX,nY,data);
}


template <class DATA, class PRECISION, class SUBSOURCE>
inline void Get(
	const SourceCombineOf<DATA,3,PRECISION,SUBSOURCE>& src, DATA& dataOut,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ
)
{
	src.VirtualGet(dataOut,nX,nY,nZ);
}

template <class DATA, class PRECISION, class SUBSOURCE>
inline void Set(
	SourceCombineOf<DATA,3,PRECISION,SUBSOURCE>& src,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ, const DATA& data
)
{
	src.VirtualSet(nX,nY,nZ,data);
}






#define MAKE_COMBINE_ASSISTANT(																								\
	_name,_class,_data,_dims,_precision,_code,...																\
)																																							\
	template <class SOURCE>																											\
	inline _class<_data,_dims,_precision,SOURCE> *															\
	_name(__VA_ARGS__ SOURCE* vsrcIn, int csrcIn)																\
	{																																						\
		typedef _class<_data,_dims,_precision,SOURCE> SRC_OUT;										\
																																							\
		SRC_OUT *psrcOut=new SRC_OUT;																							\
		for (int n=0; n<csrcIn; n++){																							\
			psrcOut->AddSource(&vsrcIn[n]);																					\
		}																																					\
		_code;																																		\
	 	HandoffPointer(psrcOut);																									\
		return psrcOut;																														\
	}																																						\
																																							\
	template <class SOURCE>																											\
	inline _class<_data,_dims,_precision,SOURCE> *															\
	_name(__VA_ARGS__ SOURCE** vpsrcIn, int csrcIn)															\
	{																																						\
		typedef _class<_data,_dims,_precision,SOURCE> SRC_OUT;										\
																																							\
		SRC_OUT *psrcOut=new SRC_OUT;																							\
		for (int n=0; n<csrcIn; n++){																							\
			psrcOut->AddSource(vpsrcIn[n]);																					\
		}																																					\
		_code;																																		\
	 	HandoffPointer(psrcOut);																									\
		return psrcOut;																														\
	}																																						\
																																							\
	template <class SOURCE, class SOURCE_SUB>																		\
	inline _class<_data,_dims,_precision,SOURCE> *															\
	_name(__VA_ARGS__ SOURCE_SUB* vsrcIn, int csrcIn, SOURCE*(*fxn)(SOURCE_SUB*) ) \
	{																																						\
		typedef _class<_data,_dims,_precision,SOURCE> SRC_OUT;										\
																																							\
		SRC_OUT *psrcOut=new SRC_OUT;																							\
		for (int n=0; n<csrcIn; n++){																							\
			psrcOut->AddSource(fxn(&vsrcIn[n]));																		\
		}																																					\
		_code;																																		\
	 	HandoffPointer(psrcOut);																									\
		return psrcOut;																														\
	}																																						\
																																							\



#endif // __SOURCECOMBINE_H__

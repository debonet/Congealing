#ifndef __SOURCEMASKCOMBINE_H__
#define __SOURCEMASKCOMBINE_H__

// $Id$

#include "Source.h"
#include "Registry.h"
#include "libRefCount.h"

#undef DEBUG 
#define DEBUG 0
#include "libDebug.h"

template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
class SourceMaskCombineOf 
	: public SourceOf<DATA, DIMENSIONALITY, PRECISION>
{
	typedef VectorOf<SOURCECOMBINE*> VECPSRC;
protected:
	SOURCEMASK* m_psourceMask;
	VECPSRC m_vecpsrc;
	mutable PointOf<DIMENSIONALITY, PRECISION> m_ptSize;

protected:
	SourceMaskCombineOf()
		: SourceOf<DATA, DIMENSIONALITY, PRECISION>()
		,	m_psourceMask(NULL)
		,	m_vecpsrc()

	{
	}

	SourceMaskCombineOf(SourceMaskCombineOf<DATA,DIMENSIONALITY,PRECISION,SOURCEMASK,SOURCECOMBINE>&src)
		: SourceOf<DATA,DIMENSIONALITY,PRECISION>(src)
		, m_vecpsrc(src.m_vecpsrc)
	{
		m_psourceMask=src.m_psourceMask;
		ClaimPointer(m_psourceMask);
	}

	virtual ~SourceMaskCombineOf()
	{
		ReleasePointer(m_psourceMask);
		if (m_vecpsrc.AboutToDie()){
			for (int n=0; n<m_vecpsrc.C(); n++){
				ReleasePointer(m_vecpsrc[n]);
			}
		}
	}

public:

	SOURCEMASK* PSourceMask() 
	{
		return m_psourceMask;
	}

	const SOURCEMASK* PSourceMask() const
	{
		return m_psourceMask;
	}

	SOURCECOMBINE* PSourceCombine(int n)
	{
		return m_vecpsrc[n];
	}

	const SOURCECOMBINE* PSourceCombine(int n) const
	{
		return m_vecpsrc[n];
	}


	void SetSourceMask(SOURCEMASK* psourceMask)
	{
		ChangePointer(m_psourceMask,psourceMask);
	}

	void AddSourceCombine(SOURCECOMBINE* psource)
	{
		ClaimPointer(psource);
		m_vecpsrc.Append(psource);
	}

	const int& CSourceCombine() const
	{
		return m_vecpsrc.C();
	}

	void WipeSource()
	{
		m_psourceMask=NULL;
	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceMaskCombineOf(" + DIMENSIONALITY + "D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

	virtual String DescribeCommon() const
	{
		String s;
		for (int n=0; n<m_vecpsrc.C(); n++){
			s = s + ( 
				_INDENT + (
					_LINE + "sourcecombine: " + n + LINE_ +
					(_INDENT + this->m_vecpsrc[n]->Describe() + LINE_)
				)
			);
		}

		return _INDENT + (
			_LINE + "size: " + this->Size().Describe() + LINE_ + 
			( _INDENT + "source mask:" + LINE_ + 
				this->m_psourceMask->Describe() 
			) + 
			s
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg){};

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
 		this->m_ptSize=this->PSourceMask()->Size();
	}

	virtual void PrepareForAccess() const
	{
		// tell sources to prepare first
		m_psourceMask->PrepareForAccess();

		// tell each sourcecombine to prepare first
		for (int n=0; n<m_vecpsrc.C(); n++){
			PSourceCombine(n)->PrepareForAccess();
		}

		// the call the specific MaskCombine's AccessPreparation function
		this->PrepareForAccessAction();
	}
};



template <class DATA, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
inline void Get(
	const SourceMaskCombineOf<DATA,3,PRECISION,SOURCEMASK,SOURCECOMBINE>& src, 
	DATA& dataOut, const PRECISION& nX, const PRECISION& nY
)
{
	src.VirtualGet(dataOut,nX,nY);
}


template <class DATA, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
inline void Set(
	SourceMaskCombineOf<DATA,3,PRECISION,SOURCEMASK,SOURCECOMBINE>& src,  
	const PRECISION& nX, const PRECISION& nY, const DATA& data
)
{
	src.VirtualSet(nX,nY,data);
}


template <class DATA, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
inline void Get(
	const SourceMaskCombineOf<DATA,3,PRECISION,SOURCEMASK,SOURCECOMBINE>& src, DATA& dataOut,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ
)
{
	src.VirtualGet(dataOut,nX,nY,nZ);
}

template <class DATA, class PRECISION, class SOURCEMASK, class SOURCECOMBINE>
inline void Set(
	SourceMaskCombineOf<DATA,3,PRECISION,SOURCEMASK,SOURCECOMBINE>& src,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ, const DATA& data
)
{
	src.VirtualSet(nX,nY,nZ,data);
}



#define MAKE_MASKCOMBINE_ASSISTANT(																						\
	_name,_class,_data,_dims,_precision,_code,...																\
)																																							\
	template <class SOURCEMASK,class SOURCECOMBINE>															\
	inline _class<_data,_dims,_precision,SOURCEMASK,SOURCECOMBINE> *						\
	_name(__VA_ARGS__ SOURCEMASK* psrcMask, SOURCECOMBINE* vsrcIn, int csrcIn)	\
	{																																						\
		typedef _class<_data,_dims,_precision,SOURCEMASK,SOURCECOMBINE> SRC_OUT;	\
																																							\
		SRC_OUT *psrcOut=new SRC_OUT;																							\
		psrcOut->SetSourceMask(psrcMask);																					\
		for (int n=0; n<csrcIn; n++){																							\
			psrcOut->AddSourceCombine(&vsrcIn[n]);																	\
		}																																					\
		_code;																																		\
	 	HandoffPointer(psrcOut);																									\
		return psrcOut;																														\
	}																																						\
																																							\
	template <class SOURCEMASK,class SOURCECOMBINE>															\
	inline _class<_data,_dims,_precision,SOURCEMASK,SOURCECOMBINE> *						\
	_name(__VA_ARGS__ SOURCEMASK* psrcMask, SOURCECOMBINE** vpsrcIn, int csrcIn)	\
	{																																						\
		typedef _class<_data,_dims,_precision,SOURCEMASK,SOURCECOMBINE> SRC_OUT;	\
																																							\
		SRC_OUT *psrcOut=new SRC_OUT;																							\
		psrcOut->SetSourceMask(psrcMask);																					\
		for (int n=0; n<csrcIn; n++){																							\
			psrcOut->AddSourceCombine(vpsrcIn[n]);																	\
		}																																					\
		_code;																																		\
	 	HandoffPointer(psrcOut);																									\
		return psrcOut;																														\
	}																																						\
																																							\
	template <class SOURCEMASK, class SOURCECOMBINE, class SOURCE_SUB>					\
	inline _class<_data,_dims,_precision,SOURCEMASK,SOURCECOMBINE> *						\
	_name(																																			\
		__VA_ARGS__																																\
		SOURCEMASK* psrcMask,																											\
		SOURCE_SUB* vsrcIn, int csrcIn, SOURCECOMBINE*(*fxn)(SOURCE_SUB*) )				\
	{																																						\
		typedef _class<_data,_dims,_precision,SOURCEMASK,SOURCECOMBINE> SRC_OUT;	\
																																							\
		SRC_OUT *psrcOut=new SRC_OUT;																							\
		psrcOut->SetSourceMask(psrcMask);																					\
		for (int n=0; n<csrcIn; n++){																							\
			psrcOut->AddSourceCombine(fxn(&vsrcIn[n]));															\
		}																																					\
		_code;																																		\
	 	HandoffPointer(psrcOut);																									\
		return psrcOut;																														\
	}




#endif // __SOURCEMASKCOMBINE_H__

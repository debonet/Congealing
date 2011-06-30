// $Id$

#ifndef __SOURCE_H__
#define __SOURCE_H__

#include "Point.h"
#include "Point2D.h"
#include "Point3D.h"
#include "libBaseTypes.h"
#include "Registry.h"
#include "libRefCount.h"

#undef DEBUG 
#define DEBUG 0
#include "libDebug.h"


#ifdef _WIN32
#  define ALWAYS_INLINE
#else 
#define ALWAYS_INLINE __attribute__((always_inline))
#endif


typedef RegistryOf<Parameter*> RegistryOfParameters;

inline void RegisterPointAsParameter(RegistryOfParameters &reg, PointOf<2,Real>& pt){
	reg.Register(&pt.X());
	reg.Register(&pt.Y());
}

inline void RegisterPointAsParameter(RegistryOfParameters &reg, PointOf<3,Real>& pt){
	reg.Register(&pt.X());
	reg.Register(&pt.Y());
	reg.Register(&pt.Z());
}



// Untemplated core of all sources
class BasicSource : public Counted
{
public:
	BasicSource(const BasicSource& bs) 
		: Counted(bs)
	{}

	BasicSource()
		: Counted()
	{}

	virtual ~BasicSource() ALWAYS_INLINE
	{}

	virtual String Describe() const =0;

	virtual void Serialize(Stream& st) const=0;
	virtual void Deserialize(Stream &st)=0;
};


#include "libSerialize.h"

template<class DATA, int DIMENSIONALITY, class PRECISION>
class SourceBaseOf
	: public BasicSource
{
	typedef PointOf<DIMENSIONALITY,PRECISION> POINT;

public:
	typedef PRECISION type_precision;
	typedef DATA type_data;
	static const int type_dimensionality = DIMENSIONALITY;
	typedef PointOf<DIMENSIONALITY,PRECISION> type_point;

	template <class T>
	static PRECISION cast_precision(const T&t)  {
		return PRECISION(t);
	}

	virtual ~SourceBaseOf() ALWAYS_INLINE
	{}

	virtual String DescribeCommon() const
	{
		return _INDENT + (
			_LINE + "size: " + this->Size().Describe() + LINE_ 
		);
	}

	virtual const POINT& Size() const=0;

	virtual void VirtualGetPoint(DATA& dataOut, const POINT& pt) const =0;
	virtual void VirtualSetPoint(const POINT& pt, const DATA& data)=0;

	virtual POINT Bound(const POINT& pt) const=0;

	virtual void RegisterDataAsParameters(RegistryOfParameters& reg) =0;

	virtual void PrepareForAccess() const
	{}

	PRECISION CSize() const
	{
		return Size().CVolume();
	}

	PRECISION CSize(int n) const
	{
		return Size().Dim(n);
	}
};


template<class DATA, int DIMENSIONALITY, class PRECISION>
class SourceOf
	: public SourceBaseOf<DATA,DIMENSIONALITY,PRECISION>
{
};


template<class DATA, class PRECISION>
class SourceOf<DATA, 2, PRECISION>
	: public SourceBaseOf<DATA,2,PRECISION>
{
public:
	static const int DIMENSIONALITY=2;
	typedef PointOf<DIMENSIONALITY,PRECISION> POINT;
public:
	virtual ~SourceOf() ALWAYS_INLINE
	{}

	virtual void VirtualGet(DATA& dataOut, const PRECISION& nX, const PRECISION& nY) const =0;
	virtual void VirtualSet(const PRECISION& nX, const PRECISION& nY, const DATA& data)=0;
};


template<class DATA, class PRECISION>
class SourceOf<DATA, 3, PRECISION>
	: public SourceBaseOf<DATA,3,PRECISION>
{
public:
	static const int DIMENSIONALITY=3;
	typedef PointOf<DIMENSIONALITY,PRECISION> POINT;
public:

	virtual ~SourceOf() ALWAYS_INLINE
	{}

	virtual void VirtualGet(DATA& dataOut, const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ) const =0;
	virtual void VirtualSet(const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ, const DATA& data)=0;
 
};




template <class SOURCE, class DATA, int DIMENSIONALITY, class PRECISION>
inline void GetPoint(
	const SOURCE& src, 
	DATA& dataOut, const PointOf<DIMENSIONALITY,PRECISION>& pt
)
{
	src.GetPoint(dataOut,pt);
}

template <class DATA, int DIMENSIONALITY, class PRECISION, class PRECISION2>
inline void GetPoint(
	const SourceOf<DATA,DIMENSIONALITY,PRECISION>& src, 
	DATA& dataOut, const PointOf<DIMENSIONALITY,PRECISION2>& pt
)
{
	src.VirtualGetPoint(dataOut,pt);
}

template <class SOURCE, class DATA, int DIMENSIONALITY, class PRECISION>
inline void SetPoint(
	SOURCE& src, 
	const PointOf<DIMENSIONALITY,PRECISION>& pt, const DATA& data
)
{
	src.SetPoint(pt,data);
}

template <class DATA, int DIMENSIONALITY, class PRECISION, class PRECISION2>
inline void SetPoint(
	SourceOf<DATA,DIMENSIONALITY,PRECISION>& src, 
	const PointOf<DIMENSIONALITY,PRECISION2>& pt, const DATA& data
)
{
	src.VirtualSetPoint(pt,data);
}

#define TESTERROR(x) 

#define SOURCE_ACTUALS_2D																											\
	static const int DIMENSIONALITY=2;																					\
	typedef PointOf<DIMENSIONALITY,PRECISION> type_point;												\
	typedef DATA type_data;																											\
	typedef PRECISION type_precision;																						\
	static const int type_dimensionality=DIMENSIONALITY;												\
																																							\
	inline void GetPoint(DATA& dataOut, const PointOf<2,PRECISION>& pt) const					\
	{																																						\
		this->Get(dataOut,pt.X(),pt.Y());																					\
	}																																						\
																																							\
	inline void VirtualGetPoint(DATA& dataOut, const PointOf<2,PRECISION>& pt) const		\
	{																																						\
		TESTERROR("called VirtualGetPoint");																		\
		this->Get(dataOut,pt.X(),pt.Y());																					\
	}																																						\
																																							\
	inline void SetPoint(const PointOf<2,PRECISION>& pt, const DATA& data)							\
	{																																						\
		this->Set(pt.X(),pt.Y(),data);																						\
	}																																						\
																																							\
	inline void VirtualSetPoint(const PointOf<2,PRECISION>& pt, const DATA& data)			\
	{																																						\
		TESTERROR("called VirtualSetPoint");																				\
		this->Set(pt.X(),pt.Y(),data);																						\
	}																																						\
																																							\
	inline void VirtualGet(DATA& dataOut, const PRECISION& rX, const PRECISION& rY) const \
	{																																						\
		TESTERROR("called VirtualGet");																						\
		Get(dataOut,rX,rY);																												\
		this->Get(dataOut,rX,rY);																									\
	}																																						\
																																							\
	inline void VirtualSet(const PRECISION &rX, const PRECISION &rY, const DATA& data)	\
	{																																						\
		TESTERROR("called VirtualSet");																					\
		this->Set(rX,rY,data);																										\
	}																																						




template <class SOURCE, class DATA, class PRECISION>
inline void Get(
	const SOURCE& src, 
	DATA& dataOut, const PRECISION& nX, const PRECISION& nY
)
{
	src.Get(dataOut,nX,nY);
}
template <class DATA, class PRECISION, class PRECISION2>
inline void Get(
	const SourceOf<DATA,2,PRECISION>& src, 
	DATA& dataOut, const PRECISION2& nX, const PRECISION2& nY
)
{
	src.VirtualGet(dataOut,src.cast_precision(nX),src.cast_precision(nY));
}

template <class SOURCE, class DATA, class PRECISION>
inline void Set(
	SOURCE& src, 
	const PRECISION& nX, const PRECISION& nY, const DATA& data
)
{
	src.Set(nX,nY,data);
}

template <class DATA, class PRECISION, class PRECISION2>
inline void Set(
	SourceOf<DATA,2,PRECISION>& src,  
	const PRECISION2& nX, const PRECISION2& nY, const DATA& data
)
{
	src.VirtualSet(src.cast_precision(nX),src.cast_precision(nY),data);
}



#define SOURCE_ACTUALS_3D																											\
	static const int DIMENSIONALITY=3;																					\
	typedef PointOf<DIMENSIONALITY,PRECISION> type_point;												\
	typedef DATA type_data;																											\
	typedef PRECISION type_precision;																						\
	static const int type_dimensionality=DIMENSIONALITY;												\
																																							\
	inline void GetPoint(DATA& dataOut, const PointOf<3,PRECISION>& pt) const					\
	{																																						\
		this->Get(dataOut,pt.X(),pt.Y(),pt.Z());																	\
	}																																						\
																																							\
	inline void VirtualGetPoint(DATA& dataOut, const PointOf<3,PRECISION>& pt) const		\
	{																																						\
		TESTERROR("called VirtualGetPoint");																					\
		this->Get(dataOut,pt.X(),pt.Y(),pt.Z());																				\
	}																																						\
																																							\
	inline void SetPoint(const PointOf<3,PRECISION>& pt, const DATA& data)							\
	{																																						\
		this->Set(pt.X(),pt.Y(),pt.Z(),data);																			\
	}																																						\
																																							\
	inline void VirtualSetPoint(const PointOf<3,PRECISION>& pt, const DATA& data)			\
	{																																						\
		TESTERROR("called VirtualSetPoint");																					\
		this->Set(pt.X(),pt.Y(),pt.Z(),data);																			\
	}																																						\
																																							\
	inline void VirtualGet(DATA& dataOut, const PRECISION& rX, const PRECISION& rY, const PRECISION& rZ) const \
	{																																						\
		TESTERROR("called VirtualGet");																							\
		this->Get(dataOut,rX,rY,rZ);																							\
	}																																						\
																																							\
	inline void VirtualSet(const PRECISION &rX, const PRECISION &rY, const PRECISION& rZ, const DATA& data) \
	{																																						\
		TESTERROR("called VirtualSet");																							\
		this->Set(rX,rY,rZ,data);																									\
	}


template <class SOURCE, class DATA, class PRECISION>
inline void Get(
	const SOURCE& src, DATA& dataOut,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ
)
{
	src.Get(dataOut,nX,nY,nZ);
}

template <class DATA, class PRECISION,class PRECISION2>
inline void Get(
	const SourceOf<DATA,3,PRECISION>& src, DATA& dataOut,
	const PRECISION2& nX, const PRECISION2& nY, const PRECISION2& nZ
)
{
	src.VirtualGet(dataOut,src.cast_precision(nX),src.cast_precision(nY),src.cast_precision(nZ));
}

template <class SOURCE, class DATA, class PRECISION>
inline void Set(
	SOURCE& src,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ, const DATA& data
)
{
	src.Set(nX,nY,nZ,data);
}

template <class DATA, class PRECISION, class PRECISION2>
inline void Set(
	SourceOf<DATA,3,PRECISION>& src,
	const PRECISION2& nX, const PRECISION2& nY, const PRECISION2& nZ, const DATA& data
)
{
	src.VirtualSet(src.cast_precision(nX),src.cast_precision(nY),src.cast_precision(nZ),data);
}




#define TypeOfSource(x) typename x::type_source
#define TypeOfPrecision(x) typename x::type_precision
#define TypeOfData(x) typename x::type_data
#define TypeOfDimensionality(x) x::type_dimensionality
#define TypeOfPoint(x) typename x::type_point





#endif // __SOURCE_H__

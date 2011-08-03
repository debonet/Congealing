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


/// a registry of pointers to parameters which can 
/// be passed around so that parameters can be modified in place
/// (without having to copy prior to manipulation, and after manipulation)
typedef RegistryOf<Parameter*> RegistryOfParameters;


/// puts both points in Point2DReal onto a registry 
/// \param reg registry to which parameter addresses are added
/// \param pt point whos components are to be added
inline void RegisterPointAsParameter(RegistryOfParameters &reg, PointOf<2,Real>& pt){
	reg.Register(&pt.X());
	reg.Register(&pt.Y());
}

/// puts all points in PointeDReal onto a registry 
/// \param reg registry to which parameter addresses are added
/// \param pt point whos components are to be added
inline void RegisterPointAsParameter(RegistryOfParameters &reg, PointOf<3,Real>& pt){
	reg.Register(&pt.X());
	reg.Register(&pt.Y());
	reg.Register(&pt.Z());
}



/// Untemplated core of all sources. 
/// Defines the dimensionless and typeless parts of the Source interface
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

	/// describes the source as a human readable string
	virtual String Describe() const =0;

	/// serializes the source into a stream 
	virtual void Serialize(Stream& st) const=0;

	/// deserializes the source from a stream 
	virtual void Deserialize(Stream &st)=0;
};


#include "libSerialize.h"


/// Core of all sources
/// Defines the dimension, type and precision dependent parts of 
/// the Source interface
/// \tparam DATA the storage type of the source
///	\tparam DIMENSIONALITY the number of dimensions of the source
/// \tparam PRECISION the class used to index each dimension
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

	/// a human readable description of the core aspects of a source
	virtual String DescribeCommon() const
	{
		return _INDENT + (
			_LINE + "size: " + this->Size().Describe() + LINE_ 
		);
	}

	/// the size of the source data
	/// \returns a point of DIMENSIONALITY,PRECISION
	virtual const POINT& Size() const=0;

	/// a virtual getter for a datapoint
	/// \param dataOut where the data is stored
	/// \param pt the point in the source from which the data is retrived
	virtual void VirtualGetPoint(DATA& dataOut, const POINT& pt) const =0;

	/// a virtual setter for a datapoint
	/// \param pt the point in the source into which the data is stored
	/// \param data the data to be stored
	virtual void VirtualSetPoint(const POINT& pt, const DATA& data)=0;

	/// Bound a point to the size of the source
	virtual POINT Bound(const POINT& pt) const=0;

	/// if the source has any paramters, add them to the registery
	/// \param reg the registry into which paramter potiners are added
	virtual void RegisterDataAsParameters(RegistryOfParameters& reg) =0;

	/// allow the source to perform any initial work required to 
	/// access its data accurately
	virtual void PrepareForAccess() const
	{}

	/// the volume of the source (product of the dimensions)
	PRECISION CSize() const
	{
		return Size().CVolume();
	}

	/// the size of a particular dimension
	/// \param n dimension whos size is requested
	PRECISION CSize(int n) const
	{
		return Size().Dim(n);
	}
};


/// A generic Source.
/// base class for all sources
/// \tparam DATA the storage type of the source
///	\tparam DIMENSIONALITY the number of dimensions of the source
/// \tparam PRECISION the class used to index each dimension
template<class DATA, int DIMENSIONALITY, class PRECISION>
class SourceOf
	: public SourceBaseOf<DATA,DIMENSIONALITY,PRECISION>
{
};


/// A generic 2D Source.
/// \tparam DATA the storage type of the source
/// \tparam PRECISION the class used to index each dimension
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

	/// a virtual getter using explicitly listed coordinates instead of a point
	/// \param dataOut where the data is stored
	/// \param nX coordinate of 0th dimension
	/// \param nY coordinate of 1st dimension
	virtual void VirtualGet(DATA& dataOut, const PRECISION& nX, const PRECISION& nY) const =0;

	/// a virtual setter using explicitly listed coordinates instead of a point
	/// \param nX coordinate of 0th dimension
	/// \param nY coordinate of 1st dimension
	/// \param data data to be stored
	virtual void VirtualSet(const PRECISION& nX, const PRECISION& nY, const DATA& data)=0;
};


/// A generic 3D Source.
/// \tparam DATA the storage type of the source
/// \tparam PRECISION the class used to index each dimension
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

	/// a virtual getter using explicitly listed coordinates instead of a point
	/// \param dataOut where the data is stored
	/// \param nX coordinate of 0th dimension
	/// \param nY coordinate of 1st dimension
	/// \param nZ coordinate of 2nd dimension
	virtual void VirtualGet(DATA& dataOut, const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ) const =0;

	/// a virtual setter using explicitly listed coordinates instead of a point
	/// \param nX coordinate of 0th dimension
	/// \param nY coordinate of 1st dimension
	/// \param nZ coordinate of 2nd dimension
	/// \param data data to be stored
	virtual void VirtualSet(const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ, const DATA& data)=0;
 
};




/// data accessor for a source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
template <class SOURCE, class DATA, int DIMENSIONALITY, class PRECISION>
inline void GetPoint(
	const SOURCE& src, 
	DATA& dataOut, const PointOf<DIMENSIONALITY,PRECISION>& pt
)
{
	src.GetPoint(dataOut,pt);
}

/// data accessor for a source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
template <class DATA, int DIMENSIONALITY, class PRECISION, class PRECISION2>
inline void GetPoint(
	const SourceOf<DATA,DIMENSIONALITY,PRECISION>& src, 
	DATA& dataOut, const PointOf<DIMENSIONALITY,PRECISION2>& pt
)
{
	src.VirtualGetPoint(dataOut,pt);
}

/// data accessor for a source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
template <class SOURCE, class DATA, int DIMENSIONALITY, class PRECISION>
inline void SetPoint(
	SOURCE& src, 
	const PointOf<DIMENSIONALITY,PRECISION>& pt, const DATA& data
)
{
	src.SetPoint(pt,data);
}

/// data accessor for a source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
template <class DATA, int DIMENSIONALITY, class PRECISION, class PRECISION2>
inline void SetPoint(
	SourceOf<DATA,DIMENSIONALITY,PRECISION>& src, 
	const PointOf<DIMENSIONALITY,PRECISION2>& pt, const DATA& data
)
{
	src.VirtualSetPoint(pt,data);
}

#define TESTERROR(x) 

/// Macro to implement inlinable and virtual accessors for 2D sources
/// this is needed because each derived source needs to have its own
/// methods to ensure inlinability
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




/// data accessor for a 2D source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
/// \param src source into which data is to be stored
/// \param dataOut where the data is stored
/// \param nX coordinate of 0th dimension
/// \param nY coordinate of 1st dimension
template <class SOURCE, class DATA, class PRECISION>
inline void Get(
	const SOURCE& src, 
	DATA& dataOut, const PRECISION& nX, const PRECISION& nY
)
{
	src.Get(dataOut,nX,nY);
}

/// data accessor for a 2D source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
/// \param src source into which data is to be stored
/// \param dataOut where the data is stored
/// \param nX coordinate of 0th dimension
/// \param nY coordinate of 1st dimension
template <class DATA, class PRECISION, class PRECISION2>
inline void Get(
	const SourceOf<DATA,2,PRECISION>& src, 
	DATA& dataOut, const PRECISION2& nX, const PRECISION2& nY
)
{
	src.VirtualGet(dataOut,src.cast_precision(nX),src.cast_precision(nY));
}

/// data accessor for a 2D source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
/// \param src source into which data is to be stored
/// \param data data to be stored
/// \param nX coordinate of 0th dimension
/// \param nY coordinate of 1st dimension
template <class SOURCE, class DATA, class PRECISION>
inline void Set(
	SOURCE& src, 
	const PRECISION& nX, const PRECISION& nY, const DATA& data
)
{
	src.Set(nX,nY,data);
}

/// data accessor for a 2D source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
/// \param src source into which data is to be stored
/// \param data data to be stored
/// \param nX coordinate of 0th dimension
/// \param nY coordinate of 1st dimension
template <class DATA, class PRECISION, class PRECISION2>
inline void Set(
	SourceOf<DATA,2,PRECISION>& src,  
	const PRECISION2& nX, const PRECISION2& nY, const DATA& data
)
{
	src.VirtualSet(src.cast_precision(nX),src.cast_precision(nY),data);
}


/// Macro to implement inlinable and virtual accessors for 3D sources
/// this is needed because each derived source needs to have its own
/// methods to ensure inlinability
#define SOURCE_ACTUALS_3D																								\
	static const int DIMENSIONALITY=3;																		\
	typedef PointOf<DIMENSIONALITY,PRECISION> type_point;									\
	typedef DATA type_data;																								\
	typedef PRECISION type_precision;																			\
	static const int type_dimensionality=DIMENSIONALITY;									\
																																				\
	inline void GetPoint(DATA& dataOut, const PointOf<3,PRECISION>& pt) const	\
	{																																						\
		this->Get(dataOut,pt.X(),pt.Y(),pt.Z());																	\
	}																																						\
																																							\
	inline void VirtualGetPoint(DATA& dataOut, const PointOf<3,PRECISION>& pt) const \
	{																																			\
		TESTERROR("called VirtualGetPoint");																\
		this->Get(dataOut,pt.X(),pt.Y(),pt.Z());														\
	}																																			\
																																							\
	inline void SetPoint(const PointOf<3,PRECISION>& pt, const DATA& data) \
	{																																			\
		this->Set(pt.X(),pt.Y(),pt.Z(),data);																\
	}																																			\
																																				\
	inline void VirtualSetPoint(const PointOf<3,PRECISION>& pt, const DATA& data)	\
	{																																			\
		TESTERROR("called VirtualSetPoint");																\
		this->Set(pt.X(),pt.Y(),pt.Z(),data);																\
	}																																			\
																																				\
	inline void VirtualGet(DATA& dataOut, const PRECISION& rX, const PRECISION& rY, const PRECISION& rZ) const \
	{																																			\
		TESTERROR("called VirtualGet");																			\
		this->Get(dataOut,rX,rY,rZ);																				\
	}																																			\
																																				\
	inline void VirtualSet(const PRECISION &rX, const PRECISION &rY, const PRECISION& rZ, const DATA& data) \
	{																																			\
		TESTERROR("called VirtualSet");																			\
		this->Set(rX,rY,rZ,data);																						\
	}


/// data accessor for a 3D source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
/// \param src source into which data is to be stored
/// \param dataOut where the data is stored
/// \param nX coordinate of 0th dimension
/// \param nY coordinate of 1st dimension
/// \param nZ coordinate of 2nd dimension
template <class SOURCE, class DATA, class PRECISION>
inline void Get(
	const SOURCE& src, DATA& dataOut,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ
)
{
	src.Get(dataOut,nX,nY,nZ);
}

/// data accessor for a 3D source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
/// \param src source into which data is to be stored
/// \param dataOut where the data is stored
/// \param nX coordinate of 0th dimension
/// \param nY coordinate of 1st dimension
/// \param nZ coordinate of 2nd dimension
template <class DATA, class PRECISION,class PRECISION2>
inline void Get(
	const SourceOf<DATA,3,PRECISION>& src, DATA& dataOut,
	const PRECISION2& nX, const PRECISION2& nY, const PRECISION2& nZ
)
{
	src.VirtualGet(dataOut,src.cast_precision(nX),src.cast_precision(nY),src.cast_precision(nZ));
}

/// data accessor for a 3D source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
/// \param src source into which data is to be stored
/// \param data data to be stored
/// \param nX coordinate of 0th dimension
/// \param nY coordinate of 1st dimension
/// \param nZ coordinate of 2nd dimension
template <class SOURCE, class DATA, class PRECISION>
inline void Set(
	SOURCE& src,
	const PRECISION& nX, const PRECISION& nY, const PRECISION& nZ, const DATA& data
)
{
	src.Set(nX,nY,nZ,data);
}

/// data accessor for a 3D source as a function instead of method
/// used to facilitate inlining, instead of forcing virtual function
/// \param src source into which data is to be stored
/// \param data data to be stored
/// \param nX coordinate of 0th dimension
/// \param nY coordinate of 1st dimension
/// \param nZ coordinate of 2nd dimension
template <class DATA, class PRECISION, class PRECISION2>
inline void Set(
	SourceOf<DATA,3,PRECISION>& src,
	const PRECISION2& nX, const PRECISION2& nY, const PRECISION2& nZ, const DATA& data
)
{
	src.VirtualSet(src.cast_precision(nX),src.cast_precision(nY),src.cast_precision(nZ),data);
}



/// macro to encapsulate ackward "typename class::element" syntax
#define TypeOfSource(x) typename x::type_source

/// macro to encapsulate ackward "typename class::element" syntax
#define TypeOfPrecision(x) typename x::type_precision

/// macro to encapsulate ackward "typename class::element" syntax
#define TypeOfData(x) typename x::type_data

/// macro to encapsulate ackward "typename class::element" syntax
#define TypeOfDimensionality(x) x::type_dimensionality

/// macro to encapsulate ackward "typename class::element" syntax
#define TypeOfPoint(x) typename x::type_point





#endif // __SOURCE_H__

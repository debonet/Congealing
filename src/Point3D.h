// $Id$

#ifndef __POINT3D_H__
#define __POINT3D_H__

#include "Point.h"
#include "Promotion.h"
#include "libSerialize.h"
template <class PRECISION>
class PointOf<3,PRECISION>
{
public:
	PRECISION m_xX;
	PRECISION m_xY;
	PRECISION m_xZ;

	typedef PRECISION type_precision;
	static const int DIMENSIONALITY = 3;
	static const int type_dimensionality = 3;

public:
	PointOf(PRECISION* vx){
		m_xX=vx[0];
		m_xY=vx[1];
		m_xZ=vx[2];
	}

	template <class PRECISION2>
	PointOf(const PRECISION2& x){
		m_xX=x;
		m_xY=x;
		m_xZ=x;
	}

	PointOf()
	{
	}

	String Describe() const;

	template <class OTHERPRECISION>
	PointOf(const PointOf<3,OTHERPRECISION> &pt){
		m_xX=pt.m_xX;
		m_xY=pt.m_xY;
		m_xZ=pt.m_xZ;
	}

	const PRECISION& Dim(int n) const{
		return (n==0)?m_xX:((n==1)?m_xY:m_xZ);
	}
	PRECISION& Dim(int n) {
		return (n==0)?m_xX:((n==1)?m_xY:m_xZ);
	}

	PRECISION CVolume() const{
		return m_xX*m_xY*m_xZ;
	}

	PointOf<3,PRECISION>& AdvanceLoop(
		const PointOf<3,PRECISION>& ptBy = 1
	){
		m_xX+=ptBy.m_xX;
		return *this;
	}

	bool StillLooping(
		const PointOf<3,PRECISION>& ptTo,
		const PointOf<3,PRECISION>& ptFrom=0,
		const PointOf<3,PRECISION>& ptBy=1
	){
		if (m_xX<ptTo.X()){
			return true;
		}

		if (m_xY+ptBy.m_xY<ptTo.Y()){
			m_xX=ptFrom.m_xX;
			m_xY+=ptBy.m_xY;
			return true;
		}

		if (m_xZ+ptBy.m_xZ<ptTo.Z()){
			m_xX=ptFrom.m_xX;
			m_xY=ptFrom.m_xY;
			m_xZ+=ptBy.m_xZ;
			return true;
		}

		return false;
	}


	PointOf(PRECISION xX, PRECISION xY, PRECISION xZ){
		m_xX=xX;
		m_xY=xY;
		m_xZ=xZ;
	}

	const PRECISION& X() const {return m_xX;}
	const PRECISION& Y() const {return m_xY;}
	const PRECISION& Z() const {return m_xZ;}

	PRECISION& X() {return m_xX;}
	PRECISION& Y() {return m_xY;}
	PRECISION& Z() {return m_xZ;}

	PointOf<3,PRECISION> Bound(
		const PointOf<3,PRECISION> &ptMin,
		const PointOf<3,PRECISION> &ptMax
	) const {
		return PointOf<3,PRECISION>(
			::Bound(X(),ptMin.X(),ptMax.X()),
			::Bound(Y(),ptMin.Y(),ptMax.Y()),
			::Bound(Z(),ptMin.Z(),ptMax.Z())
		);
	}

	operator PointOf<3,Real>() const;

	static String SerializationId()
	{
		return "Point3D";
	}

	void Serialize(Stream &st) const
	{ 
		::Serialize(st,m_xX);
		::Serialize(st,m_xY);
		::Serialize(st,m_xZ);
	}

	void Deserialize(Stream& st)
	{
		::Deserialize(st,m_xX);
		::Deserialize(st,m_xY);
		::Deserialize(st,m_xZ);
	}
};


//============================================================================
// quick names
//============================================================================
typedef PointOf<3,int> Point3D;
typedef PointOf<3,Real> Point3DReal;


//============================================================================
// casting operators
//============================================================================
template<>
inline Point3DReal::operator Point3DReal() const
{
	return *this;
}


template<>
inline Point3D::operator Point3DReal() const
{
	return Point3DReal(Real(m_xX), Real(m_xY),Real(m_xZ));
}



//============================================================================
// Describe() implementations
//============================================================================
template<class PRECISION>
inline String PointOf<3,PRECISION>::Describe() const
{
	return String("(") +  m_xX + ", " +  m_xY + ", " + m_xZ + ")" ;
}


//============================================================================
//============================================================================
template<class PRECISION>
inline PointOf<3,PRECISION> NewRandomPoint(
	const PointOf<3,PRECISION> &pt
)
{
	return PointOf<3,PRECISION> (
		Random(PRECISION(0),pt.X()),
		Random(PRECISION(0),pt.Y()),
		Random(PRECISION(0),pt.Z())
	);
}



//============================================================================
// create (point x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT3D_OPERATOR(_op)																						\
	template <class T1, class T2>																					\
	inline typename PromotionOf<PointOf<3,T1>,PointOf<3,T2> >::type operator _op ( \
		const PointOf<3,T1>& pt1, const PointOf<3,T2>& pt2									\
	)																																			\
	{																																			\
		return typename PromotionOf<PointOf<3,T1>,PointOf<3,T2> >::type(			\
			pt1.X() _op pt2.X(),pt1.Y() _op pt2.Y(), pt1.Z() _op pt2.Z()			\
		);																																	\
	}																																			\

POINT3D_OPERATOR(+);
POINT3D_OPERATOR(-);
POINT3D_OPERATOR(*);
POINT3D_OPERATOR(/);

//============================================================================
// create (point x point) operators
// where x is {+=,-=,*=,/=}
//============================================================================
#define POINT3D_SELFOPERATOR(_op)																				\
	template <class T1, class T2>																					\
	inline PointOf<3,T1> operator _op (																		\
		PointOf<3,T1>& pt1, const PointOf<3,T2>& pt2												\
	)																																			\
	{																																			\
		pt1.X() _op pt2.X();																								\
		pt1.Y() _op pt2.Y();																								\
		pt1.Z() _op pt2.Z();																								\
		return pt1;																													\
	}																																			

POINT3D_SELFOPERATOR(+=);
POINT3D_SELFOPERATOR(*=);
POINT3D_SELFOPERATOR(-=);
POINT3D_SELFOPERATOR(/=);

//============================================================================
// create (point x scalar) operators
// where x is {+=,-=,*=,/=}
//============================================================================
#define POINT3D_SELFSCALAROPERATOR(_op)																	\
	template <class T1, class T2>																					\
	inline PointOf<3,T1> operator _op (																		\
		PointOf<3,T1>& pt1, const T2& t2																		\
	)																																			\
	{																																			\
		pt1.X() _op t2;																											\
		pt1.Y() _op t2;																											\
		pt1.Z() _op t2;																											\
		return pt1;																													\
	}																																			

POINT3D_SELFSCALAROPERATOR(+=);
POINT3D_SELFSCALAROPERATOR(-=);
POINT3D_SELFSCALAROPERATOR(*=);
POINT3D_SELFSCALAROPERATOR(/=);


//============================================================================
// create (point x scalar) and (scalar x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT3D_SCALAROPERATOR(_op)																	\
	template <class T1, class T2>																			\
	inline typename PromotionOf<PointOf<3,T1>,T2 >::type operator _op (	\
		const PointOf<3,T1>& pt1, const T2& t2													\
	)																																	\
	{																																	\
		return typename PromotionOf<PointOf<3,T1>,T2 >::type(							\
			pt1.X() _op t2,pt1.Y() _op t2, pt1.Z() _op t2									\
		);																															\
	}																																	\
	template <class T1, class T2>																			\
	inline typename PromotionOf<T1,PointOf<3,T2> >::type operator _op (	\
		const T1& t1, const PointOf<3,T2>& pt2													\
	)																																	\
	{																																	\
		return typename PromotionOf<T1,PointOf<3,T2> >::type(							\
			t1 _op pt2.X(),t1 _op pt2.Y(), t1 _op pt2.Z()									\
		);																															\
	}

POINT3D_SCALAROPERATOR(+);
POINT3D_SCALAROPERATOR(-);
POINT3D_SCALAROPERATOR(*);
POINT3D_SCALAROPERATOR(/);


//============================================================================
// create (point x point) operators
// where x is {<,<=,>,>=,==,!=}
//============================================================================
#define POINT3D_COMPARATOR(_op,_opCombine)							\
	template <class T1, class T2>													\
	inline bool operator _op (														\
		const PointOf<3,T1>& pt1, const PointOf<3,T2>& pt2	\
	)																											\
	{																											\
		return (																						\
			pt1.X() _op pt2.X()																\
			_opCombine pt1.Y() _op pt2.Y()										\
			_opCombine pt1.Z() _op pt2.Z()										\
		);																									\
	}

POINT3D_COMPARATOR(<,||);
POINT3D_COMPARATOR(<=,||);
POINT3D_COMPARATOR(>,||);
POINT3D_COMPARATOR(>=,||);
POINT3D_COMPARATOR(!=,||);
POINT3D_COMPARATOR(==,&&);

//============================================================================
// create (point x point) operators
// where x is {<,<=,>,>=,==,!=}
//============================================================================
#define POINT3D_SCALARCOMPARATOR(_op,_opCombine)	\
	template <class T1, class T2>										\
	inline bool operator _op (											\
		const PointOf<3,T1>& pt1,const T2& t2					\
	)																								\
	{																								\
		return (																			\
			pt1.X() _op t2															\
			_opCombine pt1.Y() _op t2										\
			_opCombine pt1.Z() _op t2										\
		);																						\
	}																								\
	template <class T1, class T2>										\
	inline bool operator _op (											\
		const T1& t1, const PointOf<3,T2>& pt2				\
	)																								\
	{																								\
		return (																			\
			t1 _op pt2.X()															\
			_opCombine t1 _op pt2.Y()										\
			_opCombine t1 _op pt2.Z()										\
		);																						\
	}

POINT3D_SCALARCOMPARATOR(<,||);
POINT3D_SCALARCOMPARATOR(<=,||);
POINT3D_SCALARCOMPARATOR(>,||);
POINT3D_SCALARCOMPARATOR(>=,||);
POINT3D_SCALARCOMPARATOR(!=,||);
POINT3D_SCALARCOMPARATOR(==,&&);






//============================================================================
// create (point x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT3D_FUNCTION(_fxn)																								\
	template <class T1, class T2>																								\
	inline typename PromotionOf<PointOf<3,T1>,PointOf<3,T2> >::type								\
	_fxn (																																			\
		const PointOf<3,T1>& pt1, const PointOf<3,T2>& pt2												\
	)																																						\
	{																																						\
		return typename PromotionOf<PointOf<3,T1>,PointOf<3,T2> >::type(						\
			_fxn(pt1.X(), pt2.X())																									\
			, _fxn(pt1.Y(), pt2.Y())																								\
			, _fxn(pt1.Z(), pt2.Z())																								\
		);																																				\
	}

POINT3D_FUNCTION(min);
POINT3D_FUNCTION(max);


//============================================================================
// create (point x scalar) and (scalar x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT3D_SCALARFUNCTION(_fxn)																\
	template <class T1, class T2>																			\
	inline typename PromotionOf<PointOf<3,T1>,T2 >::type								\
	_fxn (																														\
		const PointOf<3,T1>& pt1, const T2& t2													\
	)																																	\
	{																																	\
		return typename PromotionOf<PointOf<3,T1>,T2 >::type(							\
			_fxn(pt1.X(), t2)																							\
			, _fxn(pt1.Y(), t2)																						\
			, _fxn(pt1.Z(), t2)																						\
		);																															\
	}																																	\
	template <class T1, class T2>																			\
	inline typename PromotionOf<T1,PointOf<3,T2> >::type								\
	_fxn (																														\
		const T1& t1, const PointOf<3,T2>& pt2													\
	)																																	\
	{																																	\
		return typename PromotionOf<T1,PointOf<3,T2> >::type(							\
			_fxn(t1, pt2.X())																							\
			, _fxn(t1, pt2.Y())																						\
			, _fxn(t1, pt2.Z())																						\
		);																															\
	}

POINT3D_SCALARFUNCTION(min);
POINT3D_SCALARFUNCTION(max);

//============================================================================
// create (point x scalar) and (scalar x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT3D_SOLOFUNCTION(_fxn)																	\
	template <class T1>																								\
	inline PointOf<3,T1>																							\
	_fxn (																														\
		const PointOf<3,T1>& pt1																				\
	)																																	\
	{																																	\
		return PointOf<3,T1>(																						\
			_fxn(pt1.X())																									\
			, _fxn(pt1.Y())																								\
			, _fxn(pt1.Z())																								\
		);																															\
	}																																	\


POINT3D_SOLOFUNCTION(exp);
POINT3D_SOLOFUNCTION(sqrt);
POINT3D_SOLOFUNCTION(cos);
POINT3D_SOLOFUNCTION(sin);
POINT3D_SOLOFUNCTION(tan);
POINT3D_SOLOFUNCTION(atan);



#endif //#ifndef __POINT3D_H__

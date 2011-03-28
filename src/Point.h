// $Id$

#ifndef __POINT_H__
#define __POINT_H__

#include "String.h"
#include "Promotion.h"
#include <stdarg.h>
#include "libSerialize.h"

#undef DEBUG
#define DEBUG 0
#include "libDebug.h"

// TODO: consider adding fixed-vector functionality? i.e. dot and cross product

template <int DIMENSIONALITY, class PRECISION>
class PointOf
{
public:
	PRECISION m_vx[DIMENSIONALITY];

	typedef PRECISION type_precision;
	static const int type_dimensionality = DIMENSIONALITY;

public:
	PointOf(PRECISION* vx)
	{
		memcpy(m_vx,vx,DIMENSIONALITY*sizeof(PRECISION));
	}

	template <class PRECISION2>
	PointOf(const PRECISION2& x)
	{
		for (int n=0; n<DIMENSIONALITY; n++){
			m_vx[n]=x;
		}
	}

	PointOf(const PointOf<DIMENSIONALITY,PRECISION>& pt)
	{
		memcpy(m_vx,pt.m_vx,DIMENSIONALITY*sizeof(PRECISION));
	}
	
	template<int DIMENSIONALITY2,class PRECISION2>
	PointOf(const PointOf<DIMENSIONALITY2,PRECISION2>& pt)
	{
		int c=min(DIMENSIONALITY,DIMENSIONALITY2);
		for (int n=0; n<c; n++){
			m_vx[n]=pt.m_vx[n];
		}
	}

	template<int DIMENSIONALITY2,class PRECISION2>
	PointOf<DIMENSIONALITY,PRECISION>& operator=(
		const PointOf<DIMENSIONALITY2,PRECISION2> &pt
	)
	{
		int c=min(DIMENSIONALITY,DIMENSIONALITY2);
		for (int n=0; n<c; n++){
			m_vx[n]=pt.m_vx[n];
		}
		return *this;
	}


	PointOf()
	{
	}

	// use two given args here to disambiguate from PointOf(scalar)
	PointOf(const PRECISION& x0,const PRECISION& x1, ...){
		m_vx[0]=x0;
		m_vx[1]=x1;

		va_list listArgs;
		va_start(listArgs, x1);
		for (int n=2; n<DIMENSIONALITY; n++){
			m_vx[n]=va_arg(listArgs,PRECISION);
		}
		va_end(listArgs);
	}


	String Describe() const
	{
		String s="(";
		for (int n=0; n<DIMENSIONALITY-1; n++){
			s=s+m_vx[n] + ", ";
		}
		s=s+m_vx[DIMENSIONALITY-1] + ")";
		return s;
	}

	const PRECISION& Dim(int n) const{
		return m_vx[n];
	}

	PRECISION& Dim(int n) {
		return m_vx[n];
	}

	RANGEPROMOTION(PRECISION) CVolume() const{
		RANGEPROMOTION(PRECISION) z=1;
		for (int n=0; n<DIMENSIONALITY; n++){
			z*=m_vx[n] ;
		}
		return z;
	}

	RANGEPROMOTION(PRECISION) Length() const{
		RANGEPROMOTION(PRECISION) z=0;
		for (int n=0; n<DIMENSIONALITY; n++){
			z+=(m_vx[n]*m_vx[n]) ;
		}
		return sqrt(z);
	}

	PointOf<DIMENSIONALITY,PRECISION>& AdvanceLoop(
		const PointOf<DIMENSIONALITY,PRECISION>& ptBy=1
	){
		m_vx[0]+=ptBy.m_vx[0];
		return *this;
	}

	bool StillLooping(
		const PointOf<DIMENSIONALITY,PRECISION>& ptTo=0,
		const PointOf<DIMENSIONALITY,PRECISION>& ptFrom=0,
		const PointOf<DIMENSIONALITY,PRECISION>& ptBy=1
	)
	{
		if (m_vx[0]<ptTo.m_vx[0]){
			return true;
		}

		for (int n=1; n<DIMENSIONALITY; n++){
			if ((m_vx[n]+ptBy.m_vx[n]) < ptTo.m_vx[n]){
				for (int n2=0; n2<n; n2++){
					m_vx[n2]=ptFrom.m_vx[0];
				}
				m_vx[n]+=ptBy.m_vx[n];
				return true;
			}
		}

		return false;
	}

	PointOf<DIMENSIONALITY,PRECISION> Bound(
		const PointOf<2,PRECISION> &ptMin,
		const PointOf<2,PRECISION> &ptMax
	) const {

		PointOf<DIMENSIONALITY,PRECISION> ptOut;

		for (int n=0; n<DIMENSIONALITY; n++){
			ptOut.m_vx[n]=::Bound(m_vx[n],ptMin.m_vx[n],ptMax.m_vx[n]);
		}
		return ptOut;
	}

	int MaxDimension() const
	{
		PRECISION rMax=m_vx[0];
		int nMax=0;
		for (int n=1; n<DIMENSIONALITY; n++){
			if (rMax<m_vx[n]){
				rMax=m_vx[n];
				nMax=n;
			}
		}
		return nMax;
	}

	int MinDimension() const
	{
		PRECISION rMin=m_vx[0];
		int nMin=0;
		for (int n=1; n<DIMENSIONALITY; n++){
			if (rMin>m_vx[n]){
				rMin=m_vx[n];
				nMin=n;
			}
		}
		return nMin;
	}

	static String SerializationId()
	{
		return String("Point(") + DIMENSIONALITY + "D)";
	}

	// points will have trailing commas in their serialization
	void Serialize(Stream& st) const
	{ 
		for (int n=0; n<DIMENSIONALITY; n++){
			::Serialize(st,m_vx[n]);
		}
	}

	void Deserialize(Stream& st)
	{
		for (int n=0; n<DIMENSIONALITY; n++){
			::Deserialize(st,m_vx[n]);
		}
	}
};



//============================================================================
//============================================================================
#define forpoint(_type,_name,_from,_to,...)																\
	for (																																		\
		_type _name=_type(_from);																							\
		_name.StillLooping(_type(_to), _type(_from), ##__VA_ARGS__);					\
		_name.AdvanceLoop(__VA_ARGS__)																				\
	)



//============================================================================
//============================================================================
template<class PRECISION, int DIMENSIONALITY>
inline PointOf<DIMENSIONALITY,PRECISION> NewRandomPoint(
	const PointOf<DIMENSIONALITY,PRECISION> &pt
)
{
	PointOf<DIMENSIONALITY,PRECISION> ptOut;

	for (int n=0; n<DIMENSIONALITY; n++){
		ptOut.m_vx[n]=Random(PRECISION(0),pt.m_vx[n]);
	}
	return ptOut;
}



//============================================================================
//============================================================================
// define point promotions
#define NEW_POINT_TYPE(_dims)																			\
	typedef PointOf< _dims , int> Point##_dims##D;															\
	typedef PointOf< _dims , Real> Point##_dims##DReal;													\
	DEFINE_SYMMETRICPROMOTION(Real,Point##_dims##D,Point##_dims##DReal);				\
	DEFINE_SYMMETRICPROMOTION(int,Point##_dims##D,Point##_dims##D);							\
	DEFINE_SYMMETRICPROMOTION(Point##_dims##DReal,Point##_dims##D,Point##_dims##DReal); \
	DEFINE_SYMMETRICPROMOTION(Real,Point##_dims##DReal,Point##_dims##DReal);		\
	DEFINE_SYMMETRICPROMOTION(int,Point##_dims##DReal,Point##_dims##DReal);


NEW_POINT_TYPE(1);
NEW_POINT_TYPE(2);
NEW_POINT_TYPE(3);
NEW_POINT_TYPE(4);
NEW_POINT_TYPE(5);
NEW_POINT_TYPE(6);
NEW_POINT_TYPE(7);
NEW_POINT_TYPE(8);
NEW_POINT_TYPE(9);
NEW_POINT_TYPE(10);
NEW_POINT_TYPE(11);
NEW_POINT_TYPE(12);
NEW_POINT_TYPE(13);
NEW_POINT_TYPE(14);
NEW_POINT_TYPE(15);
NEW_POINT_TYPE(16);


//============================================================================
// create (point x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT_OPERATOR(Op)																									\
	template <class T1, class T2, int DIMS>																		\
	inline typename PromotionOf<PointOf<DIMS,T1>,	PointOf<DIMS,T2> >::type				\
	operator Op (																																\
		const PointOf<DIMS,T1>& pt1, const PointOf<DIMS,T2>& pt2									\
	)																																						\
	{																																						\
		typename PromotionOf<PointOf<DIMS,T1>,PointOf<DIMS,T2> >::type ptOut;				\
																																							\
		for (int n=0; n<DIMS; n++){																								\
			ptOut.m_vx[n]=pt1.m_vx[n] Op pt2.m_vx[n];																\
		}																																					\
																																							\
		return ptOut;																															\
	}

POINT_OPERATOR(+);
POINT_OPERATOR(-);
POINT_OPERATOR(*);
POINT_OPERATOR(/);



//============================================================================
// create (point x point) operators
// where x is {+=,-=,*=,/=}
//============================================================================
#define POINT_SELFOPERATOR(_op)																								\
	template <class T1, class T2, int DIMS>																			\
	inline PointOf<DIMS,T1>&	operator _op (																		\
		PointOf<DIMS,T1>& pt1, const PointOf<DIMS,T2>& pt2												\
	)																																						\
	{																																						\
		for (int n=0; n<DIMS; n++){																								\
			pt1.m_vx[n] _op pt2.m_vx[n];																						\
		}																																					\
																																							\
		return pt1;																																\
	}

POINT_SELFOPERATOR(+=);
POINT_SELFOPERATOR(*=);
POINT_SELFOPERATOR(-=);
POINT_SELFOPERATOR(/=);


//============================================================================
// create (point x scalar) operators
// where x is {+=,-=,*=,/=}
//============================================================================
#define POINT_SELFSCALAROPERATOR(_op)																					\
	template <class T1, class T2, int DIMS>																			\
	inline PointOf<DIMS,T1>&	operator _op (																		\
		PointOf<DIMS,T1>& pt1, const T2& t2																				\
	)																																						\
	{																																						\
		for (int n=0; n<DIMS; n++){																								\
			pt1.m_vx[n] _op t2;																											\
		}																																					\
																																							\
		return pt1;																																\
	}

POINT_SELFSCALAROPERATOR(+=);
POINT_SELFSCALAROPERATOR(-=);
POINT_SELFSCALAROPERATOR(*=);
POINT_SELFSCALAROPERATOR(/=);


//============================================================================
// create (point x scalar) and (scalar x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT_SCALAROPERATOR(Op)																						\
	template <class T1, class T2, int DIMS>																		\
	inline typename PromotionOf<PointOf<DIMS,T1>,	T2 >::type											\
	operator Op (																																\
		const PointOf<DIMS,T1>& pt1, const T2& t2																	\
	)																																						\
	{																																						\
		typename PromotionOf<PointOf<DIMS,T1>,T2 >::type ptOut;											\
																																							\
		for (int n=0; n<DIMS; n++){																								\
			ptOut.m_vx[n]=pt1.m_vx[n] Op t2;																				\
		}																																					\
																																							\
		return ptOut;																															\
	}																																						\
																																							\
	template <class T1, class T2, int DIMS>																		\
	inline typename PromotionOf<T1,	PointOf<DIMS,T2> >::type											\
	operator Op (																																\
		const T1& t1, const PointOf<DIMS,T2>& pt2																	\
	)																																						\
	{																																						\
		typename PromotionOf<T1,PointOf<DIMS,T2> >::type ptOut;											\
																																							\
		for (int n=0; n<DIMS; n++){																								\
			ptOut.m_vx[n]=t1 Op pt2.m_vx[n];																				\
		}																																					\
																																							\
		return ptOut;																															\
	}

POINT_SCALAROPERATOR(+);
POINT_SCALAROPERATOR(-);
POINT_SCALAROPERATOR(*);
POINT_SCALAROPERATOR(/);


//============================================================================
// create (point x point) operators
// where x is {<,<=,>,>=,==,!=}
//============================================================================
#define POINT_COMPARATOR(_op,_test,_testresult)															\
	template <class T1, class T2, int DIMS>																			\
	inline bool operator _op (																									\
		const PointOf<DIMS,T1>& pt1, const PointOf<DIMS,T2>& pt2									\
	)																																						\
	{																																						\
		for (int n=0; n<DIMS; n++){																								\
			if ((pt1.m_vx[n] _op pt2.m_vx[n]) == _test)															\
				return _testresult;																										\
		}																																					\
		return !_testresult;																											\
	}

POINT_COMPARATOR(<,true,true);
POINT_COMPARATOR(<=,true,true);
POINT_COMPARATOR(>,true,true);
POINT_COMPARATOR(>=,true,true);
POINT_COMPARATOR(!=,true,true);
POINT_COMPARATOR(==,false,false);

//============================================================================
// create (point x point) operators
// where x is {<,<=,>,>=,==,!=}
//============================================================================
#define POINT_SCALARCOMPARATOR(_op,_test,_testresult)												\
	template <class T1, class T2, int DIMS>																			\
	inline bool operator _op (																									\
		const PointOf<DIMS,T1>& pt1,const T2& t2																	\
	)																																						\
	{																																						\
		for (int n=0; n<DIMS; n++){																								\
			if ((pt1.m_vx[n] _op t2) == _test)																			\
				return _testresult;																										\
		}																																					\
		return !_testresult;																											\
	}																																						\
																																							\
	template <class T1, class T2, int DIMS>																			\
	inline bool operator _op (																									\
		const T1& t1, const PointOf<DIMS,T2>& pt2																	\
	)																																						\
	{																																						\
		for (int n=0; n<DIMS; n++){																								\
			if ((t1 _op pt2.m_vx[n]) == _test)																			\
				return _testresult;																										\
		}																																					\
		return !_testresult;																											\
	}

POINT_SCALARCOMPARATOR(<,true,true);
POINT_SCALARCOMPARATOR(<=,true,true);
POINT_SCALARCOMPARATOR(>,true,true);
POINT_SCALARCOMPARATOR(>=,true,true);
POINT_SCALARCOMPARATOR(!=,true,true);
POINT_SCALARCOMPARATOR(==,false,false);












//============================================================================
// create f(point,point) functions which return points with the 
// function applied to each dimension separately
//============================================================================
#define POINT_FUNCTION(_fxn)																									\
	template <class T1, class T2, int DIMS>																			\
	inline typename PromotionOf<PointOf<DIMS,T1>,	PointOf<DIMS,T2> >::type				\
	_fxn (																																			\
		const PointOf<DIMS,T1>& pt1, const PointOf<DIMS,T2>& pt2									\
	)																																						\
	{																																						\
		typename PromotionOf<PointOf<DIMS,T1>,PointOf<DIMS,T2> >::type ptOut;				\
																																							\
		for (int n=0; n<DIMS; n++){																								\
			ptOut.m_vx[n]=_fxn(pt1.m_vx[n], pt2.m_vx[n]);														\
		}																																					\
																																							\
		return ptOut;																															\
	}

POINT_FUNCTION(min);
POINT_FUNCTION(max);


//============================================================================
// create f(point,scalar) and f(scalar,point) functions which return points 
// with the function applied to the scalar and each dimension separately
//============================================================================
#define POINT_SCALARFUNCTION(_fxn)																						\
	template <class T1, class T2, int DIMS>																			\
	inline typename PromotionOf<PointOf<DIMS,T1>,	T2 >::type											\
	_fxn (																																			\
		const PointOf<DIMS,T1>& pt1, const T2& t2																	\
	)																																						\
	{																																						\
		typename PromotionOf<PointOf<DIMS,T1>,T2 >::type ptOut;											\
																																							\
		for (int n=0; n<DIMS; n++){																								\
			ptOut.m_vx[n]=_fxn(pt1.m_vx[n], t2);																		\
		}																																					\
																																							\
		return ptOut;																															\
	}																																						\
																																							\
	template <class T1, class T2, int DIMS>																			\
	inline typename PromotionOf<T1,	PointOf<DIMS,T2> >::type											\
	_fxn (																																			\
		const T1& t1, const PointOf<DIMS,T2>& pt2																	\
	)																																						\
	{																																						\
		typename PromotionOf<T1,PointOf<DIMS,T2> >::type ptOut;											\
																																							\
		for (int n=0; n<DIMS; n++){																								\
			ptOut.m_vx[n]=_fxn(t1, pt2.m_vx[n]);																		\
		}																																					\
																																							\
		return ptOut;																															\
	}

POINT_SCALARFUNCTION(min);
POINT_SCALARFUNCTION(max);



//============================================================================
// create f(point) functions which return points with the 
// function applied to each dimension separately
//============================================================================
#define POINT_SOLOFUNCTION(_fxn)																							\
	template <class T1, int DIMS>																								\
	inline PointOf<DIMS,T1>																											\
	_fxn (																																				\
		const PointOf<DIMS,T1>& pt1																								\
	)																																						\
	{																																						\
		PointOf<DIMS,T1> ptOut;																										\
																																							\
		for (int n=0; n<DIMS; n++){																								\
			ptOut.m_vx[n]=_fxn(pt1.m_vx[n]);																				\
		}																																					\
																																							\
		return ptOut;																															\
	}

POINT_SOLOFUNCTION(exp);
POINT_SOLOFUNCTION(sqrt);
POINT_SOLOFUNCTION(cos);
POINT_SOLOFUNCTION(sin);
POINT_SOLOFUNCTION(tan);
POINT_SOLOFUNCTION(atan);







//==========================================================================
// Generic functions for all points
//==========================================================================
template <int DIMENSIONALITY, class T1, class T2>
typename PromotionOf<PointOf<DIMENSIONALITY,T1>,PointOf<DIMENSIONALITY,T2> >::type
Modulo(
	PointOf<DIMENSIONALITY,T1> t, PointOf<DIMENSIONALITY,T2> tBase
){
	PointOf<DIMENSIONALITY,int> cRem=PointOf<DIMENSIONALITY,int>(t/tBase);
	return t-cRem*tBase;
}





#endif //#ifndef __POINT_H__

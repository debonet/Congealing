// $Id$

#ifndef __POINT2D_H__
#define __POINT2D_H__

#include "Point.h"
#include "Promotion.h"
#include "libSerialize.h"
template <class PRECISION>
class PointOf<2,PRECISION>
{
public:
	PRECISION m_xX;
	PRECISION m_xY;

	typedef PRECISION type_precision;
	static const int type_dimensionality = 2;

public:
	PointOf(const PRECISION* vx){
		m_xX=vx[0];
		m_xY=vx[1];
	}

	template <class PRECISION2>
	PointOf(const PRECISION2& x){
		m_xX=x;
		m_xY=x;
	}

	PointOf()
	{
	}

	
	template<int DIMENSIONALITY2,class PRECISION2>
	PointOf(const PointOf<DIMENSIONALITY2,PRECISION2>& pt)
	{
		if (DIMENSIONALITY2>=1){
			m_xX=pt.Dim(0);
		}
		if (DIMENSIONALITY2>=2){
			m_xY=pt.Dim(1);
		}
	}

	template<int DIMENSIONALITY2,class PRECISION2>
	PointOf<2,PRECISION>& operator=(
		const PointOf<DIMENSIONALITY2,PRECISION2> &pt
	)
	{
		if (DIMENSIONALITY2>=1){
			m_xX=pt.Dim(0);
		}
		if (DIMENSIONALITY2>=2){
			m_xY=pt.Dim(1);
		}
		return *this;
	}



	String Describe() const
	{
		return String("(") +  m_xX + ", " +  m_xY + ")";
	}


	template <class OTHERPRECISION>
	PointOf(const PointOf<2,OTHERPRECISION> &pt){
		m_xX=pt.m_xX;
		m_xY=pt.m_xY;
	}

	const PRECISION& Dim(int n) const{
		return (n==0)?m_xX:m_xY;
	}
	PRECISION& Dim(int n) {
		return (n==0)?m_xX:m_xY;
	}

	PRECISION CVolume() const{
		return m_xX*m_xY;
	}

	PointOf<2,PRECISION>& AdvanceLoop(
		const PointOf<2,PRECISION>& ptBy=1
	){
		m_xX+=ptBy.m_xX;
		return *this;
	}

	bool StillLooping(
		const PointOf<2,PRECISION>& ptTo=0,
		const PointOf<2,PRECISION>& ptFrom=0,
		const PointOf<2,PRECISION>& ptBy=1
	){
		if (m_xX<ptTo.X()){
			return true;
		}

		if (m_xY+ptBy.m_xY<ptTo.Y()){
			m_xX=ptFrom.m_xX;
			m_xY+=ptBy.m_xY;
			return true;
		}

		return false;
	}

	PointOf(PRECISION xX, PRECISION xY){
		m_xX=xX;
		m_xY=xY;
	}

	const PRECISION& X() const {return m_xX;}
	const PRECISION& Y() const {return m_xY;}

	PRECISION& X() {return m_xX;}
	PRECISION& Y() {return m_xY;}

	PointOf<2,PRECISION> Bound(
		const PointOf<2,PRECISION> &ptMin,
		const PointOf<2,PRECISION> &ptMax
	) const {
		return PointOf<2,PRECISION>(
			::Bound(X(),ptMin.X(),ptMax.X()),
			::Bound(Y(),ptMin.Y(),ptMax.Y())
		);
	}


	int MaxDimension() const
	{
		return (m_xX > m_xY)?0:1;
	}

	int MinDimension() const
	{
		return (m_xX < m_xY)?0:1;
	}

	operator PointOf<2,Real>() const;

	static String SerializationId()
	{
		return "Point2D";
	}

	void Serialize(Stream& st) const
	{ 
		::Serialize(st,m_xX);
		::Serialize(st,m_xY);
	}

	void Deserialize(Stream& st)
	{
		::Deserialize(st,m_xX);
		::Deserialize(st,m_xY);
	}

};


//============================================================================
// quick names
//============================================================================
typedef PointOf<2,int> Point2D;
typedef PointOf<2,Real> Point2DReal;


//============================================================================
// casting operators
//============================================================================
template<>
inline Point2DReal::operator Point2DReal() const
{
	return *this;
}

template<>
inline Point2D::operator Point2DReal() const
{
	return Point2DReal(Real(m_xX), Real(m_xY));
}


//============================================================================
//============================================================================
template<class PRECISION>
inline PointOf<2,PRECISION> NewRandomPoint(
	const PointOf<2,PRECISION> &pt
)
{
	return PointOf<2,PRECISION> (
		Random(PRECISION(0),pt.X()),
		Random(PRECISION(0),pt.Y())
	);
}





//============================================================================
// create (point x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT2D_OPERATOR(Op)																						\
	template <class T1, class T2>																					\
	inline typename PromotionOf<PointOf<2,T1>,PointOf<2,T2> >::type operator Op (	\
		const PointOf<2,T1>& pt1, const PointOf<2,T2>& pt2									\
	)																																			\
	{																																			\
		return typename PromotionOf<PointOf<2,T1>,PointOf<2,T2> >::type(			\
			pt1.X() Op pt2.X(),pt1.Y() Op pt2.Y()															\
		);																																	\
	}

POINT2D_OPERATOR(+);
POINT2D_OPERATOR(-);
POINT2D_OPERATOR(*);
POINT2D_OPERATOR(/);



//============================================================================
// create (point x point) operators
// where x is {+=,-=,*=,/=}
//============================================================================
#define POINT2D_SELFOPERATOR(_op)																				\
	template <class T1, class T2>																					\
	inline PointOf<2,T1> operator _op (																		\
		PointOf<2,T1>& pt1, const PointOf<2,T2>& pt2												\
	)																																			\
	{																																			\
		pt1.X() _op pt2.X();																								\
		pt1.Y() _op pt2.Y();																								\
		return pt1;																													\
	}																																			\

POINT2D_SELFOPERATOR(+=);
POINT2D_SELFOPERATOR(*=);
POINT2D_SELFOPERATOR(-=);
POINT2D_SELFOPERATOR(/=);


//============================================================================
// create (point x scalar) operators
// where x is {+=,-=,*=,/=}
//============================================================================
#define POINT2D_SELFSCALAROPERATOR(_op)																	\
	template <class T1, class T2>																					\
	inline PointOf<2,T1> operator _op (																		\
		PointOf<2,T1>& pt1, const T2& t2																		\
	)																																			\
	{																																			\
		pt1.X() _op t2;																											\
		pt1.Y() _op t2;																											\
		return pt1;																													\
	}																																			

POINT2D_SELFSCALAROPERATOR(+=);
POINT2D_SELFSCALAROPERATOR(-=);
POINT2D_SELFSCALAROPERATOR(*=);
POINT2D_SELFSCALAROPERATOR(/=);



//============================================================================
// create (point x scalar) and (scalar x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT2D_SCALAROPERATOR(Op)																	\
	template <class T1, class T2>																			\
	inline typename PromotionOf<PointOf<2,T1>,T2 >::type operator Op (	\
		const PointOf<2,T1>& pt1, const T2& t2													\
	)																																	\
	{																																	\
		return typename PromotionOf<PointOf<2,T1>,T2 >::type(							\
			pt1.X() Op t2,pt1.Y() Op t2																		\
		);																															\
	}																																	\
	template <class T1, class T2>																			\
	inline typename PromotionOf<T1,PointOf<2,T2> >::type operator Op (	\
		const T1& t1, const PointOf<2,T2>& pt2													\
	)																																	\
	{																																	\
		return typename PromotionOf<T1,PointOf<2,T2> >::type(							\
			t1 Op pt2.X(),t1 Op pt2.Y()																		\
		);																															\
	}

POINT2D_SCALAROPERATOR(+);
POINT2D_SCALAROPERATOR(-);
POINT2D_SCALAROPERATOR(*);
POINT2D_SCALAROPERATOR(/);




//============================================================================
// create (point x point) operators
// where x is {<,<=,>,>=,==,!=}
//============================================================================
#define POINT2D_COMPARATOR(_op,_opCombine)							\
	template <class T1, class T2>													\
	inline bool operator _op (														\
		const PointOf<2,T1>& pt1, const PointOf<2,T2>& pt2	\
	)																											\
	{																											\
		return (																						\
			pt1.X() _op pt2.X()																\
			_opCombine pt1.Y() _op pt2.Y()										\
		);																									\
	}

POINT2D_COMPARATOR(<,||);
POINT2D_COMPARATOR(<=,||);
POINT2D_COMPARATOR(>,||);
POINT2D_COMPARATOR(>=,||);
POINT2D_COMPARATOR(!=,||);
POINT2D_COMPARATOR(==,&&);

//============================================================================
// create (point x point) operators
// where x is {<,<=,>,>=,==,!=}
//============================================================================
#define POINT2D_SCALARCOMPARATOR(_op,_opCombine)	\
	template <class T1, class T2>										\
	inline bool operator _op (											\
		const PointOf<2,T1>& pt1,const T2& t2					\
	)																								\
	{																								\
		return (																			\
			pt1.X() _op t2															\
			_opCombine pt1.Y() _op t2										\
		);																						\
	}																								\
	template <class T1, class T2>										\
	inline bool operator _op (											\
		const T1& t1, const PointOf<2,T2>& pt2				\
	)																								\
	{																								\
		return (																			\
			t1 _op pt2.X()															\
			_opCombine t1 _op pt2.Y()										\
		);																						\
	}

POINT2D_SCALARCOMPARATOR(<,||);
POINT2D_SCALARCOMPARATOR(<=,||);
POINT2D_SCALARCOMPARATOR(>,||);
POINT2D_SCALARCOMPARATOR(>=,||);
POINT2D_SCALARCOMPARATOR(!=,||);
POINT2D_SCALARCOMPARATOR(==,&&);




//============================================================================
// create (point x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT2D_FUNCTION(_fxn)																								\
	template <class T1, class T2>																								\
	inline typename PromotionOf<PointOf<2,T1>,PointOf<2,T2> >::type								\
	_fxn (																																			\
		const PointOf<2,T1>& pt1, const PointOf<2,T2>& pt2												\
	)																																						\
	{																																						\
		return typename PromotionOf<PointOf<2,T1>,PointOf<2,T2> >::type(						\
			_fxn(pt1.X(), pt2.X()), _fxn(pt1.Y(), pt2.Y())													\
		);																																				\
	}

POINT2D_FUNCTION(min);
POINT2D_FUNCTION(max);


//============================================================================
// create (point x scalar) and (scalar x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT2D_SCALARFUNCTION(_fxn)																\
	template <class T1, class T2>																			\
	inline typename PromotionOf<PointOf<2,T1>,T2 >::type								\
	_fxn (																														\
		const PointOf<2,T1>& pt1, const T2& t2													\
	)																																	\
	{																																	\
		return typename PromotionOf<PointOf<2,T1>,T2 >::type(							\
			_fxn(pt1.X(), t2) , _fxn(pt1.Y(), t2)													\
		);																															\
	}																																	\
	template <class T1, class T2>																			\
	inline typename PromotionOf<T1,PointOf<2,T2> >::type								\
	_fxn (																														\
		const T1& t1, const PointOf<2,T2>& pt2													\
	)																																	\
	{																																	\
		return typename PromotionOf<T1,PointOf<2,T2> >::type(							\
			_fxn(t1, pt2.X()), _fxn(t1, pt2.Y())													\
		);																															\
	}

POINT2D_SCALARFUNCTION(min);
POINT2D_SCALARFUNCTION(max);

//============================================================================
// create (point x scalar) and (scalar x point) operators
// where x is {+,-,*,/}
//============================================================================
#define POINT2D_SOLOFUNCTION(_fxn)																	\
	template <class T1>																								\
	inline PointOf<2,T1>																							\
	_fxn (																														\
		const PointOf<2,T1>& pt1																				\
	)																																	\
	{																																	\
		return PointOf<2,T1>(																						\
			_fxn(pt1.X()) , _fxn(pt1.Y())																	\
		);																															\
	}																																	\


POINT2D_SOLOFUNCTION(exp);
POINT2D_SOLOFUNCTION(sqrt);
POINT2D_SOLOFUNCTION(cos);
POINT2D_SOLOFUNCTION(sin);
POINT2D_SOLOFUNCTION(tan);
POINT2D_SOLOFUNCTION(atan);




#endif //#ifndef __POINT2D_H__

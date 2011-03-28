#ifndef __PROMOTION_H__
#define __PROMOTION_H__

#include "libBaseTypes.h"

template<class T1, class T2>
class PromotionOf
{
public:
};


template<class T>
class PromotionOf<T,T>
{
public:
	typedef T type;
};


#define DEFINE_SYMMETRICPROMOTION(T1,T2,TPromo)														 \
	template<> class PromotionOf<T1,T2>{public:	typedef TPromo type;	}; \
	template<> class PromotionOf<T2,T1>{public:	typedef TPromo type;	}; 

#define PROMOTION(T1,T2) typename PromotionOf<T1,T2>::type


//DEFINE_SYMMETRICPROMOTION(byte, Real, Real);
//DEFINE_SYMMETRICPROMOTION(int, Real, Real);
DEFINE_SYMMETRICPROMOTION(byte, int, int);

DEFINE_SYMMETRICPROMOTION(byte, float, float);
DEFINE_SYMMETRICPROMOTION(byte, double, double);
DEFINE_SYMMETRICPROMOTION(short, float, float);
DEFINE_SYMMETRICPROMOTION(short, double, double);
DEFINE_SYMMETRICPROMOTION(int, float, float);
DEFINE_SYMMETRICPROMOTION(int, double, double);
DEFINE_SYMMETRICPROMOTION(float, double, double);




template<class T>
class RangePromotionOf
{
public:
	typedef T type;
};


#define DEFINE_RANGEPROMOTION(T,TPromo)															 	\
	template<> class RangePromotionOf<T>{public:	typedef TPromo type;	}; 

#define RANGEPROMOTION(T) typename RangePromotionOf<T>::type

DEFINE_RANGEPROMOTION(byte,int);

#endif //#ifndef __PROMOTION_H__

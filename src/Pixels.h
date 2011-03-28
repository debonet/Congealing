#ifndef __PIXELS_H__ 
#define __PIXELS_H__ 

#include "libBaseTypes.h"
#include "Promotion.h"
#include "Point.h"

typedef byte Pixel8BitGrey;
typedef short Pixel12BitGrey;
typedef Real PixelRealGrey;

// TODO: convert pixels to points?
//typedef PointOf<1,byte> Pixel8BitGrey;
//typedef PointOf<1,Real> PixelRealGrey;
//typedef PointOf<1,int> Pixel32BitGrey;
//DEFINE_SYMMETRICPROMOTION(Pixel8BitGrey,Real,PixelRealGrey);
//DEFINE_SYMMETRICPROMOTION(Pixel8BitGrey,int,Pixel32BitGrey);
//DEFINE_RANGEPROMOTION(Pixel8BitGrey,Pixel32BitGrey);

DEFINE_RANGEPROMOTION(Pixel12BitGrey,int);



template<class T>
class PixelInfoOf
{};

#define _PIXEL_MMMFunctions(_type,_min,_mid,_max,_range)	\
	template<>																							\
	class PixelInfoOf<_type>																\
	{																												\
	public:																									\
		static const _type min = _min;												\
		static const _type mid = _mid;												\
		static const _type max = _max;												\
		static const RangePromotionOf<_type>::type range = _range;	\
	};																											

#define MinValue(_type) PixelInfoOf<_type>::min
#define MidValue(_type) PixelInfoOf<_type>::mid
#define MaxValue(_type) PixelInfoOf<_type>::max
#define Range(_type) PixelInfoOf<_type>::range

_PIXEL_MMMFunctions(Pixel8BitGrey,0,127,255,256);
_PIXEL_MMMFunctions(Pixel12BitGrey,0,2047,4095,4096);

template <class T>
int SignOfDiff(const T& t1, const T& t2)
{
	return t1>t2?1:-1;
}

template <class T>
int AbsDiff(const T& t1, const T& t2)
{
	return (t1<t2)?t2-t1:t1-t2;
}


inline void Serialize(Stream& st, const Pixel8BitGrey& t)
{
	return st.Append(t);
}

inline void Deserialize(Stream& st, Pixel8BitGrey& t)
{
	t=st.Read();
}

#endif //__PIXELS_H__ 


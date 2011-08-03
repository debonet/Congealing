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


/// provides meta information about its template argument pixel
/// class. Provided members are:
///  \li \c mid mid point of the pixel
///  \li \c min minimum value of pixel
///  \li \c max maximum value of pixel
///  \li \c range max-min + 1
/// \tparam T the pixel class whose information is to be provided
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


/// the minimum value of a particular pixel type
/// \param _type type of pixel
#define MinValue(_type) PixelInfoOf<_type>::min

/// the middle value of a particular pixel type
/// \param _type type of pixel
#define MidValue(_type) PixelInfoOf<_type>::mid

/// the maximum value of a particular pixel type
/// \param _type type of pixel
#define MaxValue(_type) PixelInfoOf<_type>::max

/// the range of a particular pixel type
/// \param _type type of pixel
#define Range(_type) PixelInfoOf<_type>::range

_PIXEL_MMMFunctions(Pixel8BitGrey,0,127,255,256);
_PIXEL_MMMFunctions(Pixel12BitGrey,0,2047,4095,4096);

/// the sign of the difference of two pixels
/// \tparam T pixel type
/// \param t1 one pixel
/// \param t2 another pixel
/// \returns -1 if negative, 1 otherwise
template <class T>
int SignOfDiff(const T& t1, const T& t2)
{
	return t1>t2?1:-1;
}

/// the absolute difference of two pixels
/// \tparam T pixel type
/// \param t1 one pixel
/// \param t2 another pixel
/// \returns the absolute difference of two pixel values
template <class T>
int AbsDiff(const T& t1, const T& t2)
{
	return (t1<t2)?t2-t1:t1-t2;
}


/// the stream serialization of a pixel
/// \param st the stream
/// \param t the pixel to be setialized
inline void Serialize(Stream& st, const Pixel8BitGrey& t)
{
	return st.Append(t);
}

/// the stream deserialization of a pixel
/// \param st the stream
/// \param t the pixel into which the value is to be stored
inline void Deserialize(Stream& st, Pixel8BitGrey& t)
{
	t=st.Read();
}

#endif //__PIXELS_H__ 


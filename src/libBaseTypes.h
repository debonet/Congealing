#ifndef __LIBBASETYPES_H__
#define __LIBBASETYPES_H__


#include <float.h> // for REAL_MAX

//==========================================================================
//==========================================================================
#define PI 3.14


//==========================================================================
//==========================================================================
#define REAL_AS_DOUBLE 1
#if REAL_AS_DOUBLE
typedef double Real;
#  define REAL_MAX DBL_MAX
#  define SFMT_REAL "%lf"
#else
typedef float Real;
#  define REAL_MAX FLT_MAX
#  define SFMT_REAL "%f"
#endif


//==========================================================================
//==========================================================================
template<class T>
inline const char *ScanFormat(){ return ""; }
template<class T>
inline const char *PrintFormat(){ return ""; }

template<>
inline const char *ScanFormat<int>(){ return "%d"; }
template<>
inline const char *PrintFormat<int>(){ return "%d"; }

template<>
inline const char *ScanFormat<double>(){ return "%lf"; }
template<>
inline const char *PrintFormat<double>(){ return "%20.20lf"; }

template<>
inline const char *ScanFormat<float>(){ return "%f"; }
template<>
inline const char *PrintFormat<float>(){ return "%20.20f"; }

//==========================================================================
//==========================================================================
typedef Real Parameter;

//==========================================================================
//==========================================================================
typedef unsigned char byte;

//==========================================================================
//==========================================================================
typedef enum
{
	Source,
	SourceAccessor,
	SourceAccessorCubicInterpolate,
	SourceAccessorEdgePad,
	SourceAccessorLinearInterpolate,
	SourceAccessorNearestNeighbor,
	SourceMemory,
	SourceTransform,
	SourceTransform2D,
	SourceTransformRotate,
	SourceTransformScale,
	SourceTransformTranslate,
	SourceTransformWarp
} SourceType;

//==========================================================================
//==========================================================================
typedef enum
{
	Nearest=0,
	Linear=1,
	Cubic=3
} InterpolationType;

//==========================================================================
//==========================================================================
typedef enum
{
	DimensionX=0,
	DimensionY=1,
	DimensionZ=2,
} DimensionType;


//==========================================================================
//==========================================================================
typedef enum
{
	Zero,
	Reflect,
	Repeat,
	Wrap,
} PaddingType;


#endif //__LIBBASETYPES_H__

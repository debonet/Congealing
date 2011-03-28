//============================================================================
// Transform List
//============================================================================
#include "Pixels.h"

#include "SourceGenerics.h"

#include "SourceTransformRotate.h"
#include "SourceTransformTranslate.h"
#include "SourceTransformScale.h"
#include "SourceTransformWarp.h"
#include "SourceTransformContrast.h"
#include "SourceTransformRemap.h"
#include "SourceTransformDataScale.h"
#include "SourceTransformSpatialScale.h"

#include "SourceAccessorEdgePad.h"
#include "SourceAccessorNearestNeighbor.h"
#include "SourceAccessorLinearInterpolate.h"
#include "SourceAccessorCubicInterpolate.h"
#include "SourceAccessorSlice.h"
#include "SourceAccessorConvolution.h"
#include "SourceAccessorAperature.h"
#include "SourceAccessorDistributionField.h"
#include "SourceAccessorPointwiseOperator.h"

#include "SourceCombineLayout.h"


//============================================================================
// Transform Registry
//============================================================================
//   can assume environment with
// 
//    template <class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
//
// 
#define ALL_TRANSFORMS_DO(_do)																					\
	_do(Rotate,       SourceTransformRotateOf,	   <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Scale,        SourceTransformScaleOf,     <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Warp,         SourceTransformWarpOf,      <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Translate,    SourceTransformTranslateOf, <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Contrast,     SourceTransformContrastOf,  <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Remap,        SourceTransformRemapOf,     <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(DataScale,    SourceTransformDataScaleOf, <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(SpatialScale, SourceTransformSpatialScaleOf, <DATA,DIMENSIONALITY,PRECISION,SOURCE>) 


//============================================================================
// ACCESSORS Registry
//============================================================================
//   can assume environment with
// 
//    template <class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
// 
#define ALL_ACCESSORS_DO(_do)																									\
	_do(Nearest,     SourceAccessorNearestNeighborOf,	   <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Linear,      SourceAccessorLinearInterpolateOf,  <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Cubic,       SourceAccessorCubicInterpolateOf,   <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Pad,         SourceAccessorEdgePadOf,            <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Slice,       SourceAccessorSliceOf,              <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Convolution, SourceAccessorConvolutionOf,        <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(Aperature,   SourceAccessorAperatureOf,          <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
	_do(DField,      SourceAccessorDistributionFieldOf,  <DATA,DIMENSIONALITY,PRECISION,SOURCE>) \
\
	_do(Layout,      SourceCombineLayoutOf,  <DATA,DIMENSIONALITY,PRECISION,SOURCE>) 

#define ALL_SOURCES_DO(_do)											\
	ALL_TRANSFORMS_DO(_do)												\
	ALL_ACCESSORS_DO(_do) 


//============================================================================
// Source Nomenclature
//============================================================================
//
//               sourcename := "Source_" type ["_" modifier "_" underlying-source-type]
//
// where 
//
//                     type := data-type dimensions precision-type
//
//                data-type := { "G" (Pixel8BitGrey) , ...}
//
//               dimensions := integer
//
//                precision := { "I" (int), "R" (real), ... }
//
//                 modifier := transformation | accessor
//
//           transformation := { "Rotate", "Warp", "Scale", ... }
//
//                 accessor := { "NearestNeighbor", "Linear", "Cubic", ...}
//
//   underlying-source-type := type
//   
// examples
//
//   Source_G2I = 
//     a generic source of Pixel8BitGrey from a 2D grid indexed by ints
//
//   Source_G3R_Linear_G3I = 
//     a linear interpolation (indexed by reals) of a 3d int-indexed source 
//       of Pixel8BitGrey
//

typedef SourceOf<Pixel8BitGrey, 2, int>    Source_G2I;
typedef SourceOf<Pixel8BitGrey, 2, Real>   Source_G2R;

typedef SourceOf<Pixel8BitGrey, 3, int>   Source_G3I;
typedef SourceOf<Pixel8BitGrey, 3, Real>  Source_G3R;

typedef SourceOf<Pixel12BitGrey, 2, int>   Source_S2I;
typedef SourceOf<Pixel12BitGrey, 2, Real>  Source_S2R;
typedef SourceOf<Pixel12BitGrey, 3, int>   Source_S3I;
typedef SourceOf<Pixel12BitGrey, 3, Real>  Source_S3R;

#define _SetUpModifierInner(_dataname,_data,_dimension,_name,_class)  \
	typedef _class<_data, _dimension, int,  Source_##_dataname##_dimension##I> \
	   Source_##_dataname##_dimension##I_##_name##_##_dataname##_dimension##I; \
	typedef _class<_data, _dimension, Real, Source_##_dataname##_dimension##I> \
     Source_##_dataname##_dimension##R_##_name##_##_dataname##_dimension##I; \
	typedef _class<_data, _dimension, int,  Source_##_dataname##_dimension##R> \
     Source_##_dataname##_dimension##I_##_name##_##_dataname##_dimension##R; \
	typedef _class<_data, _dimension, Real, Source_##_dataname##_dimension##R> \
     Source_##_dataname##_dimension##R_##_name##_##_dataname##_dimension##R; 

#define _SetUpModifier(_name,_class,_templateparams...)	\
	_SetUpModifierInner(G,Pixel8BitGrey,2,_name,_class) \
	_SetUpModifierInner(G,Pixel8BitGrey,3,_name,_class) \
	_SetUpModifierInner(S,Pixel12BitGrey,2,_name,_class) \
	_SetUpModifierInner(S,Pixel12BitGrey,3,_name,_class) 

// set up all the names for all the functional sources
ALL_SOURCES_DO(_SetUpModifier);

// set up a generic Transform name
_SetUpModifier(Transform,SourceTransformOf,<DATA,DIMENSIONALITY,PRECISION,SOURE>);


typedef SourceAccessorSliceOf<Pixel12BitGrey,2,int,Source_S3I> Source_S2I_Slice_S3I;
typedef SourceAccessorSliceOf<Pixel8BitGrey,2,int,Source_G3I> Source_G2I_Slice_G3I;
typedef SourceAccessorSliceOf<Pixel8BitGrey,2,Real,Source_G3R> Source_G2R_Slice_G3R;
typedef SourceAccessorSliceOf<Pixel8BitGrey,2,int,Source_G3R> Source_G2I_Slice_G3R;

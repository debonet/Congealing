#ifndef __LIBBRAINWEB_H__
#define __LIBBRAINWEB_H__

#include "SourceRegistry.h"

#if USE_CUBIC_IO
#  define BRAIN_WEB_INTERPOLANT SourceAccessorCubicInterpolateOf
#  define BRAIN_WEB_INTERPOLANT_FUNCTION CubicInterpolation
#else
#  define BRAIN_WEB_INTERPOLANT SourceAccessorNearestNeighborOf
#  define BRAIN_WEB_INTERPOLANT_FUNCTION NearestNeighbor
#endif

typedef SourceTransformScaleOf<
	Pixel8BitGrey,3,Real,
	BRAIN_WEB_INTERPOLANT<Pixel8BitGrey,3,Real,GreyVolume> > 
BrainWebVolume;


//============================================================================
//============================================================================
BrainWebVolume* ReadBrainWeb(const char* sfl);

#endif //__LIBBRAINWEB_H__

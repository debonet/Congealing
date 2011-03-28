#ifndef __LIBOPTIMIZE_NELDERMEAD_H__
#define __LIBOPTIMIZE_NELDERMEAD_H__

#include <stdlib.h>
#include "libBaseTypes.h"

//============================================================================
//============================================================================
Parameter NelderMeadOptimize(
	int cDimensions,
	Parameter* vrParamMin,
	Parameter* vrInitialScale,
	Parameter fxnEval(const Parameter*, const Parameter& rErrMin, const void*),
	const void* pData,
	Real rErrThreshold,
	int cIterations
);
#endif //#define __LIBOPTIMIZE_NELDERMEAD_H__

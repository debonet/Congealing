#ifndef __LIBOPTIMIZE_RANDOMWALK_H__
#define __LIBOPTIMIZE_RANDOMWALK_H__
#include <stdlib.h>

//============================================================================
//============================================================================
bool Optimize_RandomWalk(
	int cDimensions,
	double* vrParamMin,
	double* vrInitialScale,
	double fxnEval(double*, const double& rErrMin, const void*),
	const void* pData=NULL,
	double rErrThreshold = .01,
	int cIterations = 100000
);
#endif

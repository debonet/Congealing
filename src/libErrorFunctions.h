#ifndef __LIBERRORFUNCTIONS_H__
#define __LIBERRORFUNCTIONS_H__


#include "Promotion.h"
#include "libUtility.h"

//============================================================================
//============================================================================
template <class PIXEL>
inline Real VarianceOfSet(int cpxl, const PIXEL* vpxl)
{

	RANGEPROMOTION(PIXEL) pxlSum=0;

	for (int npxl=0; npxl<cpxl; npxl++){
		pxlSum+=vpxl[npxl];
	}
	PROMOTION(PIXEL,Real) rMean = pxlSum/Real(cpxl);

	PROMOTION(PIXEL,Real) rVar=0;
	for (int npxl=0; npxl<cpxl; npxl++){
		rVar+=sqr(vpxl[npxl]-rMean);
	}

	rVar /= cpxl;

	return rVar;
}


#endif // __LIBERRORFUNCTIONS_H__

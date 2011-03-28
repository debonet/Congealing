#ifndef __LIBGAUSSIAN_H__
#define __LIBGAUSSIAN_H__

#include "Image.h"

//============================================================================
//============================================================================
template <class PRECISION> 
inline typename PromotionOf<PRECISION,Real>::type
Gaussian(
	const PRECISION& rWhere, const PRECISION& rMean, const PRECISION& rSigma
){
	static const Real grSqrt2Pi = 2.50662827;

	return exp(-1 * sqr((rWhere-rMean)/rSigma) / 2)/rSigma/grSqrt2Pi;
}

//============================================================================
//============================================================================
template <class PRECISION> 
inline PROMOTION(PRECISION,Real)
Gaussian(
	const PRECISION& rWhere, const PRECISION& rMean, const PROMOTION(PRECISION,Real)& rSigma
){
	return Gaussian(
		PROMOTION(PRECISION,Real)(rWhere),
		PROMOTION(PRECISION,Real)(rMean),
		PROMOTION(PRECISION,Real)(rSigma)
	);
}


//============================================================================
//============================================================================
template<int DIMENSIONALITY>
ImageOf<Real,DIMENSIONALITY>* 
GaussianCurve(
	const PointOf<DIMENSIONALITY,Real>& ptrSigma, 
	const PointOf<DIMENSIONALITY,int>& ptSupport
)
{
	typedef  PointOf<DIMENSIONALITY,int> POINT;
	typedef ImageOf<Real,DIMENSIONALITY> SRC_OUT;

	SRC_OUT* psrc = new SRC_OUT();
	psrc->Allocate(ptSupport);

	const POINT ptMean=ptSupport/2;

	forpoint(POINT,pt,0,ptSupport){
		SetPoint(*psrc,pt,Gaussian(pt,ptMean,ptrSigma).CVolume());
	}
	HandoffPointer(psrc);
	return psrc;
}


#endif //#ifdef __LIBGAUSSIAN_H__

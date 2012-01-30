#ifndef __LIBASSISTANTS_H__
#define __LIBASSISTANTS_H__

#include "SourceRegistry.h"
#include "SourceCombinePointwiseOperator.h"
#include "SourceMaskCombinePointwiseOperator.h"

// TODO: find proper homes for these things


//============================================================================
// Generic Interpolate assistant
//============================================================================
// uses MAKE_ASSISTANT() created assistant functions
template <class SOURCE>
SourceAccessorOf<TypeOfData(SOURCE),	TypeOfDimensionality(SOURCE),	Real, SOURCE>*
Interpolate(InterpolationType it, SOURCE* psrc)
{
	switch(it){
	case Nearest:	return NearestNeighbor(psrc);
	case Linear:  return LinearInterpolation(psrc);
	case Cubic:   return CubicInterpolation(psrc);
	default:{
		CONGEAL_ERROR("unknown interpolation type %d", it);
	}
	}
}


//============================================================================
// Rasterize assistant
//============================================================================
template <class SOURCE>			
inline ImageOf<TypeOfData(SOURCE), TypeOfDimensionality(SOURCE)>*
Rasterize(SOURCE* psrcIn)
{
	typedef ImageOf<TypeOfData(SOURCE), TypeOfDimensionality(SOURCE)> SRC_OUT;

	ClaimPointer(psrcIn);
	SRC_OUT *psrcOut=new SRC_OUT();
	AllocatedCopy(*psrcOut,psrcIn);

	ReleasePointer(psrcIn);
	HandoffPointer(psrcOut);																							
	return psrcOut;
}

//============================================================================
// Rasterize assistant
//============================================================================
template <class SOURCE_OUT, class SOURCE_IN>			
inline SOURCE_OUT* 
RasterizeInto(SOURCE_OUT *psrcOut, SOURCE_IN* psrcIn)
{
	ClaimPointer(psrcIn);
	ClaimPointer(psrcOut);
	AllocatedCopy(*psrcOut,psrcIn);
	ReleasePointer(psrcIn);
	HandoffPointer(psrcOut);
	return psrcOut;
}





//============================================================================
// GaussianBlurAlong assistant 
//    provides Gaussian Blur Along a single dimension
//============================================================================
MAKE_ASSISTANT(
	GaussianBlurAlong,
	SourceAccessorConvolutionOf,
	({
		PointOf<TypeOfDimensionality(SOURCE),  Real> ptSigma(1.);
		ptSigma.Dim(int(dt))=rSigma;
		PointOf<TypeOfDimensionality(SOURCE), int> ptSupport(1);
		ptSupport.Dim(int(dt))=cSupport;
		
		psrcOut->SetFilter(GaussianCurve(ptSigma,  ptSupport));
	}),
	const DimensionType& dt, const Real& rSigma, const int& cSupport,
);


//============================================================================
// DownsampleAlong assistant 
//    provides Downsampling with 1-2-1 filtering along a single dimension
//============================================================================
template <class SOURCE>			
inline SourceAccessorAperatureOf<
	TypeOfData(SOURCE),
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	SourceTransformScaleOf<
		TypeOfData(SOURCE),
		TypeOfDimensionality(SOURCE),
		TypeOfPrecision(SOURCE),
		SourceAccessorConvolutionOf<
			TypeOfData(SOURCE),
			TypeOfDimensionality(SOURCE),
			TypeOfPrecision(SOURCE),
			SOURCE
> > > *
DownsampleAlong(
	const DimensionType& dt, 
	SOURCE* psrcIn
)
{
	psrcIn->PrepareForAccess();

	typedef PointOf<TypeOfDimensionality(SOURCE),int> POINT;
	POINT ptSupport(1);
	ptSupport.Dim(int(dt))=3;
	ImageOf<Real,TypeOfDimensionality(SOURCE)>* piFilter=new ImageOf<Real,TypeOfDimensionality(SOURCE)>;
	piFilter->Allocate(ptSupport);

	// NOTE: is there a cleaner way to do this?
	Real vr[3]={.25,.5,.25};
	int n=0;
	forpoint(POINT,pt,0,ptSupport){
		SetPoint(*piFilter,pt,vr[n]);
		n++;
	}

	PointOf<TypeOfDimensionality(SOURCE),Real> ptScale(1.);
	ptScale.Dim(int(dt))=.5;

	typedef SourceAccessorConvolutionOf<
		TypeOfData(SOURCE),
		TypeOfDimensionality(SOURCE),
		TypeOfPrecision(SOURCE),
		SOURCE
	> SRC_OUT;			

	SRC_OUT *psrcOut=new SRC_OUT;
	psrcOut->SetSource(psrcIn);
	psrcOut->SetFilter(HandoffPointer(piFilter));
	return Aperature(
		PointOf<TypeOfDimensionality(SOURCE),Real>(0),
		ptScale*psrcIn->Size(),
		ScaleBy(
			ptScale,
			HandoffPointer(psrcOut)
		)
	);																							
}


//============================================================================
//============================================================================
template <class DATA, class PRECISION>
ImageOf<DATA,2>* RasterizeDownsample(
	SourceOf<DATA,2,PRECISION>* psrcIn
)
{
	return Rasterize(
		DownsampleAlong(
			DimensionY,
			Rasterize(
				DownsampleAlong(
					DimensionX,
					psrcIn
				)
			)
		)
	);
}


//============================================================================
//============================================================================
template <class SOURCE>
ImageOf<TypeOfData(SOURCE),TypeOfDimensionality(SOURCE)>* RasterizeDownsample(
	SOURCE* psrcIn
)
{
	return Rasterize(
		DownsampleAlong(
			DimensionZ,
			Rasterize(
				DownsampleAlong(
					DimensionY,
					Rasterize(
						DownsampleAlong(
							DimensionX,
							psrcIn
						)
					)
				)
			)
		)
	);
}


//============================================================================
//============================================================================
template <class SOURCE>
ImageOf<TypeOfData(SOURCE),TypeOfDimensionality(SOURCE)>* RasterizeDownsample(
	int cTimes,
	SOURCE* psrcIn
)
{
	if (cTimes==0){
		return Rasterize(psrcIn);
	}

	ClaimPointer(psrcIn);
	ImageOf<TypeOfData(SOURCE),TypeOfDimensionality(SOURCE)>* piOut=RasterizeDownsample(psrcIn);
	ReleasePointer(psrcIn);
	while(cTimes>1){
		ImageOf<TypeOfData(SOURCE),TypeOfDimensionality(SOURCE)>* piT=RasterizeDownsample(piOut);
		piOut=piT;
		cTimes--;
	}
	return piOut;
}




#endif

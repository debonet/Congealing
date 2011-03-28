#ifndef __LIBDOWNSAMPLE_H__
#define __LIBDOWNSAMPLE_H__

#include "Image.h"
#include "libSourceOperations.h"



//============================================================================
// Generic
//============================================================================


//============================================================================
//============================================================================
template <class DATA, int DIMENSIONALITY, class PRECISION>
ImageOf<DATA,DIMENSIONALITY> DownsampleAlong(
	const PointOf<DIMENSIONALITY, PRECISION> &ptDimension,
	const SourceOf<DATA,DIMENSIONALITY,PRECISION>& iIn
)
{
	typedef PointOf<DIMENSIONALITY,PRECISION> POINT;

	ImageOf<DATA,DIMENSIONALITY> iOut;
	
	POINT ptScale=POINT(1) + ptDimension;
	iOut.Allocate(iIn.Size()/ptScale);

	DATA pxlm1;
	DATA pxl0;
	DATA pxlp1;

	forpoint(POINT,pt,0,iOut.Size()){
		GetPoint(iIn,pxlm1,pt * ptScale - ptDimension);
		GetPoint(iIn,pxl0,pt * ptScale);
		GetPoint(iIn,pxlp1,pt * ptScale + ptDimension);
		SetPoint(iOut,pt,(pxlm1 + 2*pxl0	+ pxlp1	)/4);
	}

	return iOut;
}

//============================================================================
//============================================================================
template <class DATA, int DIMENSIONALITY, class PRECISION>
ImageOf<DATA,DIMENSIONALITY> Downsample(
	int cTimes,
	const SourceOf<DATA,DIMENSIONALITY,PRECISION>& srcIn,
)
{
	if (cTimes==0){
		ImageOf<DATA,DIMENSIONALITY> iOut;
		AllocatedCopy(iOut,srcIn);
		return iOut;
	}

	ImageOf<DATA,DIMENSIONALITY> iOut=Downsample(srcIn);
	while(cTimes>1){
		ImageOf<DATA,DIMENSIONALITY> iT=Downsample(iOut);
		iOut=iT;
		cTimes--;
	}

	return iOut;
}



//============================================================================
// 2D
//============================================================================
template <class DATA, class PRECISION>
ImageOf<DATA,2>Downsample(
	const SourceOf<DATA,2,PRECISION>& src
)
{
	ImageOf<DATA,2> iT0  = DownsampleAlong(src,Point2D(1,0));
	ImageOf<DATA,2> iOut = DownsampleAlong(iT0,Point2D(0,1));

	return iOut;
}


//============================================================================
// 3D
//============================================================================
template <class DATA, class PRECISION>
ImageOf<DATA,3>Downsample(
	const SourceOf<DATA,3,PRECISION>& src
)
{
	src.PrepareForAccess();
	ImageOf<DATA,3> iT0  = DownsampleAlong(src,Point3D(1,0,0));
	ImageOf<DATA,3> iT1  = DownsampleAlong(iT0,Point3D(0,1,0));
	ImageOf<DATA,3> iOut = DownsampleAlong(iT1,Point3D(0,0,1));

	return iOut;
}






//============================================================================
//============================================================================
template <class DATA, int DIMENSIONALITY, class PRECISION>
ImageOf<DATA,DIMENSIONALITY>Downsample(
	int cTimes,
	SourceOf<DATA,DIMENSIONALITY,PRECISION>* psrcIn
)
{
	ClaimPointer(psrcIn);
	if (cTimes==0){
		ImageOf<DATA,DIMENSIONALITY> iOut;
		AllocatedCopy(iOut,*psrcIn);
		ReleasePointer(psrcIn);
		return iOut;
	}

	ImageOf<DATA,DIMENSIONALITY> iOut=Downsample(*psrcIn);
	ReleasePointer(psrcIn);
	while(cTimes>1){
		ImageOf<DATA,DIMENSIONALITY> iT=Downsample(iOut);
		iOut=iT;
		cTimes--;
	}

	return iOut;
}





#endif //#ifndef __LIBDOWNSAMPLE_H__

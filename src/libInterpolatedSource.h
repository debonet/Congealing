#ifndef __LIBITERPOLATEDSOURCE_H__
#define __LIBITERPOLATEDSOURCE_H__

#include "SourceAccessor.h"
#include "SourceAccessorLinearInterpolate.h"
#include "SourceAccessorCubicInterpolate.h"
#include "SourceAccessorNearestNeighbor.h"

template <class DATA, int DIMENSIONALITY>
inline SourceAccessorOf<DATA, DIMENSIONALITY, Real, SourceOf<DATA,DIMENSIONALITY,int> >*
NewInterpolatedSource (SourceOf<DATA,DIMENSIONALITY,int>* psrc, InterpolationType it=Cubic)
{
	typedef SourceOf<DATA,DIMENSIONALITY,int> SOURCE;
	SourceAccessorOf<DATA, DIMENSIONALITY, Real, SOURCE>* pinterp;

	switch(it){
	case Nearest:{
		pinterp=new SourceAccessorNearestNeighborOf<DATA, DIMENSIONALITY, Real, SOURCE>;
		break;
	}
	case Linear:{
		pinterp=new SourceAccessorLinearInterpolateOf<DATA, DIMENSIONALITY, Real, SOURCE>;
		break;
	}
	case Cubic:{
		pinterp=new SourceAccessorCubicInterpolateOf<DATA, DIMENSIONALITY, Real, SOURCE>;
		break;
	}
	default:{
		CONGEAL_ERROR("unknown interpolation type %d", it);
	}
	}

	pinterp->SetSource(psrc);
	HandoffPointer(pinterp);
	return pinterp;
}


#endif //#ifndef __LIBITERPOLATEDSOURCE_H__

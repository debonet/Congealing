#ifndef __LIBSOURCEOPERATIONS_H__
#define __LIBSOURCEOPERATIONS_H__

#include "Image.h"
#include "libRefCount.h"
#include "SourceTransform.h"

//============================================================================
//============================================================================
template <class SOURCE_IN, int DIMENSIONALITY, class DATA_OUT>
void CopyInto(
	SourceMemoryOf<DATA_OUT,DIMENSIONALITY>& srcOut, 
	const SOURCE_IN &srcIn,
	bool bPrepared=false
){
	if (!bPrepared){
		srcIn.PrepareForAccess();
	}
	DATA_OUT *vdata=srcOut.WriteData();
	typedef PointOf<DIMENSIONALITY,int> POINT;

	// get a 5% speedup by using a write pointer
	int n=0;
	forpoint(POINT,pt,0,srcOut.Size()){
		GetPoint(srcIn,vdata[n],pt);
		n++;
	}
}

//============================================================================
//============================================================================
template <class SOURCE_IN, int DIMENSIONALITY, class DATA_OUT>
void AllocatedCopy(
	SourceMemoryOf<DATA_OUT,DIMENSIONALITY>& srcOut, 
	const SOURCE_IN& srcIn
){
	srcIn.PrepareForAccess();
	srcOut.Allocate(srcIn.Size());
	CopyInto(srcOut,srcIn,false);
}

//============================================================================
//============================================================================
template <class SOURCE_IN, int DIMENSIONALITY, class DATA_OUT>
void AllocatedCopy(
	SourceMemoryOf<DATA_OUT,DIMENSIONALITY>& srcOut, 
	SOURCE_IN* psrcIn
){
	ClaimPointer(psrcIn);
	psrcIn->PrepareForAccess();
	srcOut.Allocate(psrcIn->Size());

	typedef PointOf<DIMENSIONALITY,int> POINT;

	forpoint(POINT,pt,0,srcOut.Size()){
		TypeOfData(SOURCE_IN) dataIn;
		DATA_OUT dataOut;

		GetPoint(*psrcIn,dataIn,pt);
		dataOut=dataIn;
		SetPoint(srcOut,pt,dataOut);
	}
	ReleasePointer(psrcIn);

/*
	// for some reason making a local copy is _WAY_ faster (~2x)
	ClaimPointer(psrcIn);
	SOURCE_IN srcInLocal=*psrcIn;
	srcInLocal.PrepareForAccess();
	srcOut.Allocate(srcInLocal.Size());
	CopyInto(srcOut,srcInLocal,false);
	ReleasePointer(psrcIn);

	// the slow way using a dereferenced pointer
	ClaimPointer(psrcIn);
	psrcIn->PrepareForAccess();
	srcOut.Allocate(psrcIn->Size());
	CopyInto(srcOut,*psrcIn,false);
	ReleasePointer(psrcIn);
*/
}





//============================================================================
template <class SOURCE_IN, class SOURCE_OUT>
void CopyInto(
	SOURCE_OUT *psrcOut,
	SOURCE_IN *psrcIn,
	bool bPrepared=false
){
	ClaimPointer(psrcIn);
	ClaimPointer(psrcOut);
	if (!bPrepared){
		psrcIn->PrepareForAccess();
		psrcOut->PrepareForAccess();
	}
	typedef typename SOURCE_OUT::type_point POINT;
	typedef typename SOURCE_IN::type_data DATA_IN;
	typedef typename SOURCE_OUT::type_data DATA_OUT;

	forpoint(POINT,pt,0,psrcOut->Size()){
		DATA_IN dataIn;
		DATA_OUT dataOut;

		GetPoint(*psrcIn,dataIn,pt);
		dataOut=dataIn;
		SetPoint(*psrcOut,pt,dataOut);
	}
	ReleasePointer(psrcIn);
	ReleasePointer(psrcOut);
}


#endif //__LIBSOURCEOPERATIONS_H__

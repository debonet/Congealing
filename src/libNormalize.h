#ifndef __LIBNORMALIZE_H__
#define __LIBNORMALIZE_H__

#include <typeinfo>

#include "SourceRegistry.h"
#include "libConfiguration.h"




//============================================================================
// Accessors
//   accessors have no parameters and therefore have no normalization 
//   or initial setps
//============================================================================
#define _NoNormalize(_name,_class,_templateparams...)													\
	template <class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>	\
	void NormalizeGroup(																												\
		int csource,																															\
		_class<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE> *vsrc											\
	)																																						\
	{}


#define _NoInitialSteps(_name,_class,_templateparams...)											\
	template<class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>	\
	void RegisterInitialSteps(																									\
		const _class<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE> &src,								\
		RegistryOfInitialSteps &regSteps																					\
	)																																						\
	{}

ALL_ACCESSORS_DO(_NoNormalize);
ALL_ACCESSORS_DO(_NoInitialSteps);




//============================================================================
// Rotate
//============================================================================
template <class DATA, class PRECISION, class SUBSOURCE>
void NormalizeGroup(
	int csource, 
	SourceTransformRotateOf<DATA,2, PRECISION,SUBSOURCE>* vsrc
) 
{
	Real rTAngle=0;
	for (int n=0; n<csource; n++){
		rTAngle += vsrc[n].GetAngle();
	}
	rTAngle/=csource;

	for (int n=0; n<csource; n++){
		vsrc[n].SetAngle(
			vsrc[n].GetAngle()-rTAngle
		);
	}
}


template<class DATA, class PRECISION, class SUBSOURCE>
void RegisterInitialSteps(
	const SourceTransformRotateOf<DATA,2,PRECISION,SUBSOURCE> &src,
	RegistryOfInitialSteps &regSteps
)
{
	static Real rStepDefault=ConfigValue("congeal.initialsteps.rotate",1.);
	regSteps.Register(rStepDefault);
}



template <class DATA, class PRECISION, class SUBSOURCE>
void NormalizeGroup(
	int csource, 
	SourceTransformRotateOf<DATA,3, PRECISION,SUBSOURCE>* vsrc
) 
{
	Real rTAngleX=0;
	Real rTAngleY=0;
	Real rTAngleZ=0;
	for (int n=0; n<csource; n++){
		vsrc[n].SetAngle(
			::Bound(vsrc[n].GetAngleX(),-30.,30.),
			::Bound(vsrc[n].GetAngleY(),-30.,30.),
			::Bound(vsrc[n].GetAngleZ(),-30.,30.)
		);
	}

	for (int n=0; n<csource; n++){
		rTAngleX += vsrc[n].GetAngleX();
		rTAngleY += vsrc[n].GetAngleY();
		rTAngleZ += vsrc[n].GetAngleZ();
	}
	rTAngleX/=csource;
	rTAngleY/=csource;
	rTAngleZ/=csource;

	for (int n=0; n<csource; n++){
		vsrc[n].SetAngle(	
			vsrc[n].GetAngleX()-rTAngleX,
			vsrc[n].GetAngleY()-rTAngleY,
			vsrc[n].GetAngleZ()-rTAngleZ
		);
	}
}

template<class DATA, class PRECISION, class SUBSOURCE>
void RegisterInitialSteps(
	const SourceTransformRotateOf<DATA,3,PRECISION,SUBSOURCE> &src,
	RegistryOfInitialSteps &regSteps
)
{
	static Real rStepDefault=ConfigValue("congeal.initialsteps.rotate",1.);
	regSteps.Register(rStepDefault);
	regSteps.Register(rStepDefault);
	regSteps.Register(rStepDefault);
}




//============================================================================
// Scale
//============================================================================
template <class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void NormalizeGroup(
	int csource, 
	SourceTransformScaleOf<DATA,DIMENSIONALITY,PRECISION, SUBSOURCE>* vsrc
) 
{
	typedef PointOf<DIMENSIONALITY,Real> POINT;
	POINT ptScale(0.);

	for (int n=0; n<csource; n++){
		vsrc[n].SetScale(vsrc[n].Scale().Bound(POINT(.75),POINT(1.25)));
	}

	for (int n=0; n<csource; n++){
		ptScale = ptScale + vsrc[n].Scale();
	}
	ptScale=ptScale/csource;

	for (int n=0; n<csource; n++){
		vsrc[n].SetScale(
			vsrc[n].Scale() / ptScale
		);
	}
}


template <class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void RegisterInitialSteps(
	const SourceTransformScaleOf<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE> &src,
	RegistryOfInitialSteps &regSteps
)
{
	static Real rStepDefault=ConfigValue("congeal.initialsteps.scale",.01);
	for (int n=0; n<DIMENSIONALITY; n++){
		regSteps.Register(rStepDefault);
	}
}



//============================================================================
//============================================================================
template <class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void NormalizeGroup(
	int csource, 
	SourceTransformTranslateOf<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE>* vsrc
) 
{
	typedef PointOf<DIMENSIONALITY, Real> POINT;

	POINT pt=POINT(0.);

	for (int n=0; n<csource; n++){
		vsrc[n].Translate()=vsrc[n].Translate().Bound(POINT(-.1),POINT(+.1));
	}

	for (int n=0; n<csource; n++){
		pt = pt + vsrc[n].Translate();
	}
	pt=pt/csource;

	for (int n=0; n<csource; n++){
		vsrc[n].Translate() -= pt;
	}
}


template<class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void RegisterInitialSteps(
	const SourceTransformTranslateOf<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE> &src,
	RegistryOfInitialSteps &regSteps
)
{
	static Real rStepDefault=ConfigValue("congeal.initialsteps.translate",.01);
	for (int n=0; n<DIMENSIONALITY; n++){
		regSteps.Register(rStepDefault);
	}
}



//============================================================================
// Warp
//============================================================================
template <class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void NormalizeGroup(
	int csource, 
	SourceTransformWarpOf<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE>* vsrc
) 
{
	typedef PointOf<DIMENSIONALITY, Real> WARPCP;
	typedef PointOf<DIMENSIONALITY, int> POINT;

/*

	WARPCP dptMax=WARPCP(.5)/vsrc[0].GetPWarpControlPoints()->Size();
	WARPCP dptMin=-1*dptMax;

	
	forpoint(POINT,pt,0,vsrc[0].GetPWarpControlPoints()->Size()){
		for (int n=0; n<csource; n++){
			WARPCP dpt;
			GetPoint(*vsrc[n].GetPWarpControlPoints(),dpt,pt);
			dpt=dpt.Bound(dptMin,dptMax);
			SetPoint(*vsrc[n].GetPWarpControlPoints(),pt,dpt);
		}
	}

	forpoint(POINT,pt,0,vsrc[0].GetPWarpControlPoints()->Size()){
		WARPCP dptT(0.);
		for (int n=0; n<csource; n++){
			WARPCP dpt;
			GetPoint(*vsrc[n].GetPWarpControlPoints(),dpt,pt);
			dptT+=dpt;
		}
		dptT/=csource;

		for (int n=0; n<csource; n++){
			WARPCP dpt;
			GetPoint(*vsrc[n].GetPWarpControlPoints(),dpt,pt);
			SetPoint(*vsrc[n].GetPWarpControlPoints(),pt,dpt-dptT);
		}
	}

*/
}

template<class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void RegisterInitialSteps(
	const SourceTransformWarpOf<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE> &src,
	RegistryOfInitialSteps &regSteps
)
{
	typedef PointOf<DIMENSIONALITY, Real> WARPCP;
	typedef PointOf<DIMENSIONALITY, int> POINT;

	static Real rStepDefault=ConfigValue("congeal.initialsteps.warp",.5);

	WARPCP dpt=WARPCP(rStepDefault)/src.GetPWarpControlPoints()->Size();
	forpoint(POINT,pt,0,src.GetPWarpControlPoints()->Size()){
		regSteps.Register(dpt);
	}
}


//============================================================================
//============================================================================
template <class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void NormalizeGroup(
	int csource, 
	SourceTransformDataScaleOf<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE>* vsrc
) 
{
	typedef PointOf<DIMENSIONALITY, Real> POINT;

	POINT pt=POINT(0.);

	for (int n=0; n<csource; n++){
		vsrc[n].SetSlopeIntercept(
			::Bound(vsrc[n].Slope(),.2,5.),
			::Bound(vsrc[n].Intercept(),-.5,+.5)
		);
	}

	Parameter rSlopeT=0;
	Parameter rInterceptT=0;
	for (int n=0; n<csource; n++){
		rSlopeT += vsrc[n].Slope();
		rInterceptT += vsrc[n].Intercept();
	}
	rSlopeT /= csource;
	rInterceptT /= csource;

	for (int n=0; n<csource; n++){
		vsrc[n].SetSlopeIntercept(
			vsrc[n].Slope() / rSlopeT,
			vsrc[n].Intercept() - rInterceptT
		);
	}
}


template<class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void RegisterInitialSteps(
	const SourceTransformDataScaleOf<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE> &src,
	RegistryOfInitialSteps &regSteps
)
{
	static Real rStepDefaultSlope=ConfigValue("congeal.initialsteps.datascale.slope",.01);
	static Real rStepDefaultIntercept=ConfigValue("congeal.initialsteps.datascale.intercept",.01);
	regSteps.Register(rStepDefaultSlope);
	regSteps.Register(rStepDefaultIntercept);
}


//============================================================================
//============================================================================
template <class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void NormalizeGroup(
	int csource, 
	SourceTransformSpatialScaleOf<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE>* vsrc
) 
{
	typedef PointOf<DIMENSIONALITY, int> POINT;

	POINT pt=POINT(0.);

	forpoint (POINT,pt,0,vsrc[0].GetPCoefficients()->Size()){
		Parameter paramT=0;
		for (int n=0; n<csource; n++){
			Parameter param;
			SourceMemoryOf<Real,DIMENSIONALITY>* pcoeff =vsrc[n].GetPCoefficients();
			GetPoint(*pcoeff,param,pt);
			param=::Bound(param,-.5,.5);
			SetPoint(*pcoeff,pt,param);

			paramT+=param;
		}

		Parameter paramAvg=paramT/=csource;

		for (int n=0; n<csource; n++){
			Parameter param;
			SourceMemoryOf<Real,DIMENSIONALITY>* pcoeff =vsrc[n].GetPCoefficients();
			GetPoint(*pcoeff,param,pt);
			param-=paramAvg;
			SetPoint(*pcoeff,pt,param);
		}
	}
}


template<class DATA, int DIMENSIONALITY, class PRECISION, class SUBSOURCE>
void RegisterInitialSteps(
	const SourceTransformSpatialScaleOf<DATA,DIMENSIONALITY,PRECISION,SUBSOURCE> &src,
	RegistryOfInitialSteps &regSteps
)
{
	int c=src.GetPCoefficients()->Size().CVolume();
	static Parameter paramStepDefault=ConfigValue("congeal.initialsteps.spatialscale",.01);
	for (int n=0; n<c; n++){
		regSteps.Register(paramStepDefault);
	}
}


#endif //#ifndef __LIBNORMALIZE_H__

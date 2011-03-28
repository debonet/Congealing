// $Id$

#ifndef __SOURCETRANSFORMSPATIALSCALE_H__
#define __SOURCETRANSFORMSPATIALSCALE_H__

#include "SourceGenerics.h"
#include "SourceMemory.h"
#include "SourceTransform.h"
#include <math.h>
#include "libInterpolatedSource.h"


#undef DEBUG
#define DEBUG 0
#include "libDebug.h"


//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformSpatialScaleBaseOf
	: public SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	typedef PixelRealGrey COEFFICIENT; //coef
	typedef PointOf<DIMENSIONALITY, int> COEFFICIENTPOINT; //coefpt
	typedef SourceMemoryOf<COEFFICIENT, DIMENSIONALITY> COEFFICIENTS; //coef

	mutable COEFFICIENTS* m_pcoefs;

public:
	SourceTransformSpatialScaleBaseOf()
		: SourceTransformOf<DATA, DIMENSIONALITY,PRECISION, SOURCE> ()
		, m_pcoefs(NULL)
	{
	}

	virtual ~SourceTransformSpatialScaleBaseOf()
	{
		if (Counted::AboutToDie()){
			ReleasePointer(m_pcoefs);
		}
	}

	void SetCoefficients(COEFFICIENTS *pcoefs)
	{
		ReleasePointer(m_pcoefs);
		m_pcoefs=ClaimPointer(pcoefs);
	}

	void AllocateCoefficients(const COEFFICIENTPOINT &ptSize)
	{
		ReleasePointer(m_pcoefs);

		m_pcoefs=new COEFFICIENTS();
		m_pcoefs->Allocate(ptSize);
	}

	void AllocateCofficients(int c)
	{
		COEFFICIENTPOINT pt(c);
		AllocateCofficients(pt);
	}

	COEFFICIENTS* GetPCoefficients() const
	{
		return m_pcoefs;
	}


	virtual String Describe() const
	{
		return (
			_LINE + "SourceTransformSpatialScale" + LINE_ + 
			(_INDENT + "with coeffients: " + LINE_)  +
			(_INDENT2 + m_pcoefs->Describe() ) + 
			this->DescribeCommon() 
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		m_pcoefs->RegisterDataAsParameters(reg);
	}

	void PrepareForAccessAction() const
	{
		ASSERTf(m_pcoefs,"Null Cofficients");
		// prepare the coefficients for access
		m_pcoefs->PrepareForAccess();

		// and finally set our own size;
		this->m_ptSize=this->PSource()->Size();
	}


	static String SerializationId()
	{
		return String("SourceTransformSpatialScale(") + DIMENSIONALITY +"D)";
	}

	void SerializeSelf(Stream& st) const
	{ 
		::SerializePointer(st,m_pcoefs);
	}

	void DeserializeSelf(Stream& st)
	{
		::DeserializePointer(st,m_pcoefs);
	}
};





//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformSpatialScaleOf
	: public SourceTransformSpatialScaleBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};

//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformSpatialScaleOf<DATA,2,PRECISION,SOURCE> 
	: public SourceTransformSpatialScaleBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

private:
	typedef PixelRealGrey COEFFICIENT; //coef
	typedef PointOf<DIMENSIONALITY, int> COEFFICIENTPOINT; //coefpt
	typedef SourceMemoryOf<COEFFICIENT, DIMENSIONALITY>* COEFFICIENTS; //coef

public:
	void Get(DATA& dataOut, const PRECISION& rX, const PRECISION& rY) const
	{
		::Get(
			*(this->m_psource),
			dataOut, 
			rX,
			rY
		);

		int cX=this->Size().X();
		int cY=this->Size().Y();

		Real rrX=Real(rX);
		Real rrY=Real(rY);

		int cXCoefs=this->m_pcoefs->Size().X();
		int cYCoefs=this->m_pcoefs->Size().Y();

		RANGEPROMOTION(DATA) dataT = dataOut;

		dataT -= MidValue(DATA);
		for (int nXCoef=0; nXCoef<cXCoefs; nXCoef++){
			for (int nYCoef=0; nYCoef<cYCoefs; nYCoef++){
				COEFFICIENT coeff;
				this->m_pcoefs->Get(coeff,nXCoef,nYCoef);


				Real rCosX=cos(rrX/cX * PI * nXCoef);
				Real rCosY=cos(rrY/cY * PI * nYCoef);

				Real rWeight = (rCosX * rCosY) * coeff; 

				D("%d %d %d %g==%g", cXCoefs, cYCoefs, coeff,rWeight);

				dataT += (rWeight * Range(DATA));
			}
		}

		dataT += MidValue(DATA);
		dataOut = ::Bound(dataT,MinValue(DATA),MaxValue(DATA));
	}

	void Set(const PRECISION& rX, const PRECISION& rY, const DATA& data)
	{
		ERROR("not implemented");
	}
};

//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformSpatialScaleOf<DATA, 3, PRECISION, SOURCE>
	: public SourceTransformSpatialScaleBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	typedef PixelRealGrey COEFFICIENT; //coef
	typedef PointOf<DIMENSIONALITY, int> COEFFICIENTPOINT; //coefpt
	typedef SourceMemoryOf<COEFFICIENT, DIMENSIONALITY>* COEFFICIENTS; //coef

public:
	void Get(DATA& dataOut, const PRECISION& rX, const PRECISION& rY, const PRECISION& rZ) const
	{
		::Get(
			*(this->m_psource),
			dataOut, 
			rX,
			rY,
			rZ
		);

		PRECISION cX=this->Size().X();
		PRECISION cY=this->Size().Y();
		PRECISION cZ=this->Size().Z();

		// explicity ensure reals for compuation below
		Real rrX=Real(rX);
		Real rrY=Real(rY);
		Real rrZ=Real(rZ);

		int cXCoefs=this->m_pcoefs->Size().X();
		int cYCoefs=this->m_pcoefs->Size().Y();
		int cZCoefs=this->m_pcoefs->Size().Z();

//		RANGEPROMOTION(DATA) dataT = dataOut;

		Real rWeightT = 0.;

//		dataT -= MidValue(DATA);
		for (int nXCoef=0; nXCoef<cXCoefs; nXCoef++){
			for (int nYCoef=0; nYCoef<cYCoefs; nYCoef++){
				for (int nZCoef=0; nZCoef<cZCoefs; nZCoef++){
					COEFFICIENT coeff;
					this->m_pcoefs->Get(coeff,nXCoef,nYCoef,nZCoef);

					Real dXPhase = (nXCoef%2)?.5:0;
					Real dYPhase = (nYCoef%2)?.5:0;
					Real dZPhase = (nZCoef%2)?.5:0;
					
					Real rCosX=cos((rrX/cX * nXCoef+dXPhase) * PI);
					Real rCosY=cos((rrY/cY * nYCoef+dYPhase) * PI);
					Real rCosZ=cos((rrZ/cZ * nZCoef+dZPhase) * PI);

					Real rWeight = (rCosX * rCosY * rCosZ) * coeff; 

					rWeightT += rWeight;
				}
			}
		}

//		dataT += MidValue(DATA);
		
		RANGEPROMOTION(DATA) dataT = dataOut;
		dataT *= exp(rWeightT);
		dataOut = ::Bound(dataT,RANGEPROMOTION(DATA)(MinValue(DATA)),RANGEPROMOTION(DATA)(MaxValue(DATA)));
	}

	void Set(const PRECISION& rX, const PRECISION& rY, const PRECISION& rZ, const DATA& data)
	{
		ERROR("not implemented");
	}
};



//============================================================================
// assistant
//============================================================================
MAKE_ASSISTANT(
	SpatialScale, 
	SourceTransformSpatialScaleOf,
	{ 
		psrcOut->SetCoefficients(pcoeff);
	},
	SourceMemoryOf<PixelRealGrey, TypeOfDimensionality(SOURCE)>* pcoeff,
);



//============================================================================
// HACK: convert va_args to a table for convienence testing
// TODO: can this be generalized across types? (i.e. can Real ==> template<class PIXEL>?)
//============================================================================
template<int DIMENSIONALITY>
SourceMemoryOf<Real, DIMENSIONALITY>* CoefficientTable(
	const PointOf<DIMENSIONALITY, int> &ptSize, 
	...
)
{
	SourceMemoryOf<Real, DIMENSIONALITY>* pi=new SourceMemoryOf<Real, DIMENSIONALITY>();
	pi->Allocate(ptSize);

	int c=ptSize.CVolume();
		
	va_list listArgs;

	va_start(listArgs, ptSize);
	for (int n=0; n<c; n++){
		Real pxl=va_arg(listArgs,Real);
		pi->SetDatum(n,pxl);
	}
	va_end(listArgs);

	return HandoffPointer(pi);
}



#endif // __SOURCETRANSFORMSPATIALSCALE_H__

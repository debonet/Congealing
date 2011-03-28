#ifndef __CONGEALGROUP_H__
#define __CONGEALGROUP_H__

#include "libOptimize_NelderMead.h"
#include "libOptimize_GreedySwarm.h"
#include "libOptimize_RandomWalk.h"
#include "libErrorFunctions.h"
#include <string.h> //for memcpy()
#include "Pixels.h"
#include "libBaseTypes.h"
#include "Registry.h"


#include "Recipie.h"

#undef DEBUG
#define DEBUG 0
#include "libDebug.h"

//============================================================================
//=================================================p===========================
template<class RECIPIE>
class CongealGroupOf
{
	typedef CongealGroupOf<RECIPIE> THIS_TYPE;
	static const int DIMENSIONALITY=TypeOfDimensionality(RECIPIE);
	typedef TypeOfPrecision(RECIPIE) PRECISION;
	typedef TypeOfData(RECIPIE) DATA;

	typedef PointOf<DIMENSIONALITY,PRECISION> POINT;

	int m_csource;
	RECIPIE* m_precipie;

	RegistryOfParameters m_regParams;
	RegistryOfInitialSteps m_regSteps;
 
	Real(*m_fxnSourceComparisonMetric)(int c, const RECIPIE *precipie);
	Real(*m_fxnDataComparisonMetric)(int c, const DATA *vData);

	int m_cStatisticalMeasurements;

public:
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	CongealGroupOf(RECIPIE* precipie)
	{
//		m_fxnDataComparisonMetric=Real(*VarianceOfSet)(int,DATA*);
		m_precipie=precipie;
		ClaimPointer(m_precipie);
		m_fxnDataComparisonMetric=&VarianceOfSet<DATA>;
	}

	~CongealGroupOf()
	{
		ReleasePointer(m_precipie);
	}


	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void SetsourceComparisonMetric	( Real(*fxn)(int c, RECIPIE *precipie) )
	{
		m_fxnSourceComparisonMetric=fxn;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void SetDataComparisonMetric	( Real(*fxn)(int c, DATA *vData) )
	{
		m_fxnDataComparisonMetric=fxn;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	static Real StatisticalAlignmentErrorOfGroup(
		Parameter* vparam, const Real& rErrMin, const void* pgroup
	)
	{
		return ((CongealGroupOf<RECIPIE>*)pgroup)->StatisticalAlignmentError(vparam, rErrMin);
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	int CSources() const 
	{
		return m_precipie->CSources();
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	const POINT& Size() const 
	{
		return m_precipie->Size();
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	PRECISION CSize() const 
	{
		return m_precipie->CSize();
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	Real StatisticalAlignmentError(Parameter* vparam, const Real& rErrMin)
	{
		// push the parameters back into the source group
		ParameterDistribute(vparam);

		// normalize them
		m_precipie->Normalize();

		// recollect normalized parameters
		ParameterCollect(vparam);

		// get ready to access
		m_precipie->PrepareForAccess();

		DATA* vdata=new DATA[CSources()];
		Real rErrT=0;
		POINT pt;

		for (int n=0; n<m_cStatisticalMeasurements; n++){
			pt=NewRandomPoint(Size());
			m_precipie->GetPoint(vdata,pt);

			// measure error
			Real rErr=VarianceOfSet(CSources(),vdata);

			rErrT+=rErr;
		}
		rErrT /= m_cStatisticalMeasurements;

		return rErrT;
	}


	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void ParameterCollect(Parameter* vparam)
	{
		int c=m_regParams.C();
		for (int n=0; n<c; n++){
			vparam[n]=*(m_regParams.Get(n));
		}
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void ParameterDistribute(Parameter* vparam)
	{
		int c=m_regParams.C();
		for (int n=0; n<c; n++){
			*(m_regParams.Get(n))=vparam[n];
		}
	}


	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void RegisterParameters()
	{
		m_precipie->RegisterParameters(m_regParams,	m_regSteps);
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void StatisticalCongeal(
//		Real rSamplingPercentage=.1,
		int  cSamples,
		Real rThreshold=.001, 
		int cIterations=1000
	){
		m_regParams.Clear();
		m_regSteps.Clear();

		m_precipie->PrepareForAccess();
		m_precipie->RegisterParameters(m_regParams,	m_regSteps);

		ASSERTf(
			m_regParams.C() == m_regSteps.C(),
			"Must have matching number of parameters and initial scales %d != %d", 
			m_regParams.C(), m_regSteps.C()
		);

		int cParams = m_regParams.C();

//		m_cStatisticalMeasurements = int(CSize() * rSamplingPercentage);
		m_cStatisticalMeasurements = cSamples;

		Parameter* vparam = new Parameter[cParams];
		ParameterCollect(vparam);

		UD("OPTIMIZING %d PARAMETERS", cParams);
		UD("WITH %d MEASUREMENTS (%2.4g%%)", m_cStatisticalMeasurements, Real(100*m_cStatisticalMeasurements)/CSize());

		// NelderMeadOptimize
		// OptimizeGreedySwarm
		Optimize_RandomWalk
		(
			cParams,vparam,m_regSteps.V(),
			StatisticalAlignmentErrorOfGroup,this,
			rThreshold,cIterations
		);

		ParameterDistribute(vparam);

		delete [] vparam;
	}





	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void StatisticalPhasedCongeal(
//		Real rSamplingPercentage=.1,
		int  cSamples,
		Real rThreshold=.001, 
		int cIterations=1000
	){
		m_regParams.Clear();
		m_regSteps.Clear();

		m_precipie->PrepareForAccess();
		m_precipie->RegisterParameters(m_regParams);
		m_precipie->RegisterInitialSteps(m_regSteps);

		ASSERTf(
			m_regParams.C() == m_regSteps.C(),
			"Must have matching number of parameters and initial scales %d != %d", 
			m_regParams.C(), m_regSteps.C()
		);

		int cParams = m_regParams.C();

//		m_cStatisticalMeasurements = int(CSize() * rSamplingPercentage);
		m_cStatisticalMeasurements = cSamples;

		Parameter* vparam = new Parameter[cParams];
		ParameterCollect(vparam);

		UD("OPTIMIZING %d PARAMETERS", cParams);
		UD("WITH %d MEASUREMENTS (%2.4g%%)", m_cStatisticalMeasurements, Real(100*m_cStatisticalMeasurements)/CSize());

		 NelderMeadOptimize
		// OptimizeGreedySwarm
		//Optimize_RandomWalk
		(
			cParams,vparam,m_regSteps.V(),
			StatisticalAlignmentErrorOfGroup,this,
			rThreshold,cIterations
		);

		ParameterDistribute(vparam);

		delete [] vparam;
	}
};







typedef CongealGroupOf<Source_G2I>  CongealGroup_G2I;
typedef CongealGroupOf<Source_G2R>  CongealGroup_G2R;
typedef CongealGroupOf<Source_G3I>  CongealGroup_G3I;
typedef CongealGroupOf<Source_G3R>  CongealGroup_G3R;

#endif //__CONGEALGROUP_H__

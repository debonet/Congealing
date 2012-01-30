#ifndef __CONGEALGROUPPHASED_H__
#define __CONGEALGROUPPHASED_H__

#include "libOptimize_NelderMead.h"
#include "libOptimize_GreedySwarm.h"
#include "libOptimize_RandomWalk.h"
#include "libErrorFunctions.h"
#include <string.h> //for memcpy()
#include "Pixels.h"
#include "libBaseTypes.h"
#include "Registry.h"
#include "libConfiguration.h"
#include "Recipie.h"

#undef DEBUG
#define DEBUG 0
#include "libDebug.h"

#define MULTITHREADED 1

#if MULTITHREADED
#  include <pthread.h>
#  include "libBarrier.h"
#  include <unistd.h>
#endif

#ifndef FEATURE_LBFGS
#  define FEATURE_LBFGS 0
#endif

#define LBFGS 0
#define GRADIENTDESCENT 2
#define NELDERMEAD 3

#if FEATURE_LBFGS
#  include <lbfgs.h>
#endif

class NVar{
public:
	int m_n;
	Real m_rVar;
	static int SortByVar(const void* pnvar1 ,const void *pnvar2)  {
		const NVar &nvar1 = *((NVar*)pnvar1);
		const NVar &nvar2 = *((NVar*)pnvar2);

		if (nvar1.m_rVar < nvar2.m_rVar){
			return -1;
		}
		return 1;
	}
};


//============================================================================
//=================================================p===========================
template<class RECIPIE>
class CongealGroupPhasedOf
{
	static long int m_cLookups;

	typedef CongealGroupPhasedOf<RECIPIE> THIS_TYPE;
	static const int DIMENSIONALITY=RECIPIE::type_dimensionality;
	typedef typename RECIPIE::type_precision PRECISION;
	typedef typename RECIPIE::type_data DATA;
	typedef PointOf<DIMENSIONALITY,PRECISION> POINT;

	RECIPIE* m_precipie;

	RegistryOfParameters* m_vregParams;
	RegistryOfInitialSteps m_regSteps;
	int m_cSamples;
	int m_cSources;
	RANGEPROMOTION(DATA)* m_vdataSum;
	RANGEPROMOTION(DATA)* m_vdataSumSq;
	Real m_rStep;

	DATA* m_mdata;
	POINT* m_vpt;

	Parameter* m_vparam;

#if MULTITHREADED
	bool m_bDie;
	pthread_t* m_vthread;
	pthread_mutex_t m_mutexReady;
	pthread_cond_t m_condReady;
	pthread_barrier_t m_barrierDone;
#endif

	struct AlignInfo // ai
	{
		bool m_bReady;
		int m_nSource; 
		Real m_rErr;
		Real* m_vparam;
		CongealGroupPhasedOf<RECIPIE>* m_pcongeal;
	};

	AlignInfo* m_vai;

 
public:
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	CongealGroupPhasedOf(RECIPIE* precipie)
	{
		m_precipie=precipie;
		ClaimPointer(m_precipie);

		m_cSources=m_precipie->CSources();
		m_vregParams = new RegistryOfParameters[m_cSources];
		
		m_cSamples=-1;
		m_mdata=NULL;
		m_vpt=NULL;
		m_vdataSum=NULL;
		m_vdataSumSq=NULL;

		// create multiple AlignInfo's so we can eventually launch 
		// threads for each source
		m_vai=new AlignInfo[m_cSources];
		for (int nSource=0; nSource<m_cSources; nSource++){
			m_vai[nSource].m_bReady=false;
			m_vai[nSource].m_nSource=nSource;
			m_vai[nSource].m_rErr=0;
			m_vai[nSource].m_pcongeal=this;
			m_vai[nSource].m_vparam=NULL;
		}

#if MULTITHREADED
		// set up multithreading sync devices
		pthread_mutex_init(&m_mutexReady,NULL);
		pthread_cond_init(&m_condReady,NULL);
		pthread_barrier_init(&m_barrierDone,NULL,m_cSources+1);

		m_bDie=false;
		// launch threads
		m_vthread=new pthread_t[m_cSources];
		for (int nSource=0; nSource<m_cSources; nSource++){
			pthread_create(
				&(m_vthread[nSource]),
				NULL,
				OptimizeSource,
				&m_vai[nSource]
			);
		}
#endif
	}



	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	~CongealGroupPhasedOf(){
#if MULTITHREADED
		pthread_mutex_lock(&m_mutexReady); 
		m_bDie=true;
		pthread_cond_broadcast(&m_condReady);
		pthread_mutex_unlock(&m_mutexReady); 
		pthread_barrier_wait(&m_barrierDone);

		pthread_mutex_destroy(&m_mutexReady);
		pthread_cond_destroy(&m_condReady);
		pthread_barrier_destroy(&m_barrierDone);
		for (int nSource=0; nSource<m_cSources; nSource++){
			pthread_detach(m_vthread[nSource]);
		}
		delete [] m_vthread;
#endif

		delete [] m_vregParams;

		for (int nSource=0; nSource<m_cSources; nSource++){
			SafeDeleteArray(m_vai[nSource].m_vparam);
		}
		delete [] m_vai;

		SafeDeleteArray(m_mdata);
		SafeDeleteArray(m_vpt);
		SafeDeleteArray(m_vdataSum);
		SafeDeleteArray(m_vdataSumSq);

		ReleasePointer(m_precipie);
	}


private: 
	void AllocateSampleSpace(int cSamples){
		if (m_cSamples!=cSamples){
			m_cSamples=cSamples;

			SafeDeleteArray(m_mdata);
			SafeDeleteArray(m_vpt);
			SafeDeleteArray(m_vdataSum);
			SafeDeleteArray(m_vdataSumSq);

			m_mdata=new DATA[m_cSamples*m_cSources];
			m_vpt=new POINT[m_cSamples];
			m_vdataSum=new RANGEPROMOTION(DATA)[m_cSamples];
			m_vdataSumSq=new RANGEPROMOTION(DATA)[m_cSamples];
		}
	}


	void SetUpRegistries(){
		// register parameters and steps
		// NOTE: this assumes all recipies have the same number, 
		// same ordering and same types of of parameters
		m_regSteps.Clear();
		m_precipie->RegisterInitialSteps(0,m_regSteps);
		for (int nSource=0; nSource<m_cSources; nSource++){
			m_vregParams[nSource].Clear();
			m_precipie->RegisterParameters(nSource,m_vregParams[nSource]);
			CONGEAL_ASSERTf(m_vregParams[nSource].C() == m_regSteps.C(),"%d != %d",m_vregParams[nSource].C(), m_regSteps.C());
		}

		// allocate scratch transfer space for using generalize optimization
		// methods which require a packed array of parameters
		int cSources=m_cSources;
		for (int nSource=0; nSource<cSources; nSource++){
			SafeDeleteArray(m_vai[nSource].m_vparam);
			m_vai[nSource].m_vparam = new Parameter[m_regSteps.C()];
		}
	}

public:
	
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	static int SortPoints(const void* ppt1 ,const void *ppt2)  {
		const POINT &pt1 = *((POINT*)ppt1);
		const POINT &pt2 = *((POINT*)ppt2);

		if (pt1.Z()<pt2.Z()){
			return -1;
		}
		if (pt1.Z()>pt2.Z()){
			return 1;
		}

		if (pt1.Y()<pt2.Y()){
			return -1;
		}
		if (pt1.Y()>pt2.Y()){
			return 1;
		}

		if (pt1.X()<pt2.X()){
			return -1;
		}
		if (pt1.X()>pt2.X()){
			return 1;
		}

		return 0;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void StatisticalPhasedCongeal(
		int  cSamples,
		Real rThreshold=.001, 
		int cIterations=1000,
		int nFirstIteration=0,
		int nLastIteration=-1
	){
		if (nLastIteration<nFirstIteration){
			nLastIteration=cIterations-1;
		}

		AllocateSampleSpace(cSamples);
		SetUpRegistries();

		m_precipie->PrepareForAccess();
		P("Optimizing %d parameters per volume"
			"with %d measurements (%2.4g%% converage)",
			m_regSteps.C(),
			cSamples, 
			Real(100*cSamples)/m_precipie->CSize()
		);

		Real rErr=REAL_MAX;
		for (int nIteration=nFirstIteration; nIteration<=nLastIteration && rErr > rThreshold; nIteration++){
			m_cLookups=0;

			m_rStep=(Real(cIterations-nIteration)/cIterations);

			if (1||nIteration==0){
				// compute a random collection of samples, retrying if the point is not within a good pixel stack
				m_precipie->PrepareForAccess();
				for (int nSample=0; nSample<cSamples; nSample++){
					bool bGoodPoint;

					int cTries=0;
					do{
						bGoodPoint=true;
						m_vpt[nSample]=NewRandomPoint(m_precipie->Size());
						DATA *vdataGroup=&m_mdata[nSample*m_cSources];
						for (int nSource=0; nSource<m_cSources; nSource++){
							m_precipie->PSource(nSource)->GetPoint(
								vdataGroup[nSource],m_vpt[nSample]
							);
							cTries++;
							m_cLookups++;
							if (vdataGroup[nSource]==0){
								bGoodPoint=false;
								break;
							}
						}
					}
					while(!bGoodPoint);
				}

				// set up group statistics
				for (int nSample=0; nSample<cSamples; nSample++){
					DATA *vdataGroup=&m_mdata[nSample*m_cSources];
					m_vdataSum[nSample]=0;
					m_vdataSumSq[nSample]=0;
					for (int nSource=0; nSource<m_cSources; nSource++){
						m_vdataSum[nSample] +=  vdataGroup[nSource];
						m_vdataSumSq[nSample] += sqr(vdataGroup[nSource]);
					}
				}

				NVar* vnvar=new NVar[cSamples];
				// compute most unique points
	 			for (int nSample=0; nSample<cSamples; nSample++){
					vnvar[nSample].m_n=nSample;
					vnvar[nSample].m_rVar=m_vdataSumSq[nSample]/cSamples - sqr(m_vdataSum[nSample]/cSamples);
				}
				qsort(vnvar,cSamples,sizeof(NVar),NVar::SortByVar);

				int cPoints=ConfigValue("congeal.optimize.bestpoints",100);
				POINT* vpt=new POINT[cPoints];
	 			for (int nPoint=0; nPoint<cPoints; nPoint++){
					vpt[nPoint]=m_vpt[vnvar[nPoint].m_n];
				}

				cSamples=cPoints;

				AllocateSampleSpace(cSamples);
				SetUpRegistries();

				m_precipie->PrepareForAccess();

	 			for (int nPoint=0; nPoint<cPoints; nPoint++){
					m_vpt[nPoint]=vpt[nPoint];
				}
				delete [] vpt;
				delete [] vnvar;

				qsort(m_vpt,cSamples,sizeof(POINT),CongealGroupPhasedOf<RECIPIE>::SortPoints);

				for (int nSample=0; nSample<cSamples; nSample++){
					DATA *vdataGroup=&m_mdata[nSample*m_cSources];
					for (int nSource=0; nSource<m_cSources; nSource++){
						m_precipie->PSource(nSource)->GetPoint(
							vdataGroup[nSource],m_vpt[nSample]
						);
						m_cLookups++;
					}
				}
			}

			// optimize each source against the collection
			TICK(dtmOptimize);
#if MULTITHREADED
			pthread_mutex_lock(&m_mutexReady); 
			for (int nSource=0; nSource<m_cSources; nSource++){
				m_vai[nSource].m_bReady=true;
			}
			pthread_cond_broadcast(&m_condReady);
			pthread_mutex_unlock(&m_mutexReady); 

			pthread_barrier_wait(&m_barrierDone);
#else 
			// sequentially
			for (int nSource=0; nSource<m_cSources; nSource++){
				D("optimizing source %d", nSource);
				OptimizeSource(&m_vai[nSource]);
			}
#endif

			// output
			P(
				"Iteration %d error: %g stepsize: %g"
				" total time: %g lookups: %ld lookups/s:%g", 
				nIteration,
				rErr,
				m_rStep,
				TOCK(dtmOptimize), 
				m_cLookups, 
				(Real)m_cLookups/TOCK(dtmOptimize)
			);


			// Normalize the data
			m_precipie->Normalize();

			rErr=0;
			for (int nSource=0; nSource<m_cSources; nSource++){
				rErr+=m_vai[nSource].m_rErr;
			}
			rErr/=m_cSources;
		}
	}



	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	static void* OptimizeSource(void*p){
		AlignInfo* pai = (AlignInfo*)p;
		pai->m_pcongeal->OptimizeSource(pai);
		return NULL;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void OptimizeSource(AlignInfo *pai){
		int nSource=pai->m_nSource;
		String sAlgorithm = ConfigValue("congeal.optimize.algorithm","gradientdescent");
		String sError = ConfigValue("congeal.optimize.error","parzen");
		
		while(1){

#if MULTITHREADED
			pthread_mutex_lock(&m_mutexReady); 
			while(!pai->m_bReady && !m_bDie){
				pthread_cond_wait(&m_condReady,&m_mutexReady); 
			}
			pai->m_bReady=false;
			pthread_mutex_unlock(&m_mutexReady); 

			if (m_bDie){
				pthread_barrier_wait(&m_barrierDone);
				return;
			}
#endif

			// gradientdescent -------------------------------------
			if (sAlgorithm == "gradientdescent"){
				pai->m_rErr = Optimize_OneGradientStep
				(
					m_vregParams[nSource].C(),
					m_vregParams[nSource].V(),
					m_regSteps.V(),
					( (sError == "parzen") 
						? MeasureInverseParzenProbabilityInPlace
						: MeasureVarianceInPlace),
					pai,
					m_rStep
				);
			}

			// gradientdescent -------------------------------------
			else if (sAlgorithm == "randomwalk"){
				pai->m_rErr = Optimize_GreedyRandomWalk
				(
					m_vregParams[nSource].C(),
					m_vregParams[nSource].V(),
					m_regSteps.V(),
					( (sError == "parzen") 
						? MeasureInverseParzenProbabilityInPlace
						: MeasureVarianceInPlace),
					pai,
					m_rStep
				);
			}

			// brute force -------------------------------------
			else if(sAlgorithm == "bruteforce"){
				pai->m_rErr = Optimize_BruteForce
				(
					m_vregParams[nSource].C(),
					m_vregParams[nSource].V(),
					m_regSteps.V(),
					MeasureInverseParzenProbabilityInPlace,
					pai,
					m_rStep
				);
			}
#if FEATURE_LBFGS
			// L-BFGS -------------------------------------------
			else if (sAlgorithm == "lbfgs"){
				ParamsToArray(pai->m_vparam,	m_vregParams[nSource]);

				lbfgs(
					m_vregParams[nSource].C(),
					pai->m_vparam,
					&pai->m_rErr,
					EvaluateForLBFGS,
					NULL,
					pai,
					NULL
				);

				ParamsFromArray(m_vregParams[nSource],pai->m_vparam);
			}
#endif
			// Nelder-Mead -------------------------------------------
			else if (sAlgorithm == "neldermead" || sAlgorithm == "simplex"){
				ParamsToArray(pai->m_vparam,	m_vregParams[nSource]);

				// TODO: think about m_rStep interacting with m_regSteps.V()
				pai->m_rErr = NelderMeadOptimize
				(
					m_vregParams[nSource].C(),
					pai->m_vparam,
					m_regSteps.V(),
					( (sError == "parzen") 
						? MeasureInverseParzenProbability
						: MeasureVariance),
					pai,
					.0001,
					10
				);

				ParamsFromArray(m_vregParams[nSource],pai->m_vparam);
			}

			// Unknown algorithm -------------------------------------------
			else{
				P("%s",DescribeConfiguration().VCH());
				CONGEAL_ERROR(
					"Unknown algorithm." 
					"Parameter congeal.optimize.algorithm must be one of {lbfgs|gradientdescent|neldermead}."
					"Algorithm given: '%s'", sAlgorithm.VCH()
				);
			}

#if MULTITHREADED
			pthread_barrier_wait(&m_barrierDone);
#else
			return;
#endif
		}
	}

	static void ParamsFromArray(RegistryOfParameters& reg, const Parameter* vparam){
		int c=reg.C();
		for (int n=0; n<c; n++){
			(*(reg.Get(n))) = vparam[n];
		}
	}

  static void ParamsToArray(Parameter* vparam, const RegistryOfParameters& reg){
		int c=reg.C();
		for (int n=0; n<c; n++){
			vparam[n] =	(*(reg.Get(n)));
		}
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
#if FEATURE_LBFGS
	static lbfgsfloatval_t EvaluateForLBFGS(
    void *pObjectiveInfo,
		const lbfgsfloatval_t *vparam,
    lbfgsfloatval_t *vparamDerivative,
    const int cParams,
    const lbfgsfloatval_t nStep
	)
	{
		AlignInfo* pai=(AlignInfo*)pObjectiveInfo;
		const Parameter* vSteps = pai->m_pcongeal->m_regSteps.V();

		static Real rKernelFactor=ConfigValue("congeal.optimize[lbfgs].kernel",30.);
		Real rErr=MeasureInverseParzenProbability(vparam, REAL_MAX, pObjectiveInfo);
		
		for (int n=0; n<cParams; n++){
			Parameter dKernel=vSteps[n] * rKernelFactor;
			Real rErrP1;
			// WARNING: making a const non-const for a moment
			{
				// change the value temporarily
				((Parameter*)vparam)[n] += dKernel;
				rErrP1=MeasureInverseParzenProbability(vparam, REAL_MAX, pObjectiveInfo);
				// get back to original value
				((Parameter*)vparam)[n] -= dKernel;
			}
			
			// compute gradient
			vparamDerivative[n]=(rErrP1-rErr)/dKernel;

			/* 
				 Using central difference is too expensive, 
				 requiring 2x function evaluations
				 vparam[n] -= 2*dKernel;
				 Real rErrM1=MeasureInverseParzenProbability(vparam, REAL_MAX, pObjectiveInfo);

				 // get back to original value
				 vparam[n] += dKernel;

				 vparamDerivative[n]=(rErrP1-rErrM1)/dKernel/2;
			*/
		}
		return rErr;
	}
#endif




	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	static Real Optimize_BruteForce(
		int cParams,
		Parameter** vpParams,
		Parameter* vSteps,
		Real fxnObjective(const void*),
		void* pObjectiveInfo,
		Real rStep
	){
		static Real rRangeFactor=ConfigValue("congeal.optimize[bruteforce].range",30.);

		Real rErrMin=REAL_MAX;

		for (int n=0; n<cParams; n++){
			int dMin=0;
			rErrMin=REAL_MAX;
			for (int d=-20; d<20; d++){
				
				Parameter dRange=vSteps[n] * rRangeFactor * d;
				(*(vpParams[n])) += dRange;
				Real rErrP1=fxnObjective(pObjectiveInfo);

				if (rErrP1<rErrMin){
					rErrMin=rErrP1;
					dMin=d;
				}
//				UD("ERROR ALONG %5d: delta of %5.5f --> %5.5f == %5.5f", n, dRange, (*(vpParams[n])), rErrP1);

				(*(vpParams[n])) -= dRange;
			}

			Parameter dRange=vSteps[n] * rRangeFactor * dMin;
			(*(vpParams[n])) += dRange;
		}
		return rErrMin;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	static Real Optimize_GreedyRandomWalk(
		int cParams,
		Parameter** vpParams,
		Parameter* vSteps,
		Real fxnObjective(const void*),
		void* pObjectiveInfo,
		Real rStep
	){
		static Real rKernelFactor=ConfigValue("congeal.optimize[randomwalk].kernel",30.);
		static int cSteps=ConfigValue("congeal.optimize[randomwalk].steps",100);
		static int cDirections=ConfigValue("congeal.optimize[randomwalk].directions",100);

		Real rErrIn=fxnObjective(pObjectiveInfo);
		Real rErrMin=rErrIn;
		
		for (int nDir=0; nDir<cDirections; nDir++){
			int nDim=rand()%cParams;
			Parameter dStep=vSteps[nDim] * rStep * rKernelFactor;
			dStep *= Random(-1.,1.);

//			Parameter dStepThreshold = vSteps[nDim] / 100000;

			for (int nStep=0; nStep<cSteps; nStep++){
				(*(vpParams[nDim])) += dStep;

				Real rErrNew=fxnObjective(pObjectiveInfo);
				
				if (rErrNew>=rErrMin){
					(*(vpParams[nDim])) -= dStep;
					break;
/*

					dStep=-dStep*.9;
					if (abs(dStep) < dStepThreshold){
						break;
					}
*/
				}
				else{
					rErrMin=rErrNew;
				}
			}
		}

		return rErrMin;
	}


	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	static Real Optimize_OneGradientStep(
		int cParams,
		Parameter** vpParams,
		Parameter* vSteps,
		Real fxnObjective(const void*),
		void* pObjectiveInfo,
		Real rStep
	){
		static Real rKernelFactor=ConfigValue("congeal.optimize[gradientdescent].kernel",30.);
		Real rErr=fxnObjective(pObjectiveInfo);
		
		// initialize static buffer for holding derivative
		Parameter* vparamDerivative=new Parameter[cParams];
		
		for (int n=0; n<cParams; n++){
			Parameter dKernel=vSteps[n] * rStep * rKernelFactor;

			(*(vpParams[n])) += dKernel;
			Real rErrP1=fxnObjective(pObjectiveInfo);

			UD("LOOKING AT GRADIENT ALONG %d SHIFTED BY %g GIVES %g-->%g", n, dKernel,rErr,rErrP1);
			// get back to original value
			(*(vpParams[n])) -= dKernel;
			
			// compute gradient
			vparamDerivative[n]=(rErrP1-rErr)/dKernel;

			/* 
				 Using central difference is too expensive, 
				 requiring 2x function evaluations
				 (*(vpParams[n])) -= 2*dKernel;
				 Real rErrM1=fxnObjective(pObjectiveInfo);

				 // get back to original value
				 (*(vpParams[n])) += dKernel;

				 UD("LOOKING AT GRADIENT ALONG %d SHIFTED BY %g GIVES %g-->%g", n, -dKernel,rErr,rErrM1);

				 vparamDerivative[n]=(rErrP1-rErrM1)/dKernel/2;
			*/
		}
//		AlignInfo* pai=(AlignInfo*)pObjectiveInfo;

		// normalize the gradient
/*
		Real rT=0;
		for (int n=0; n<cParams; n++){
			rT += sqr(vparamDerivative[n]);
		}
		rT=sqrt(rT);
		for (int n=0; n<cParams; n++){
			vparamDerivative[n]/=rT;
		}
*/

		static Real rStepFactor=ConfigValue("congeal.optimize[gradientdescent].step",30.);
		static bool bWalkAlongGradient=ConfigValue("congeal.optimize[gradientdescent].walk",true);

		if (!bWalkAlongGradient){
			for (int n=0; n<cParams; n++){
				(*(vpParams[n])) += -vparamDerivative[n] * rStep * rStepFactor;
			}
			Real rErrNew=fxnObjective(pObjectiveInfo);
			delete [] vparamDerivative;
			return rErrNew;
		}


		static int cWalkSteps=ConfigValue("congeal.optimize[gradientdescent].walk.steps",30);

		Real rdStep=1.5 * rStep * rStepFactor / cWalkSteps;
		Real rErrMin=rErr;
		int nWalkStepMin=-1;

		// note we're taking the negative of the derivative here 
		for (int n=0; n<cParams; n++){
			vparamDerivative[n] = -vparamDerivative[n] * rdStep;
		}
		for (int nWalkStep=0; nWalkStep<cWalkSteps; nWalkStep++){
			for (int n=0; n<cParams; n++){
				(*(vpParams[n])) += vparamDerivative[n];
			}
			Real rErrNew=fxnObjective(pObjectiveInfo);
			if (rErrNew<rErrMin){
				UD("%d] %g < %g ***", nWalkStep, rErrNew, rErrMin);
				rErrMin=rErrNew;
				nWalkStepMin=nWalkStep;
			}
			else{
				UD("%d] %g", nWalkStep, rErrNew);
			}
		}

//		UD("min error was %g compared to %g", rErrMin, rErr);
//		UD("shifting by: %d of %d",  (cWalkSteps - (nWalkStepMin+1)), cWalkSteps);
		for (int n=0; n<cParams; n++){
			(*(vpParams[n])) -= vparamDerivative[n] * (cWalkSteps - (nWalkStepMin+1));
		}
		Real rErrNew=fxnObjective(pObjectiveInfo);

		UD("%g --> %g %s", rErr, rErrNew, (rErr==rErrNew?"":"*"));
		delete [] vparamDerivative;
		return rErrNew;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	// note this function copies the parameters from the AlignInfo into 
	// the recipie and then measures the variance
	static Real MeasureVariance(const Parameter* vparam, const Real& rErrMin, const void* p)
	{
		AlignInfo* pai = (AlignInfo*)p;

		ParamsFromArray(
			pai->m_pcongeal->m_vregParams[pai->m_nSource],
			vparam
		);
		return MeasureVarianceInPlace(p);
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	// note this function assumes parameter values are already inserted into
	// the recipie
	static Real MeasureVarianceInPlace(const void* p)
	{
		AlignInfo* pai = (AlignInfo*)p;

		const int& nSource = pai->m_nSource;
		const int& cSamples=pai->m_pcongeal->m_cSamples;
		const int& cSources=pai->m_pcongeal->m_cSources;

		// get ready to access
		pai->m_pcongeal->m_precipie->PrepareForAccess(nSource);
		
		Real rErrT=0;

		for (int nSample=0; nSample<cSamples; nSample++){

			DATA dataReplacement;
			::GetPoint(
				*(pai->m_pcongeal->m_precipie->PSource(nSource)),
				dataReplacement,
				pai->m_pcongeal->m_vpt[nSample]
			);

			// compute a new sum and sumsq by taking out the excluded data 
			// (which is presumably the source we're modifying)
			// and adding in the new data
			const DATA& dataExcluded = pai->m_pcongeal->m_mdata[nSample*cSources+nSource];

			const RANGEPROMOTION(DATA)& dataSum   =	(
				pai->m_pcongeal->m_vdataSum[nSample] 
				- dataExcluded
				+ dataReplacement
			);

			const RANGEPROMOTION(DATA)& dataSumSq =	(
				pai->m_pcongeal->m_vdataSumSq[nSample] 
				- sqr(dataExcluded)
				+ sqr(dataReplacement)
			);

			// TODO: apply Length() to this for general purpose computation
			Real rErr = dataSumSq/cSources - sqr(dataSum/cSources);

			rErrT+=rErr;
		}
		rErrT /= cSamples;

		return rErrT;
	}


	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	// note this function copies the parameters from the AlignInfo into 
	// the recipie and then measures the variance
	static Real MeasureInverseParzenProbability(const Parameter* vparam, const Real& rErrMin, const void* p)
	{
		AlignInfo* pai = (AlignInfo*)p;

		ParamsFromArray(
			pai->m_pcongeal->m_vregParams[pai->m_nSource],
			vparam
		);
		return MeasureInverseParzenProbabilityInPlace(p);
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	// note this function assumes parameter values are already inserted into
	// the recipie
	static Real MeasureInverseParzenProbabilityInPlace(const void* p)
	{
		static Real rSigma=ConfigValue("congeal.error[parzen].sigma",10.);
		AlignInfo* pai = (AlignInfo*)p;
				
		const int& nSourceTest = pai->m_nSource;
		const int& cSamples=pai->m_pcongeal->m_cSamples;
		const int& cSources=pai->m_pcongeal->m_cSources;

		// get ready to access
		pai->m_pcongeal->m_precipie->PrepareForAccess(nSourceTest);
		
		Real rLogProbTotal=1.;

		int cConsideredSamples=0;
//		UD(String("COMPARING SAMPLES: ") + cSamples);
		for (int nSample=0; nSample<cSamples; nSample++){
			DATA dataTest;
			::GetPoint(
				*(pai->m_pcongeal->m_precipie->PSource(nSourceTest)),
				dataTest,
				pai->m_pcongeal->m_vpt[nSample]
			);
			m_cLookups++;


/*

			if (dataTest==0){
				continue;
			}


			while (dataTest==0){
				::GetPoint(
					*(pai->m_pcongeal->m_precipie->PSource(nSourceTest)),
					dataTest,
					NewRandomPoint(pai->m_pcongeal->m_precipie->Size())
				);
				m_cLookups++;
			}
*/

			cConsideredSamples++;
			Real rLogProbSample=0;
			int cConsideredSources=0;

			for (int nSource=0; nSource<cSources; nSource++){
				if (nSource==nSourceTest){
					continue;
				}

				DATA dataPrior = pai->m_pcongeal->m_mdata[nSample*cSources+nSource];

				if (dataPrior==0){
					UD(String("Hmm? dataPrior is 0?")  + pai->m_pcongeal->m_vpt[nSample]);
					continue;
				}

/*
				int cRelookups=0;
				while (dataPrior==0){
					::GetPoint(
						*(pai->m_pcongeal->m_precipie->PSource(nSource)),
						dataPrior,
						NewRandomPoint(pai->m_pcongeal->m_precipie->Size())
					);
					m_cLookups++;
					cRelookups++;
					D("relookups: %d", cRelookups);
				}
*/

				cConsideredSources++;

				Real rProbSource = Gaussian(dataTest, dataPrior, rSigma);

				CONGEAL_ASSERTf(rProbSource>=0, "NEGATIVE ERROR %g", rProbSource);

				static Real rApriori=ConfigValue("congeal.error[parzen].apriori",.2);
				Real rLogProbSource = -log(rProbSource + rApriori);
				rLogProbSample += rLogProbSource;

//				UD(String("COMPARING ") + dataTest + " with " + dataPrior + " getting " + rProbSource + " logs to " + rLogProbSource + " and combines to " + rLogProbSample);
			}

			if (cConsideredSources>0){
				rLogProbTotal +=rLogProbSample/cConsideredSources;
			}

//			UD(String("SOURCES COMPARED: ") + cConsideredSources);
		}

		if (cConsideredSamples>0){
			rLogProbTotal /= cConsideredSamples;
		}
//		UD(String("SAMPLES COMPARED: ") + cConsideredSamples + " yields " + rLogProbTotal);

		Real rError = rLogProbTotal;

		return rError;
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	// note this function assumes parameter values are already inserted into 
	// the recipie
	static Real MeasureFullInverseParzenProbabilityInPlace(const void* p)
	{
		static Real rSigma=ConfigValue("congeal.error[parzen].sigma",10.);
		AlignInfo* pai = (AlignInfo*)p;
				
		const int& nSourceTest = pai->m_nSource;
		const int& cSources=pai->m_pcongeal->m_cSources;

		// get ready to access
		pai->m_pcongeal->m_precipie->PrepareForAccess(nSourceTest);
		
		Real rLogProbTotal=1.;
		
		// loop over all the points and measure the error
		forpoint(TypeOfPoint(RECIPIE),pt,0,(pai->m_pcongeal->m_precipie->PSource(nSourceTest))->Size()){
			DATA dataTest;
			::GetPoint(
				*(pai->m_pcongeal->m_precipie->PSource(nSourceTest)),
				dataTest,
				pt
			);

			Real rLogProbSample=0;
			for (int nSource=0; nSource<cSources; nSource++){
				if (nSource==nSourceTest){
					continue;
				}
				DATA dataPrior;
				::GetPoint(
					*(pai->m_pcongeal->m_precipie->PSource(nSource)),
					dataPrior,
					pt
				);

				Real rProbSource = Gaussian(dataTest, dataPrior, rSigma);

				CONGEAL_ASSERTf(rProbSource>=0, "NEGATIVE ERROR %g", rProbSource);

				static Real rApriori=ConfigValue("congeal.error[parzen].apriori",.2);
				rLogProbSample += -log(rProbSource + rApriori);
			}

			rLogProbTotal +=rLogProbSample;
		}

		rLogProbTotal /= (pai->m_pcongeal->m_precipie->PSource(nSourceTest))->CSize();
		rLogProbTotal /= cSources;

		Real rError = rLogProbTotal;

		return rError;
	}

		


};

template<class RECIPIE>
long int CongealGroupPhasedOf<RECIPIE>::m_cLookups;


#endif //__CONGEALGROUPPHASED_H__



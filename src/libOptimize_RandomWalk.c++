#include "libOptimize_NelderMead.h"
#include "libPGM.h"
#include "libUtility.h"
#include <float.h> // for DBL_MAX
#undef DEBUG 
#define DEBUG 0
#include "libDebug.h"


//============================================================================
// based upon the discussion and terminology found in Wikipedia
// http://en.wikipedia.org/wiki/Nelder-Mead_method
//============================================================================
bool Optimize_RandomWalk(
	int cDimensions,
	double* vrParamMin,
	double* vrInitialScale,
	double fxnEval(double*, const double& rErrMin, const void*),
	const void* pData,
	double rErrThreshold,
	int cIterations
){
	double rErrMin=fxnEval(vrParamMin,DBL_MAX,pData);

	double* vrParamTest = new double[cDimensions];
	double* vrd = new double[cDimensions];

	double *prTest=vrParamTest;
	double *prMin=vrParamMin;

	cIterations*=sqrt(cDimensions);
	int nIteration;

	for (int nDimension=0; nDimension<cDimensions; nDimension++){
		vrd[nDimension]=Random(-vrInitialScale[nDimension],vrInitialScale[nDimension])/(20);
	}

	int nStepDimension=0;
	int nStep=0;

	for (nIteration=0; nIteration<cIterations; nIteration++){
		for (int nDimension=0; nDimension<cDimensions; nDimension++){
			prTest[nDimension]=prMin[nDimension]+vrd[nDimension];
		}

		if (0 == (nIteration%100)){
			UD(
				"---------------- ITERATION %6d / %6d ---------------- [%g]", 
				nIteration, cIterations, rErrMin
			);
		}

		double rErr = fxnEval(prTest,DBL_MAX,pData);
		if (rErr<rErrMin){
			rErrMin=rErr;
			Swap(prMin,prTest);
			nStep=0;
		}
		else{
			if (nStep==0){
				vrd[nStepDimension]=-vrd[nStepDimension]*.9;
				nStep=1;
			}
			else{
				vrd[nStepDimension]=0;
				nStepDimension++;
				nStepDimension%=cDimensions;
				vrd[nStepDimension]=Random(-vrInitialScale[nStepDimension],vrInitialScale[nStepDimension])/(20);
				nStep=0;
			}
		}
	}

	if (prMin!=vrParamMin){
		for (int nDimension=0; nDimension<cDimensions; nDimension++){
			vrParamMin[nDimension]=prMin[nDimension];
		}
	}

	UD("---------------- FINAL ----------------");
	UD("iterations: %d    Error: %g", nIteration, rErrMin);
	UD("Parameters:");

	int cAcross=10;
	for (int nDimension=0; nDimension<cDimensions; nDimension+=cAcross){
		UIDX(2,"%3d-%3d] ", nDimension, min(cDimensions-1,nDimension+cAcross));
		for (int d=0; d<cAcross && (d+nDimension) < cDimensions; d++){
			PX("%+1.2f ", vrParamMin[nDimension]);
		}
		P(" ");
	}
	P("\n");


	// cleanup
	delete [] vrParamTest;

	return true;
}

	
	




//============================================================================
// template instantiation
//============================================================================


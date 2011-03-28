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
Parameter NelderMeadOptimize(
	int cDimensions,
	Parameter* vrParamMin,
	Parameter* vrInitialScale,
	Parameter fxnEval(const Parameter*, const Parameter& rErrMin, const void*),
	const void* pData,
	Real rErrThreshold,
	int cIterations
){
	int cPoints = cDimensions+1;

	Parameter** vvrParam = new Parameter* [cPoints];
	Parameter* vrErr = new Parameter [cPoints];

	Parameter* vrParamCentroid = new Parameter [cDimensions];
	Parameter* vrParamReflection = new Parameter [cDimensions];
	Parameter* vrParamExpansion = new Parameter [cDimensions];
	Parameter* vrParamContraction = new Parameter [cDimensions];

	// initalize parameter sets to cardinal axis scaled by given 
	// initial scale last parameter is origin {0,...,0}
	for (int nPoint=0; nPoint<cPoints; nPoint++){
		vvrParam[nPoint]=new Parameter[cDimensions];
		for (int nDimension=0; nDimension<cDimensions; nDimension++){

			if (nPoint==nDimension){
				vvrParam[nPoint][nDimension] = vrParamMin[nDimension] + vrInitialScale[nDimension];
			}
			else {
				vvrParam[nPoint][nDimension] = vrParamMin[nDimension];
			}
		}
	}

	// evaluate all points to get starting Simplex
	for (int nPoint=0; nPoint<cPoints; nPoint++){
		vrErr[nPoint] = fxnEval(vvrParam[nPoint],DBL_MAX,pData);
	}


	Real rErrMin;
	int nPointMin;
	bool bMinFound = false;

	const Real rFactorReflection = 1.0;
	const Real rFactorExpansion = 2.0;
	const Real rFactorContraction = 0.5;
	const Real rFactorReduction = 0.5;

	// begin iteration
	int nIteration=0; 
	for (;;nIteration++){

		// find min/max error
		rErrMin=1e100;
		nPointMin=-1;
		Real rErrMax=-1;
		int nPointMax=-1;
		Real rErrSecondMax=-1;
		int nPointSecondMax=-1;

		for (int nPoint=0; nPoint<cPoints; nPoint++){
			if (rErrMin > vrErr[nPoint]){
				rErrMin=vrErr[nPoint];
				nPointMin=nPoint;
			}
			if (rErrMax < vrErr[nPoint]){
				nPointSecondMax=nPointMax;
				rErrSecondMax=rErrMax;
				rErrMax=vrErr[nPoint];
				nPointMax=nPoint;
			}
			else if (rErrSecondMax < vrErr[nPoint]){
				rErrSecondMax=vrErr[nPoint];
				nPointSecondMax=nPoint;
			}
		}

//		D("MIN(%d @ %g)", nPointMin,rErrMin);
//		D("MAX(%d @ %g)", nPointMax,rErrMax);
//		D("SECONDMAX(%d @ %g)", nPointSecondMax,rErrSecondMax);

/*
		if (0 == (nIteration%100)){
			UD(
				"---------------- ITERATION %d ---------------- [%g %g]", 
				nIteration, rErrMin, rErrMax
			);
			UD1("RANGE(%g - %g - %g)", rErrMin, rErrSecondMax, rErrMax);
		}
*/

		// check if minimum sufficiently small
		const Real rErrFraction = (rErrMax - rErrMin) / (rErrMax + rErrMin+1);
		D1("COMPARING (%g,%g) --> %g and %g", rErrMax, rErrMin, rErrFraction, rErrThreshold);
		if ( rErrFraction < rErrThreshold){
			D("FOUND LOCAL MIN");
			bMinFound = true;
			break;
		}

		// check if too many iterations
		if (nIteration >= cIterations){
			bMinFound = false;
			break;
		}

		// compute centroid of all points
		for (int nDimension=0; nDimension<cDimensions; nDimension++){
			vrParamCentroid[nDimension]=0;
		}
	
		for (int nPoint=0; nPoint<cPoints; nPoint++){
			if (nPoint == nPointMax){
				continue;
			}
			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				vrParamCentroid[nDimension]+=vvrParam[nPoint][nDimension];
			}
		}
		for (int nDimension=0; nDimension<cDimensions; nDimension++){
			vrParamCentroid[nDimension]/=(cPoints-1);
		}


		// compute reflection
		for (int nDimension=0; nDimension<cDimensions; nDimension++){
			vrParamReflection[nDimension]	= (
				vrParamCentroid[nDimension] + (
					vrParamCentroid[nDimension] - 
					vvrParam[nPointMax][nDimension]
				) * rFactorReflection
			);
		}

		// compute reflection error
		Real rErrReflection = fxnEval(vrParamReflection,DBL_MAX,pData);
		D1("REFLECTION ERR %g", rErrReflection);

		// if better than second worst, but not better than best, use it as point
		if (rErrReflection < rErrSecondMax && rErrReflection > rErrMin){
			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				vvrParam[nPointMax][nDimension]=vrParamReflection[nDimension];
			}
			vrErr[nPointMax]=rErrReflection;
			D1("REFLECTED");
			continue;
		}

		// if best point found so far, then try expanding
		if (rErrReflection <= rErrMin){
			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				vrParamExpansion[nDimension] = (
					vrParamCentroid[nDimension] + (
						vrParamCentroid[nDimension] - 
						vvrParam[nPointMax][nDimension]
					) * rFactorExpansion
				);
			}
			Real rErrExpansion = fxnEval(vrParamExpansion,DBL_MAX,pData);

			D1("EXPANSION ERR %g", rErrReflection);

			// expansion point is better than reflection point, use it
			if (rErrExpansion < rErrReflection){
				for (int nDimension=0; nDimension<cDimensions; nDimension++){
					vvrParam[nPointMax][nDimension]=vrParamExpansion[nDimension];
				}
				vrErr[nPointMax]=rErrExpansion;
				D1("EXPANDED");
			}
			// otherwise use reflection point as above
			else{
				for (int nDimension=0; nDimension<cDimensions; nDimension++){
					vvrParam[nPointMax][nDimension]=vrParamReflection[nDimension];
				}
				vrErr[nPointMax]=rErrReflection;
				D1("REFLECTED");
			}
			continue;
		}


		// our reflection point must be the worst found 
		// since all cases exhausted above
		//assert (rErrReflection > rErrSecondMax);


		// compute the contracted point
		for (int nDimension=0; nDimension<cDimensions; nDimension++){
			vrParamContraction[nDimension] = (
				vvrParam[nPointMax][nDimension] + (
					vrParamCentroid[nDimension] - 
					vvrParam[nPointMax][nDimension]
				) * rFactorContraction
			);
		}
		Real rErrContraction = fxnEval(vrParamContraction,DBL_MAX,pData);
		D1("CONTRACTION ERR %g", rErrContraction);

		if (rErrContraction < rErrMax){
			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				vvrParam[nPointMax][nDimension]=vrParamContraction[nDimension];
			}
			vrErr[nPointMax]=rErrContraction;
			D1("CONTRACTED");
			continue;
		}


		// otherwise perform reduction
		for (int nPoint=0; nPoint<cPoints; nPoint++){
			if (nPoint == nPointMin){
				continue;
			}
			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				vvrParam[nPoint][nDimension] = (
					vvrParam[nPointMin][nDimension] + (
						vvrParam[nPoint][nDimension] 
						- vvrParam[nPointMin][nDimension] 
					) * rFactorReduction
				);
			}
			vrErr[nPoint] = fxnEval(vvrParam[nPoint],DBL_MAX,pData);
		}
		D1("REDUCED");
	}

	// copy over the final result
	for (int nDimension=0; nDimension<cDimensions; nDimension++){
		vrParamMin[nDimension]=vvrParam[nPointMin][nDimension];
	}


/*
	UD("---------------- FINAL ----------------");
	UD("iterations: %d    Error: %g", nIteration, rErrMin);
	UD("Parameters:");

	int cAcross=10;
	for (int nDimension=0; nDimension<cDimensions; nDimension+=cAcross){
		UIDX(2,"%3d-%3d] ", nDimension, min(cDimensions-1,nDimension+cAcross));
		for (int d=0; d<cAcross && (d+nDimension) < cDimensions; d++){
			PX("%+1.2f ", vvrParam[nPointMin][nDimension]);
		}
		P(" ");
	}
	P("\n");
*/


	// cleanup
	for (int nPoint=0; nPoint<cPoints; nPoint++){
		delete [] vvrParam[nPoint];
	}
	delete [] vvrParam;
	delete [] vrErr;
	delete [] vrParamReflection;
	delete [] vrParamCentroid;
	delete [] vrParamExpansion;
	delete [] vrParamContraction;

	return rErrMin;
}

	
	




//============================================================================
// template instantiation
//============================================================================


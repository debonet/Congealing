#include "libOptimize_NelderMead.h"
#include "libPGM.h"
#include "libUtility.h"
#include <math.h>

//#define DEBUG1
#include "libDebug.h"

//============================================================================
// Simple algorithm which runs for a specified number of generations/iterations
// in each iteration, evalation points are picked at random within the scale /
// swarm bound around the starting point. The point with the best evaluation
// becemes the starting point for the next generation. Each subsequent
// generation has a smaller swarm bound than the preceeding.
//============================================================================
bool OptimizeGreedySwarm(
	int cDimensions,
	double* vrParamMin,
	double* vrInitialScale,
	double fxnEval(double*, const void*),
	const void* pData,
	double rErrThreshold,
	int cIterations
){

	D("OPTIMIZING %d PARAMETERS", cDimensions);
	cIterations=int(floor(sqrt(cIterations)));
	double* vrParam = new double [cDimensions];
	double* vrParamLast = new double [cDimensions];

	for (int nDimension=0; nDimension<cDimensions; nDimension++){
		vrParamLast[nDimension] = vrParamMin[nDimension];
	}

	double rErrMin = 1e200;
	double rErrLast = 1e200;

	const int cTestPoints = cIterations*10;

	for (int nIteration=0; nIteration<cIterations; nIteration++){
		double rScale=(1.0-double(nIteration)/(cIterations+1))/2;

		for (int nTestPoint=0; nTestPoint<cTestPoints; nTestPoint++){

			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				double rPoint = double(rand()%2000000)/1000000-1.0;
				vrParam[nDimension] = vrParamLast[nDimension] + rPoint * vrInitialScale[nDimension] * rScale;
			}

			double rErr = fxnEval(vrParam,pData);

			if (rErr < rErrMin){
				rErrMin = rErr;
				// Swap(vrParam, vrParamMin);
				for (int nDimension=0; nDimension<cDimensions; nDimension++){
					vrParamMin[nDimension] = vrParam[nDimension];
				}
			}
		}

		D("Iteration %d/%d MinError %2.5f SCALE %2.5f", nIteration, cIterations, rErrMin, rScale);

		if (rErrMin < rErrLast){
			if (rErrMin < .95*rErrLast){
				D("EXPLOIT!");
				nIteration--;
			}
			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				vrParamLast[nDimension] = vrParamMin[nDimension];
			}
			rErrLast=rErrMin;
		}
	}


	D("FINAL:");
	for (int nDimension=0; nDimension<cDimensions; nDimension++){
		printf("%1.2f ", vrParamMin[nDimension]);
	}
	printf("\n");

	delete [] vrParam;
	delete [] vrParamLast;

	return true;
}

	
	

//============================================================================
// Move in random direction. If beneficial, restart counter,  restart step
// counter, continue until its no longer beneficial. Then pick another
// direction and smaller step, and continue. If cIterations unsuccessful 
// directions in a row, then stop.
//============================================================================
bool OptimizeGreedySwarmX(
	int cDimensions,
	double* vrParamMin,
	double* vrInitialScale,
	double fxnEval(double*, const void*),
	const void* pData,
	double rErrThreshold,
	int cIterations
){

	D("OPTIMIZING %d PARAMETERS", cDimensions);
//	cIterations=floor(sqrt(cIterations));
	double* vrParam = new double [cDimensions];
	double* vrdParam = new double [cDimensions];

	double rErrMin = fxnEval(vrParamMin,pData);

	for (int nIteration=0; nIteration<cIterations; nIteration++){
		double rScale=(1.0-double(nIteration)/(cIterations+1))/2;
//		double rScale=(double(nIteration)/(cIterations+1))/2;

		for (int nDimension=0; nDimension<cDimensions; nDimension++){
			double rPoint = double(rand()%2000000)/1000000-1.0;
			vrdParam[nDimension] = rPoint * vrInitialScale[nDimension] * rScale;
		}

		while(1){
			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				vrParam[nDimension] = vrParamMin[nDimension] + vrdParam[nDimension];
			}
			
			double rErr = fxnEval(vrParam,pData);

			if (rErr < rErrMin){
				D("Iteration %d/%d MinError %2.5f SCALE %2.5f", nIteration, cIterations, rErrMin, rScale);
				if (rErr < .95*rErrMin){
					nIteration=0;
				}
				rErrMin = rErr;
				for (int nDimension=0; nDimension<cDimensions; nDimension++){
					vrParamMin[nDimension] = vrParam[nDimension];
				}
			}
			else{
				break;
			}
		}

	}


	D("FINAL:");
	for (int nDimension=0; nDimension<cDimensions; nDimension++){
		printf("%1.2f ", vrParamMin[nDimension]);
	}
	printf("\n");

	delete [] vrParam;
	delete [] vrdParam;

	return true;
}

	
	

	

//============================================================================
//============================================================================
bool OptimizeGreedySwarm2(
	int cDimensions,
	double* vr0,
	double* vrInitialScale,
	double fxnEval(double*, const void*),
	const void* pData,
	double rErrThreshold,
	int cIterations
){

	D("OPTIMIZING %d PARAMETERS", cDimensions);
//	cIterations=floor(sqrt(cIterations));
	double* vrd = new double [cDimensions];
	double* vr1 = new double [cDimensions];
	double* vrTest = new double [cDimensions];

	double rErr0 = fxnEval(vr0,pData);

	int cSteps = 100;
	for (int nIteration=0; nIteration<cIterations; nIteration++){
		double rScale=(1.0-double(nIteration)/(cIterations+1))/2;

		// compute differential vector vrd
		for (int nDimension=0; nDimension<cDimensions; nDimension++){
			double rPoint = double(rand()%2000000)/1000000-1.0;
			vrd[nDimension] = rPoint * vrInitialScale[nDimension] * rScale;
		}

		bool bReflect=false;
		for (int nSteps=0; nSteps<cSteps; nSteps++){
			// compute vector 1
			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				vr1[nDimension] = vr0[nDimension] + vrd[nDimension];
			}
			double rErr1 = fxnEval(vr1,pData);

			bool bMoreThan0=false;
			bool bMoreThan1=false;
			bool bLessThan01=false;

			int cTestPoints = 10;
			// compute intermediate points
			for (int nTestPoint=0; nTestPoint<cTestPoints; nTestPoint++){
				double rPoint = double(rand()%1000000)/1000000;
				for (int nDimension=0; nDimension<cDimensions; nDimension++){
					vrTest[nDimension] = vr0[nDimension] + rPoint * vrd[nDimension];
				}
				double rErrTest = fxnEval(vrTest,pData);
			
				if (rErrTest > rErr0){
					bMoreThan0 = true;
				}

				if (rErrTest > rErr1){
					bMoreThan1 = true;
				}

				if (rErrTest < rErr0 && rErrTest < rErr1){
					bLessThan01 = true;
				}
			}


			// case 1
			if (!bLessThan01){
				D1("CASE 1");
				if (rErr0 < rErr1){
					D1("CASE 1a");
					if (bReflect){
//						for (int nDimension=0; nDimension<cDimensions; nDimension++){
//							vrd[nDimension] = -vrd[nDimension]/2;
//						}
//						D("BOUNCEAROUND");
						break;
					}
					else{
						bReflect=true;
						for (int nDimension=0; nDimension<cDimensions; nDimension++){
							vrd[nDimension] = -vrd[nDimension];
						}
					}
				}
				else{
					D1("CASE 1b");
					rErr0=rErr1;
					for (int nDimension=0; nDimension<cDimensions; nDimension++){
						vr0[nDimension] = vr1[nDimension];
					}
				}
			}
			// case 2
			else if (!bMoreThan0 || !bMoreThan1){
				D1("CASE 2");
				if (rErr0 < rErr1){
					for (int nDimension=0; nDimension<cDimensions; nDimension++){
						vrd[nDimension] = vrd[nDimension]/2;
					}
				}
				else{
					rErr0=rErr1;
					for (int nDimension=0; nDimension<cDimensions; nDimension++){
						vr0[nDimension] = vr1[nDimension];
						vrd[nDimension] = -vrd[nDimension]/2;
					}
				}
			}
			else { // assert (bLessThan01)
				D1("CASE 3");
				if (rErr0 < rErr1){
					for (int nDimension=0; nDimension<cDimensions; nDimension++){
						vr0[nDimension] = vr0[nDimension] + vrd[nDimension]/3;
						vrd[nDimension] = vrd[nDimension]/3;
					}
				}
				else{
					rErr0=rErr1;
					for (int nDimension=0; nDimension<cDimensions; nDimension++){
						vr0[nDimension] = vr0[nDimension] + 2*vrd[nDimension]/3;
						vrd[nDimension] = -vrd[nDimension]/3;
					}
				}
			}

			double rdTotal=0;
			for (int nDimension=0; nDimension<cDimensions; nDimension++){
				rdTotal+=sqr(vrd[nDimension]);
			}
			if (rdTotal<rErrThreshold){
//				D("PUNT AFTER %d STEPS", nSteps);
				break;
			}

		}
		if (0 == (nIteration % 100)){
			D("Iteration %d/%d MinError %2.5f SCALE %2.5f", nIteration, cIterations, rErr0, rScale);
		}
	}


	D("FINAL:");
	for (int nDimension=0; nDimension<cDimensions; nDimension++){
		printf("%1.2f ", vr0[nDimension]);
	}
	printf("\n");

	delete [] vrd;
	delete [] vr1;
	delete [] vrTest;

	return true;
}

	
//============================================================================
//============================================================================
bool OptimizeGreedySwarm3(
	int cDimensions,
	double* vrMin,
	double* vrInitialScale,
	double fxnEval(double*, const void*),
	const void* pData,
	double rErrThreshold,
	int cIterations
){

	D("OPTIMIZING %d PARAMETERS", cDimensions);
//	cIterations=floor(sqrt(cIterations));
	double* vr0 = new double [cDimensions];
	double* vr1 = new double [cDimensions];
	double* vrTest = new double [cDimensions];

	for (int nDimension=0; nDimension<cDimensions; nDimension++){
		vr0[nDimension]=vrMin[nDimension];
	}
	double rErr0 = fxnEval(vr0,pData);
	double rErrFirst = rErr0;

	cIterations *= 10;
	for (int nIteration=0; nIteration<cIterations; nIteration++){
		double rScale=(1.0-double(nIteration)/(cIterations+1))/4;

		// compute differential vector vrd
		for (int nDimension=0; nDimension<cDimensions; nDimension++){
			double rPoint = double(rand()%2000000)/1000000-1.0;
			vr1[nDimension] = vr0[nDimension] + rPoint * vrInitialScale[nDimension] * rScale;
		}

		double rErr1 = fxnEval(vr1,pData);

		if (rErr1<(rErr0 * (1.0 + rScale/15)) && rErr1<rErrFirst){
			double *vrT=vr0;
			vr0=vr1;
			vr1=vrT;
			rErr0=rErr1;
			D("Iteration %d/%d MinError %2.5f SCALE %2.5f", nIteration, cIterations, rErr0, rScale);
		}
	}

	 D("FINAL:");
	for (int nDimension=0; nDimension<cDimensions; nDimension++){
		vrMin[nDimension]=vr0[nDimension];
		printf("%1.2f ", vr0[nDimension]);
	}
	printf("\n");

	delete [] vr0;
	delete [] vr1;
	delete [] vrTest;

	return true;
}

	
	




//============================================================================
// template instantiation
//============================================================================


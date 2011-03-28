bool OptimizeGreedySwarm(
	int cDimensions,
	double* vrParamMin,
	double* vrInitialScale,
	double fxnEval(double*, const void*),
	const void* pData,
	double rErrThreshold,
	int cIterations
);

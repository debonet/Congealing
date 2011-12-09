#ifndef __CONGEAL_RECIPIE_OUTPUT_H
#define __CONGEAL_RECIPIE_OUTPUT_H

#include "libPGM.h"
#include "SourceCombineLayout.h"
#include "libMatlabIO.h"


// set options for both CongealRecipieOutput and main
typedef NiftiDataVolume ScanVolume;
typedef NiftiPixel ScanPixel;
#if DATARANGE==256
  typedef Source_G2R_Layout_G2R ScanLayout;
#elif DATARANGE==4096
  typedef Source_S2R_Layout_S2R ScanLayout;
#elif DATARANGE==65536
  typedef Source_S2R_Layout_S2R ScanLayout;
#else
	error("unknown DATARANGE compilation value");
#endif

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// note this function assumes parameter values are already inserted into 
// the recipie
template <class SOURCE>
Real MeasureFullInverseParzenProbabilityInPlace(int cSources, SOURCE *vsrc)
{
	typedef typename SOURCE::type_data DATA;
	static Real rSigma=ConfigValue("congeal.error[parzen].sigma",10.);
	static Real rApriori=ConfigValue("congeal.error[parzen].apriori",.2);

	// get ready to access
	for (int n=0; n<cSources; n++){
		vsrc[n].PrepareForAccess();
	}

	Real rError=0;
		
	for (int nSourceTest=0; nSourceTest<cSources; nSourceTest++){
		Real rLogProbTotal=1.;
		
		// loop over all the points and measure the error
		forpoint(TypeOfPoint(SOURCE),pt,0,vsrc[nSourceTest].Size()){
			DATA dataTest;
			::GetPoint(vsrc[nSourceTest],dataTest,pt);

			Real rLogProbSample=0;
			for (int nSource=0; nSource<cSources; nSource++){
				if (nSource==nSourceTest){
					continue;
				}
				DATA dataPrior;
				::GetPoint(vsrc[nSource],dataPrior,	pt);

				Real rProbSource = Gaussian(dataTest, dataPrior, rSigma);

				CONGEAL_ASSERTf(rProbSource>=0, "NEGATIVE ERROR %g", rProbSource);

				rLogProbSample += -log(rProbSource + rApriori);
			}

			rLogProbTotal +=rLogProbSample;
		}
		rLogProbTotal /= vsrc[nSourceTest].CSize();
		rLogProbTotal /= cSources;
		rError += rLogProbTotal;
	}

	return rError;
}


//============================================================================
//============================================================================
template <class RECIPIE>
Real ComputeParzenError(RECIPIE& recipie)
{

	TICK(dtm);

	UD1("Computing full parzen error (%g)", TOCK(dtm));

	int c=	recipie.CSources();

	ScanVolume *vscan = new ScanVolume[c];

	// bite the bullet and make copies of all of the transformed volumes
	// so that we don't have to constantly recompute the transformation
	// for every probability density estimation
	for (int n=0; n<c; n++){
		TICK(dtmCopy);
		AllocatedCopy(vscan[n],*recipie.PSource(n));
		UD("Time to copy out transformed volume %d (%g)", n, TOCK(dtmCopy));
	}

	Real rError=MeasureFullInverseParzenProbabilityInPlace(c,vscan);

	UD("Done computing full parzen error (%g)", TOCK(dtm));

	delete [] vscan;
	return rError;
}


//============================================================================
//============================================================================
template <class RECIPIE>
void CongealRecipieOutput_Matlab(
	const String& sdir, 
	const String &sflPrefix, 
	RECIPIE& recipie, 
	int nSchedule, 
	int nProgress
){
	SaveMatlab(
		StringF(sdir + sflPrefix + "%03d.%03d.m",nSchedule,nProgress),
		StringF(sflPrefix + "%03d.%03d",nSchedule,nProgress),
		recipie
	);
}


//============================================================================
//============================================================================
template <class RECIPIE>
void CongealRecipieOutput_Error(
	const String& sdir, 
	const String &sflPrefix, 
	RECIPIE& recipie, 
	int nSchedule, 
	int nProgress
){
	String sfl=StringF(sdir + sflPrefix + "%03d.%03d.err",nSchedule,nProgress);
	Real rErr= ComputeParzenError(recipie);
	UD("GOT ERROR %f", rErr);
	String s=StringF("%lf\n",rErr);
	FILE *pfl=fopen(sfl,"w");
	if (!pfl){
		CONGEAL_ERROR("could not open file %s",sfl.VCH());
	}
	else{
		fprintf(pfl,s.VCH());
		fclose(pfl);
	}
}

//============================================================================
//============================================================================
template <class RECIPIE>
void CongealRecipieOutput_SourceLayout(
	const String& sdir, 
	const String &sflPrefix, 
	RECIPIE& recipie, 
	int nSchedule, 
	int nProgress
){
	const int dataMean=ConfigValue("congeal.output.colors.mid",2048);
	const int dataRange=ConfigValue("congeal.output.colors.range",1024);

	UD1("writing out layout of transformed sources");

	for (int dtSlice=0; dtSlice<3; dtSlice++){
		int nSlice=(
			ConfigValue(
				StringF("congeal.output.slice[%d]",dtSlice),
				ConfigValue("congeal.output.slice",.5)) 
		) * recipie.PSource(0)->Size().Dim(dtSlice);

		ScanLayout layoutSources;
		int cLayout=congeal_min(recipie.CSources(),ConfigValue("congeal.output.source.layout.grid",1024));
		int cC = floor(sqrt(cLayout));
		int cR = floor(sqrt(cLayout));

		{
			TICK(dtmWriteVolume);

			for (int n=0; n<cLayout; n++){

				layoutSources.AddSource(
					Aperature(
						Point2D(-50.,-50.),Point2D(1024./cC+50,1024./cR+50),
						ScaleUpto(
							Point2D(1024./cC,1024./cR),
							Slice(
								DimensionType(dtSlice),nSlice,
								(//										LinearInterpolation(
									recipie.PSource(n))))));
			}
			layoutSources.AutoArrange();

			UD("ARRANGE TIME (%g)", TOCK(dtmWriteVolume));
		}

		{
			TICK(dtmWriteVolume);
			String sfl=StringF(sdir + sflPrefix + "%03d.%03d-sourcelayout-dim%d-%03d",nSchedule,nProgress,dtSlice,nSlice);
			WritePGM(
				sfl+".pgm",
				Rasterize(						
					GreyPixelRangeScale(
						dataMean,dataRange,

						Aperature(
							Point2D(0,0),
							Point2D(
								ConfigValue("congeal.output.source.layout.width",512),
								ConfigValue("congeal.output.source.layout.height",512)),
							ScaleUpto(
								Point2D(
									ConfigValue("congeal.output.source.layout.width",512),
									ConfigValue("congeal.output.source.layout.height",512)),
								&layoutSources)))));

			UD("WRITE SOURCE TIME (%g)", TOCK(dtmWriteVolume));
			system(
				StringF(
					ConfigValue(
						StringF("congeal.output.source.layout.postprocess.dim%d",dtSlice),
						ConfigValue("congeal.output.source.layout.postprocess","")),
					sfl.VCH()));
		}
	}
}


//============================================================================
//============================================================================
template <class RECIPIE>
void CongealRecipieOutput_SourceSlices(
	const String& sdir, 
	const String &sflPrefix, 
	RECIPIE& recipie, 
	int nSchedule, 
	int nProgress
){
	const int dataMean=ConfigValue("congeal.output.colors.mid",2048);
	const int dataRange=ConfigValue("congeal.output.colors.range",1024);

	UD1("writing out transformed source slices");
	for (int dtSlice=0; dtSlice<3; dtSlice++){
		int nSlice=(
			ConfigValue(StringF("congeal.output.slice[%d]",dtSlice),ConfigValue("congeal.output.slice",.5)) 
		) * recipie.PSource(0)->Size().Dim(dtSlice);

		int cSources = recipie.CSources();
		for (int nSource=0; nSource<cSources; nSource++){

			TICK(dtmWriteVolume);

			String sfl=StringF(sdir + sflPrefix + "%03d.%03d-source%d-dim%d-%03d",nSchedule,nProgress,nSource,dtSlice,nSlice);

			WritePGM(
				sfl + ".pgm",
				Rasterize(
					Aperature(
						Point2D(0,0),
						Point2D(
							ConfigValue("congeal.output.source.slices.width",512),
							ConfigValue("congeal.output.source.slices.height",512)),
						ScaleUpto(
							Point2D(
								ConfigValue("congeal.output.source.slices.width",512),
								ConfigValue("congeal.output.source.slices.height",512)
							),
							GreyPixelRangeScale(
								dataMean,dataRange,
								Slice(
									DimensionType(dtSlice),nSlice,
									recipie.PSource(nSource)
								))))));

			UD("WRITE SOURCE SLICE TIME (%g)", TOCK(dtmWriteVolume));

			system(
				StringF(
					ConfigValue(
						StringF("congeal.output.source.slices.postprocess.dim%d",dtSlice),
						ConfigValue("congeal.output.source.slices.postprocess","")),
					sfl.VCH()));
		}
	}
}

//============================================================================
//============================================================================
template <class RECIPIE>
void CongealRecipieOutput_AverageSlices(
	const String& sdir, 
	const String &sflPrefix, 
	RECIPIE& recipie, 
	int nSchedule, 
	int nProgress
){
	const int dataMean=ConfigValue("congeal.output.colors.mid",2048);
	const int dataRange=ConfigValue("congeal.output.colors.range",1024);

	for (int dtSlice=0; dtSlice<3; dtSlice++){
		int nSlice=(
			ConfigValue(StringF("congeal.output.slice[%d]",dtSlice),ConfigValue("congeal.output.slice",.5)) 
		) * recipie.PSource(0)->Size().Dim(dtSlice);

		UD1("writing out average slics");
		{
			TICK(dtmWriteVolume);

			String sfl=StringF(sdir + sflPrefix + "%03d.%03d-average-dim%d-%03d",nSchedule,nProgress,dtSlice,nSlice);

			WritePGM(
				sfl + ".pgm",
				Rasterize(
					Aperature(
						Point2D(0,0),
						Point2D(
							ConfigValue("congeal.output.average.slices.width",512),
							ConfigValue("congeal.output.average.slices.height",512)),
						ScaleUpto(
							Point2D(
								ConfigValue("congeal.output.average.slices.width",512),
								ConfigValue("congeal.output.average.slices.height",512)
							),
							GreyPixelRangeScale(
								dataMean,dataRange,
								Slice(
									DimensionType(dtSlice),nSlice,
									(//									LinearInterpolation(
										Average(
											recipie.VSources(),
											recipie.CSources()))))))));

			UD("WRITE AVERAGE SLICES TIME (%g)", TOCK(dtmWriteVolume));

			system(
				StringF(
					ConfigValue(
						StringF("congeal.output.average.slices.postprocess.dim%d",dtSlice),
						ConfigValue("congeal.output.average.slices.postprocess","")),
					sfl.VCH()));
		}
	}
}

//============================================================================
//============================================================================
template <class RECIPIE>
void CongealRecipieOutput_AverageVolume(
	const String& sdir, 
	const String &sflPrefix, 
	RECIPIE& recipie, 
	int nSchedule, 
	int nProgress
){
	UD1("Writing NIFTI average volume ");
	TICK(dtmWriteVolume);

	UD("STARTING WRITE NIFTI TIME (%g)", TOCK(dtmWriteVolume));
	WriteNifti(
		StringF(sdir + sflPrefix + "%03d.%03d.nii",nSchedule,nProgress),
		Rasterize(
			Aperature(
				Point3D(0),Point3D(256),
				Average(
					recipie.VSources(),
					recipie.CSources()))));

	UD("WRITE NIFTI TIME (%g)", TOCK(dtmWriteVolume));
}
//============================================================================
//============================================================================
template <class RECIPIE>
void CongealRecipieOutput(RECIPIE& recipie, int nSchedule, int nProgress)
{

	String sdir = ConfigValue("congeal.output.path","../output/congeal/");
	String sflPrefix = ConfigValue("congeal.output.prefix","out");

	// MATLAB-----------------------------------
	if (ConfigValue("congeal.output.matlab",true)){
		CongealRecipieOutput_Matlab(
			sdir,sflPrefix,recipie,nSchedule,nProgress
		);
	}

	// FULL ERROR -------------------------------
	if (ConfigValue("congeal.output.error",true)){
		CongealRecipieOutput_Error(
			sdir,sflPrefix,recipie,nSchedule,nProgress
		);
	}

	// INPUT LAYOUT -----------------------------
	if (ConfigValue("congeal.output.source.layout",false)){
		CongealRecipieOutput_SourceLayout(
			sdir,sflPrefix,recipie,nSchedule,nProgress
		);
	}


	// SOURCE SLICES ----------------------------
	if (ConfigValue("congeal.output.source.slices",true)){
		CongealRecipieOutput_SourceSlices(
			sdir,sflPrefix,recipie,nSchedule,nProgress
		);
	}

	//----------------------------------------
	// AVERAGE SLICES
	if (ConfigValue("congeal.output.average.slices",true)){
		CongealRecipieOutput_AverageSlices(
			sdir,sflPrefix,recipie,nSchedule,nProgress
		);
	}

	//----------------------------------------
	// AVERAGE VOLUME
	if (ConfigValue("congeal.output.average.volume",true)){
		CongealRecipieOutput_AverageVolume(
			sdir,sflPrefix,recipie,nSchedule,nProgress
		);
	}
}


#endif

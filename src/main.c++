#include "libBaseTypes.h"

// file I/O libraries
#include "libBrainWebIO.h"
#include "libNiftiIO.h"
#include "libPGM.h"

#include "libSourceOperations.h"
#include "libNormalize.h"
#include "SourceRegistry.h"

//#include "libDownsample.h"
#include "CongealGroup.h"
#include "CongealGroupPhased.h"

#include "PaddedImage.h"

#include "libConfiguration.h"

#include "libAssistants.h"

#include "SourceCombine.h"
#include "SourceCombinePointwiseOperator.h"
#include "SourceCombineLayout.h"

#include "SourceAccessorRangeScale.h"

#include "SourceTransformSpatialScale.h"
#include "libMatlabIO.h"

#undef DEBUG 
#define DEBUG 3
#include "libDebug.h"

#include <unistd.h> // for fork()
#include <sys/wait.h>
#include <errno.h>

#define FORK_WRITES 1

typedef NiftiDataVolume ScanVolume;
typedef NiftiPixel ScanPixel;
typedef Source_S2R_Layout_S2R ScanLayout;
//typedef Source_G2R_Layout_G2R ScanLayout;

//============================================================================
//============================================================================
// A congeal recipie for a translate, rotate, warp of a volume
//
// some unused transform options
//		_layer(DataScale,     SourceTransformDataScaleOf,     ScanPixel, 3, Real, INPUT) 
//		_layer(SpatialScale,  SourceTransformSpatialScaleOf,  ScanPixel, 3, Real, INPUT) 
//		_layer(DataScale,     SourceTransformDataScaleOf,     ScanPixel, 3, Real, INPUT) 

#define my_mid_layers(_layer)																									\
		_layer(Translate0,    SourceTransformTranslateOf,     ScanPixel, 3, Real, INPUT) \
		_layer(Rotate,        SourceTransformRotateOf,        ScanPixel, 3, Real, Translate0) \
		_layer(Translate1,    SourceTransformTranslateOf,     ScanPixel, 3, Real, Rotate) \
		_layer(Scale,         SourceTransformScaleOf,         ScanPixel, 3, Real, Translate1) \
		_layer(Warp0, SourceTransformWarpOf, ScanPixel, 3, Real,Scale) \
		_layer(Warp1, SourceTransformWarpOf, ScanPixel, 3, Real,Warp0)	\
		_layer(Warp2, SourceTransformWarpOf, ScanPixel, 3, Real,Warp1)	

#define my_last_layer(_layer)																									\
	_layer(Warp3, SourceTransformWarpOf, ScanPixel, 3, Real,Warp2)

MAKE_RECIPIE(
	Recipie_G3R_All_G3I,
	ScanVolume,
	my_mid_layers,
	my_last_layer
);

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
void WriteRecipie(RECIPIE& recipie, int nSchedule, int nProgress)
{

	String sdir = ConfigValue("congeal.output.path","../output/congeal/");
	String sflPrefix = ConfigValue("congeal.output.prefix","out");


	// MATLAB-----------------------------------
	SaveMatlab(
		StringF(sdir + sflPrefix + "%03d.%03d.m",nSchedule,nProgress),
		StringF(sflPrefix + "%03d.%03d",nSchedule,nProgress),
		recipie
	);

	// FULL ERROR -----------------------------------
	{
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

	//----------------------------------------
	UD1("writing out overview congealed volumes");
	{
		const int dataMean=ConfigValue("congeal.output.colors.mid",2048);
		const int dataRange=ConfigValue("congeal.output.colors.range",1024);

		for (int dtSlice=0; dtSlice<3; dtSlice++){
			int nSlice=ConfigValue("congeal.output.slice",.5) * recipie.PSource(0)->Size().Dim(dtSlice);

			ScanLayout layoutInputs;
			int cLayout=congeal_min(recipie.CSources(),ConfigValue("congeal.output.sourcegrid",1024));
			int cC = floor(sqrt(cLayout));
			int cR = floor(sqrt(cLayout));

			{
				TICK(dtmWriteVolume);

				for (int n=0; n<cLayout; n++){

					layoutInputs.AddSource(
						ScaleTo(
							Point2D(1024./cC,1024./cR),
							Aperature(
								Point2D(-50.,-50.),Point2D(350.,350.),
								ScaleTo(
									Point2D(300.,300.),
									Slice(
										DimensionType(dtSlice),nSlice,
										(//										LinearInterpolation(
											recipie.PSource(n)))))));
				}
				layoutInputs.AutoArrange();

				UD("ARRANGE TIME (%g)", TOCK(dtmWriteVolume));
			}

			{
				TICK(dtmWriteVolume);
				WritePGM(
					StringF(sdir + sflPrefix + "%03d.%03d-inputs%d-%03d.pgm",nSchedule,nProgress,dtSlice,nSlice),
					Rasterize(						
						GreyPixelRangeScale(
							dataMean,dataRange,
							ScaleTo(
								Point2D(512,512),
								&layoutInputs))));

				UD("WRITE INPUT TIME (%g)", TOCK(dtmWriteVolume));
			}

			{
				TICK(dtmWriteVolume);

				WritePGM(
					StringF(sdir + sflPrefix + "%03d.%03d-average%d-%03d.pgm",nSchedule,nProgress,dtSlice,nSlice),
					Rasterize(
						ScaleUpto(
							Point2D(
								ConfigValue("congeal.output.average.width",512),
								ConfigValue("congeal.output.average.height",512)
							),
							GreyPixelRangeScale(
								dataMean,dataRange,
								Slice(
									DimensionType(dtSlice),nSlice,
									(//									LinearInterpolation(
										Average(
											recipie.VSources(),
											recipie.CSources())))))));

				UD("WRITE AVERAGE TIME (%g)", TOCK(dtmWriteVolume));
			}
		}
	}


	UD1("Writing NIFTI average volume ");
	{
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

}



//============================================================================
//============================================================================
void LoadAndScaleSources(int c, ScanVolume* vscan, const String& sConfig)
{
	D("loading and downsampling sources");

	String sflInputfiles = ConfigValue("congeal.inputfiles.list","");
	String sflfmtInput = ConfigValue("congeal.inputfiles.pattern","%d.mha");

	bool bFromFile = sflInputfiles!="";

	FILE *pfl=NULL;
	if(bFromFile){
		pfl=fopen(sflInputfiles,"r");
		CONGEAL_ASSERTf(pfl,"could not open file: " + sflInputfiles);
	}

	for (int n=0; n<c; n++){
		String sInputFormat = ConfigValue(
			StringF("congeal.inputfile[%d].format",n), 
			ConfigValue("congeal.inputfile.format","brainweb"));

		String sflDefault="";
		if (bFromFile){
			char sT[4096]={'\0'};
			fgets(sT,sizeof(sT)-1,pfl);
			sT[strlen(sT)-1]=0; // trim whitespace. NOTE: Ugly. TODO: fix this
			sflDefault = sT;
		}
		else{
			sflDefault=StringF(sflfmtInput,n);
		}

		String sfl=ConfigValue(
			StringF("congeal.inputfile[%d]",n),
			sflDefault
		);

		if (sInputFormat == "brainweb"){
			CONGEAL_ERROR("not currently supported");
/*
			vscan[n]=DereferencePointer(
				RasterizeDownsample(			
					ConfigValue(sConfig+".downsample",0), 
					ReadBrainWeb(sfl)
				)
			);
*/
		}
		else if (sInputFormat == "nifti"){
			vscan[n]=DereferencePointer(
				RasterizeDownsample(			
					ConfigValue(sConfig+".downsample",0), 
					ReadNifti(sfl)
				)
			);

		}
	}

	if (bFromFile){
		fclose(pfl);
	}
}


//============================================================================
//============================================================================
void IncreaseWarpfieldControlPointResolution(
	int c,Recipie_G3R_All_G3I& recipie, const String& sConfig
){

	D("increasing resolution of warpfield control points");
	recipie.PrepareForAccess();
	for (int n=0; n<c; n++){
		IncreaseWarpfieldControlPointResolution(
			recipie.LayerWarp0(n),
			PointOf<3,Real>(ConfigValue(sConfig + ".warpfield[0].size", 1)),
			InterpolationType(ConfigValue(sConfig + ".warpfield[0].interpolation", 3)));

		IncreaseWarpfieldControlPointResolution(
			recipie.LayerWarp1(n),
			PointOf<3,Real>(ConfigValue(sConfig + ".warpfield[1].size", 1)),
			InterpolationType(ConfigValue(sConfig + ".warpfield[1].interpolation", 3)));

		IncreaseWarpfieldControlPointResolution(
			recipie.LayerWarp2(n),
			PointOf<3,Real>(ConfigValue(sConfig + ".warpfield[2].size", 1)),
			InterpolationType(ConfigValue(sConfig + ".warpfield[2].interpolation", 3)));

		IncreaseWarpfieldControlPointResolution(
			recipie.LayerWarp3(n),
			PointOf<3,Real>(ConfigValue(sConfig + ".warpfield[3].size", 1)),
			InterpolationType(ConfigValue(sConfig + ".warpfield[3].interpolation", 3)));
	}
}

//============================================================================
//============================================================================
void UpdateScheduleRegistrationParameters(
	Recipie_G3R_All_G3I& recipie, const String& sConfig
){

	D("set up schedule registration parameters");
	bool b=ConfigValue(sConfig+".optimize.affine", true);
	recipie.OptimizeLayerTranslate0(ConfigValue(sConfig+".optimize.translate[0]", b));
	recipie.OptimizeLayerRotate(ConfigValue(sConfig+".optimize.rotate", b));
	recipie.OptimizeLayerTranslate1(ConfigValue(sConfig+".optimize.translate[1]", b));
	recipie.OptimizeLayerScale(ConfigValue(sConfig+".optimize.scale", b));

//	recipie.OptimizeLayerDataScale(ConfigValue(sConfig+".optimize.contrast", true));
//	recipie.OptimizeLayerSpatialScale(ConfigValue(sConfig+".optimize.spatialscale", true));

//	recipie.OptimizeLayerWarp(ConfigValue(sConfig+".optimize.warp", true));
	recipie.OptimizeLayerWarp0(ConfigValue(sConfig+".optimize.warp[0]", true));
	recipie.OptimizeLayerWarp1(ConfigValue(sConfig+".optimize.warp[1]", true));
	recipie.OptimizeLayerWarp2(ConfigValue(sConfig+".optimize.warp[2]", true));
	recipie.OptimizeLayerWarp3(ConfigValue(sConfig+".optimize.warp[3]", true));
}


//============================================================================
//============================================================================
void Congeal(
	int nSchedule, Recipie_G3R_All_G3I& recipie, const String& sConfig
){

	D("congealing");

	CongealGroupPhasedOf<Recipie_G3R_All_G3I> cg(&recipie);

	int cSamples=ConfigValue(
		sConfig + ".optimize.samples",
		ConfigValue("congeal.optimize.samples",500));

	int cIterations=ConfigValue(
		sConfig + ".optimize.iterations",
		ConfigValue("congeal.optimize.iterations",1000));

	int cProgress=ConfigValue(
		sConfig + ".optimize.progresspoints",
		ConfigValue("congeal.optimize.progresspoints",10))+1;

	int cIterationsPerProgress=cIterations/cProgress;
	for (int nProgress=0; nProgress<cProgress; nProgress++){
		if (nProgress>0){
			cg.StatisticalPhasedCongeal(
				cSamples,	
				.0001, 
				cIterations,
				(nProgress-1)*cIterationsPerProgress,
				(nProgress)*cIterationsPerProgress-1);
		}

#if FORK_WRITES
		if (fork()==0){
#endif
			//we're in the child
			UD(">>> WRITING: %d.%d", nSchedule,nProgress);
			recipie.PrepareForAccess();
//			D(recipie.Describe());
			WriteRecipie(recipie, nSchedule,nProgress);
			UD("<<< WRITING: %d.%d", nSchedule,nProgress);
#if FORK_WRITES
			exit(1);
		}
		else{
			// reap any writing processes which have finished
			int zReturn;
			do { ;} while (wait3(&zReturn,WNOHANG,0)!=0);
		}
#endif
	}
}


//============================================================================
//============================================================================
bool LoadCachedRecipie(const char *sflCache, Recipie_G3R_All_G3I& recipie, ScanVolume *vscan)
{
	Stream st;
	if (!ReadStream(st,sflCache)){
		return false;
	}
	recipie.Deserialize(st);
	return true;
}

//============================================================================
//============================================================================
void SaveCachedRecipie(const char *sflCache, Recipie_G3R_All_G3I& recipie)
{
	Stream st;
	st.EnsureSpace(recipie.CSources() * 100*1024); // guess 100k each?
	recipie.Serialize(st);
	WriteStream(st, sflCache);
}

//============================================================================
//============================================================================
int LinesInFile(const char* sfl)
{
	FILE *pfl=fopen(sfl,"r");
	if (!pfl){
		return 0;
	}
	int c=0;
	char s[4096]={'\0'};
	while(fgets(s,sizeof(s)-1,pfl) != NULL){
		c++;
	}

	fclose(pfl);
	return c;
}


//============================================================================
//============================================================================
void CongealSchedules()
{
	int c=ConfigValue("congeal.inputfiles",4);
	
	if (c==0){
		c=LinesInFile(ConfigValue("congeal.inputfiles.list",""));
	}

	ScanVolume vscan[c];
	Recipie_G3R_All_G3I recipie(c,vscan);

	P(DescribeConfiguration());

	String sdir = ConfigValue("congeal.output.path","../output/congeal/");
	String sflPrefix = ConfigValue("congeal.output.prefix","../output/congeal/out");

	system("cp '" + GetConfigFile() + "' '" + sdir + sflPrefix + ".config'");

	int cSchedules=ConfigValue("congeal.schedules",3);
	for (int nSchedule=0; nSchedule<cSchedules; nSchedule++){
		String sConfig=String("congeal.schedule[") + nSchedule + "]";
		UD("CONGEALING SCHEDULE %d/%d", nSchedule, cSchedules);

		bool bCache=ConfigValue(sConfig + ".cache",	ConfigValue("congeal.cache",true));
		String sflCache = StringF(sdir + sflPrefix + "%03d.cache",nSchedule);

		UD("Checking cache");
		if (bCache && LoadCachedRecipie(sflCache, recipie, vscan)){
			UD("Using cached congealing results for recipie for schedule %d/%d", nSchedule, cSchedules);
		}
		else {
			// do congealing steps
			LoadAndScaleSources(c,vscan,sConfig);
			UpdateScheduleRegistrationParameters(recipie,sConfig);
			IncreaseWarpfieldControlPointResolution(c,recipie,sConfig);
//			IncreaseSpatialScaleResolution(c,recipie,sConfig);
			Congeal(nSchedule,recipie,sConfig);

			UD("Caching congealed recipie for later use");
			SaveCachedRecipie(sflCache,recipie);
		}
	}

	// only wait for the last child, assuming they all complete 
	// in the order they were launched.
	int zReturn;
	do {;} while(wait(&zReturn)!=-1 && errno !=ECHILD);
}



//============================================================================
//============================================================================
void ApplyCongeal()
{
	String sflCache=ConfigValue("apply.cachefile","../output/congeal/out000.cache");
	int nTransform=ConfigValue("apply.transform",0);

	String sdir = ConfigValue("apply.output.path","../output/congeal/");
	String sflPrefix = ConfigValue("apply.output.prefix","out");
	ScanVolume scan;

	Recipie_G3R_All_G3I recipie(1,&scan);

	if (ConfigValue("apply.input","file") == String("file")){
		String sflSource=ConfigValue("apply.inputfile","../input/in.nifti");
		String sInputFormat=ConfigValue("apply.inputfile.format","nifti");
		if (sInputFormat == "nifti"){
			scan=DereferencePointer(
				RasterizeDownsample(			
					ConfigValue("apply.inputfile.downsample",0), 
					ReadNifti(sflSource)
				)
			);
		}
	}
	else{
		int cSize=ConfigValue("apply.input.precomputed.size",256);
		int cX=ConfigValue("apply.input.precomputed.x",cSize);
		int cY=ConfigValue("apply.input.precomputed.y",cSize);
		int cZ=ConfigValue("apply.input.precomputed.z",cSize);
		char chDir=ConfigValue("apply.input.precomputed.dimension",'x');
		int nDim=chDir=='x'?0:(chDir=='y'?1:2);

		scan.Allocate(Point3D(cX,cY,cZ));
		
		forpoint(Point3D,pt,0,scan.Size()){
			SetPoint(scan,pt,pt.Dim(nDim));
		}
	}

	scan.PrepareForAccess();

	Stream st;
	if (!ReadStream(st,sflCache)){
		CONGEAL_ERROR("cache file not found " + sflCache);
	}

	// HACK: because cache file contain an enitre group of transform-sets
	// we 'eat up' the prior sets on the stream by repeateded deserializing
	for (int n=0; n<=nTransform; n++){
		recipie.Deserialize(st);
	}
	recipie.PrepareForAccess();


	const int dataMean=ConfigValue("apply.output.colors.mid",2048);
	const int dataRange=ConfigValue("apply.output.colors.range",1024);

	// write out three slices
	for (int dtSlice=0; dtSlice<3; dtSlice++){
		int nSlice=(
			ConfigValue("apply.output.slice",.5) 
			* recipie.PSource(0)->Size().Dim(dtSlice)
		);

		WritePGM(
			StringF(sdir + sflPrefix + "%d-%03d.pgm",dtSlice,nSlice),
			Rasterize(
				ScaleUpto(
					Point2D(
						ConfigValue("apply.output.width",512),
						ConfigValue("apply.output.height",512)
					),
					GreyPixelRangeScale(
						dataMean,dataRange,
						Slice(
							DimensionType(dtSlice),nSlice,
							recipie.PSource(0)
						)
					)
				)
			)
		);
	}

	// write out the whole volume
	{
		TICK(dtmWriteVolume)

		UD("STARTING WRITE NIFTI TIME (%g)", TOCK(dtmWriteVolume));
		WriteNifti(
			StringF(sdir + sflPrefix + ".nii"),
			Rasterize(
				recipie.PSource(0)
			)
		);
		UD("WRITE NIFTI TIME (%g)", TOCK(dtmWriteVolume));
	}

}





//============================================================================
//============================================================================
int main(int cCLA, char *vsCLA[]){

	if (cCLA>1){
		int nCLA=1;
		if (vsCLA[nCLA][0] != '-'){
			SetConfigFile(vsCLA[1]);
			nCLA++;
		}
		
		for (; nCLA<cCLA-1; nCLA++){
			String sCmd=ConfigValue(String("command[") + vsCLA[nCLA] + "]","");
			if (sCmd.C() == 0){
				CONGEAL_ERROR("unknown command line option " + String(vsCLA[nCLA]));
			}
			SetConfigValue(sCmd,vsCLA[nCLA+1]);
		}
	}
	
	ConfigValue("test","congeal");
	int cStaticObjects=ObjectsOutstanding();
	{
		String sTest=ConfigValue("test","congeal");

		if (sTest==String("congeal")){
			CongealSchedules();
		}
		else if (sTest==String("apply")){
			ApplyCongeal();
		}
		else{
			CONGEAL_ERROR("Unknown test:"+ sTest +"--");
		}

	}
	D("Objects leaked %d", ObjectsOutstanding()	-cStaticObjects );

	CONGEAL_ASSERTf(	
		ObjectsOutstanding() == cStaticObjects, 
		"Objects leaked %d", 
		ObjectsOutstanding()	-cStaticObjects 
	);
}







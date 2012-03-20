#include "libBaseTypes.h"

#include "libBrainWebIO.h"
#include "libNiftiIO.h"
#include "libPGM.h"

#include "libSourceOperations.h"
#include "libNormalize.h"
#include "SourceRegistry.h"
#include "CongealGroup.h"
#include "CongealGroupPhased.h"

#include "libConfiguration.h"
#include "libAssistants.h"
#include "SourceCombine.h"
#include "SourceCombinePointwiseOperator.h"
#include "SourceAccessorRangeScale.h"
#include "SourceTransformSpatialScale.h"

#include <unistd.h> // for fork()
#include <sys/wait.h>
#include <errno.h>
#include "CongealRecipieOutput.h"

#ifndef FORK_OUTPUT_WRITES 
#define FORK_OUTPUT_WRITES 1
#endif

#undef DEBUG 
#define DEBUG 3
#include "libDebug.h"


//============================================================================
//============================================================================
// 
// SET UP OUR RECIPIE FOR CONGEALING
// 
// This can be tuned depending upon the current research programme
// Currently configured for aA congeal recipie for a translate, rotate, 
// and three levels of warp of a volume
//
// See Recipie.h for more detail about creating Recipies.
//
// NOTE: because c++ is not introspective you need to manually make sure 
// that changes to this structure are reflected in 
// UpdateScheduleRegistrationParameters() and for warps in 
// IncreaseWarpfieldControlPointResolution()
// 
// some unused transform options
//		_layer(DataScale,     SourceTransformDataScaleOf,     ScanPixel, 3, Real, INPUT) 
//		_layer(SpatialScale,  SourceTransformSpatialScaleOf,  ScanPixel, 3, Real, INPUT) 
//		_layer(DataScale,     SourceTransformDataScaleOf,     ScanPixel, 3, Real, INPUT) 
//
// 
#define congeal_mid_layers(_layer)                                                      \
	_layer(DataScale,     SourceTransformDataScaleOf,     ScanPixel, 3, Real, INPUT)      \
	_layer(Translate0,    SourceTransformTranslateOf,     ScanPixel, 3, Real, DataScale)  \
	_layer(Rotate,        SourceTransformRotateOf,        ScanPixel, 3, Real, Translate0) \
	_layer(Translate1,    SourceTransformTranslateOf,     ScanPixel, 3, Real, Rotate)     \
	_layer(Scale,         SourceTransformScaleOf,         ScanPixel, 3, Real, Translate1) \
	_layer(Warp0,         SourceTransformWarpOf,          ScanPixel, 3, Real, Scale)      \
	_layer(Warp1,         SourceTransformWarpOf,          ScanPixel, 3, Real, Warp0)      \
	_layer(Warp2,         SourceTransformWarpOf,          ScanPixel, 3, Real, Warp1)

#define congeal_last_layer(_layer)																									    \
	_layer(Warp3,         SourceTransformWarpOf,          ScanPixel, 3, Real, Warp2)

MAKE_RECIPIE(
	Recipie_G3R_All_G3I,
	ScanVolume,
	congeal_mid_layers,
	congeal_last_layer
);



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
	 BrainWeb I/O has is not currently supported 
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

#if FORK_OUTPUT_WRITES
		if (fork()==0){
#endif
			//we're in the child
			UD(">>> WRITING: %d.%d", nSchedule,nProgress);
			recipie.PrepareForAccess();
//			D(recipie.Describe());
			CongealRecipieOutput(recipie, nSchedule,nProgress);
			UD("<<< WRITING: %d.%d", nSchedule,nProgress);
#if FORK_OUTPUT_WRITES
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
double *AllocateHist(
	int c
){
	double* vrHist = new double [c];
	for (int n=0; n<c; n++){
		vrHist[n]=0;
	}
	return vrHist;
}

double *ComputeHist(
	int c, int nSource, Recipie_G3R_All_G3I& recipie, double *vrHistAccumulate
){
	double* vrHist = AllocateHist(c);
	
	NiftiPixel pxl;
	forpoint(Point3D,pt,0,recipie.PSource(nSource)->Size()){
		recipie.PSource(nSource)->GetPoint(pxl,pt);
		vrHist[pxl]++;
		vrHistAccumulate[pxl]++;
	}

	return vrHist;
}

void NormalizeHist(
	int c,double *vrHist
){
	double r=0;
	for (int n=0; n<c; n++){
		r+=vrHist[n];
	}
	for (int n=0; n<c; n++){
		vrHist[n]/=r;
	}
}

void FreeHist(
	double *vrHist
){
	delete [] vrHist;
}


//============================================================================
//============================================================================
double KLDivergence(
	int c,double *vr1,double *vr2
){
	double r = 0;
	for (int n=0; n<c; n++){
		if (vr1[n]!= 0 && vr2[n]!= 0){
			r+=vr1[n] * log2(vr1[n]/vr2[n]);
		}
	}
	return r;
}

double LinInterp(double r,double r1,double r2){
	return (1.0-r)*r1 + r * r2;
}

//============================================================================
//============================================================================
void NormalizeBrightness(
	Recipie_G3R_All_G3I& recipie, const String& sConfig
){
	D("Normalizing");
	recipie.PrepareForAccess();

	int c=256;
	double* vrHist = AllocateHist(c);

	// initalize global histogram to 1, instead of 0 to stabilize KL 
	for (int n=0; n<c; n++){
		vrHist[n]=1;
	}

	int cSource = recipie.CSources();
	double** vvrHist = new double* [c];
	for (int nSource=0; nSource<cSource; nSource++){
		vvrHist[nSource] = ComputeHist(	c,nSource,recipie,vrHist );
		NormalizeHist(c, vvrHist[nSource]);
	}
	NormalizeHist(c, vrHist);

	double* vrScale = new double[cSource];
	double* vrKLDivergence = new double[cSource];
	double* vrT = AllocateHist(c);
	for (int nSource=0; nSource<cSource; nSource++){
		bool b=false;
		for (double r = .1 ; r< 5; r+=.01){
			for (int n=0; n<c; n++){
				
				double rBefore  = 0.;
				double rAfter   = 0.;
				double rScaled  = r*n;

				int nBefore    = int(rScaled);
				int nAfter     = int(rScaled+1);

				if (nBefore < c){
					rBefore = vvrHist[nSource][nBefore];
					if (nAfter < c){
						rAfter = vvrHist[nSource][nAfter];
					}
				}

				vrT[n] = LinInterp(rScaled - double(nBefore), rBefore, rAfter);
			}

			NormalizeHist(c, vrT);
			double rKLDivergence = KLDivergence(c, vrT,vrHist);
			if (!b || vrKLDivergence[nSource] >  rKLDivergence){
				b=true;
				vrKLDivergence[nSource] = rKLDivergence;
				vrScale[nSource] = 1./r;
			}
		}
	}

	FreeHist(vrT);
	FreeHist(vrHist);
	for (int nSource=0; nSource<cSource; nSource++){
		FreeHist(vvrHist[nSource]);
	}
	delete [] vvrHist;

	for (int nSource=0; nSource<cSource; nSource++){
		recipie.LayerDataScale(nSource).SetSlopeIntercept(vrScale[nSource],0);
	}

	for (int nSource=0; nSource<cSource; nSource++){
		UD("NORMALIZE %d: %g", nSource, vrScale[nSource]);
	}
	delete [] vrScale;

	D("DONE normalizing");
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

	P("%s",DescribeConfiguration().VCH());

	String sdir = ConfigValue("congeal.output.path","../output/congeal/");
	String sflPrefix = ConfigValue("congeal.output.prefix","out");

	system("cp '" + GetConfigFile() + "' '" + sdir + sflPrefix + ".config'");

	int cSchedules=ConfigValue("congeal.schedules",3);
	for (int nSchedule=0; nSchedule<cSchedules; nSchedule++){
		String sConfig=String("congeal.schedule[") + nSchedule + "]";
		UD("CONGEALING SCHEDULE %d/%d", nSchedule, cSchedules);

		bool bCache=ConfigValue(sConfig + ".cache",	ConfigValue("congeal.cache",true));
		String sflCache = StringF(sdir + sflPrefix + "%03d.cache",nSchedule);

		UD("Checking cache");

		if (bCache && LoadCachedRecipie(sflCache, recipie, vscan)){
			// if found, load from cache
			UD("Using cached congealing results for recipie for schedule %d/%d", nSchedule, cSchedules);
		}
		else {
			// otherwise do congealing steps
			LoadAndScaleSources(c,vscan,sConfig);
			if (nSchedule == 0){
				NormalizeBrightness(recipie,sConfig);
			}
			UpdateScheduleRegistrationParameters(recipie,sConfig);
			IncreaseWarpfieldControlPointResolution(c,recipie,sConfig);
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
		// if the first arguemnt does not begin with "-" its presumed to be
		// the path to a congfig file to use
		if (vsCLA[nCLA][0] != '-'){
			SetConfigFile(vsCLA[1]);
			nCLA++;
		}
		
		// examine the command line arguments and set any specificed parameters
		// see parameter: command[*]  for details
		for (; nCLA<cCLA-1; nCLA++){
			String sCmd=ConfigValue(String("command[") + vsCLA[nCLA] + "]","");
			if (sCmd.C() == 0){
				CONGEAL_ERROR("unknown command line option " + String(vsCLA[nCLA]));
			}
			SetConfigValue(sCmd,vsCLA[nCLA+1]);
		}
	}
	
	// do some memory object tracking
	int cStaticObjects=ObjectsOutstanding();
	{
		// run the operation
		String sOperation=ConfigValue("operation","congeal");

		if (sOperation==String("congeal")){
			CongealSchedules();
		}
		else if (sOperation==String("apply")){
			ApplyCongeal();
		}
		else{
			CONGEAL_ERROR("Unknown operation: '"+ sOperation +"'");
		}
	}

	D("Objects leaked %d", ObjectsOutstanding()	-cStaticObjects );


	if (ObjectsOutstanding() != cStaticObjects){
		P("Warnining %d objects leaked", ObjectsOutstanding()	-cStaticObjects);
	}
}








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
#include "libBaseTypes.h"

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



template<class RECIPIE>
Real CongealGroupPhasedOf<RECIPIE>::m_vrFastGaussian[FASTGAUSSIAN_TABLESIZE];

//============================================================================
// DistributionField10
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	DistributionField10,
	SourceAccessorDistributionFieldOf,
	typename DistributionFieldInfo<typename SOURCE::type_data>::type,
	SOURCE::type_dimensionality,
	typename SOURCE::type_precision,
	{ psrcOut->SetSigmaParzen(10.); }
);

//============================================================================
// DistributionField10
//============================================================================
MAKE_COMBINE_ASSISTANT(
	AverageDistributionField10,
	SourceCombinePointwiseOperatorOf,
	OperatorAverageOf<typename DistributionFieldInfo<typename SOURCE::type_data>::type COMMA typename DistributionFieldInfo<typename SOURCE::type_data>::type>,
	TypeOfDimensionality(DistributionFieldInfo<typename SOURCE::type_data>::type),
	TypeOfPrecision(DistributionFieldInfo<typename SOURCE::type_data>::type),
	{}
);


//============================================================================
//============================================================================
template <class RECIPIE>
void WriteRecipie(RECIPIE& recipie, int nSchedule, int nProgress)
{

	String sdir = ConfigValue("congeal.output.path","../output/congeal/");
	String sflPrefix = ConfigValue("congeal.output.prefix","out");

	SaveMatlab(
		StringF(sdir + sflPrefix + "%03d.%03d.m",nSchedule,nProgress),
		StringF(sflPrefix + "%03d.%03d",nSchedule,nProgress),
		recipie
	);

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


	{
		TICK(dtmWriteVolume);

		UD("STARTING WRITE NIFTI TIME (%g)", TOCK(dtmWriteVolume));
		WriteNifti(
			StringF(sdir + sflPrefix + "%03d.nii",nSchedule),
			Rasterize(
				Average(
					recipie.VSources(),
					recipie.CSources())));

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
/*
void IncreaseWarpfieldControlPointResolutionX(
	int c,Recipie_G3R_All_G3I& recipie, const String& sConfig
){

	D("increasing resolution of warpfield control points");
	recipie.PrepareForAccess();
	for (int n=0; n<c; n++){
		recipie.LayerWarp(n).SetWarpControlPoints(
			RasterizeInto(
				HandoffPointer(new WarpControlPoints3D),
				ScaleTo(
					PointOf<3,Real>(ConfigValue(sConfig + ".warpfield.size", 1)),
					recipie.LayerWarp(n).GetPInterpolatedWarpControlPoints())));
		recipie.LayerWarp(n).SetWarpfieldInterpolation(
			InterpolationType(ConfigValue(sConfig + ".warpfield.interpolation", 3)));
	}
}
*/

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
template <class SOURCE_IN, class SOURCE_OUT>
void XCopyInto(
	SOURCE_OUT *psrcOut,
	SOURCE_IN *psrcIn,
	bool bPrepared=false
){
	ClaimPointer(psrcIn);
	ClaimPointer(psrcOut);
	if (!bPrepared){
		psrcIn->PrepareForAccess();
	}
	typedef typename SOURCE_OUT::type_point POINT;
	typedef typename SOURCE_IN::type_data DATA_IN;
	typedef typename SOURCE_OUT::type_data DATA_OUT;

	forpoint(POINT,pt,0,psrcOut->Size()){
		DATA_IN dataIn;
		DATA_OUT dataOut;

		GetPoint(*psrcIn,dataIn,pt);
		dataOut=dataIn;
		SetPoint(*psrcOut,pt,dataOut);
	}
	ReleasePointer(psrcIn);
	ReleasePointer(psrcOut);
}


//============================================================================
//============================================================================
/*
void IncreaseSpatialScaleResolution(
	int c,Recipie_G3R_All_G3I& recipie, const String& sConfig
){
	D("increasing resolution spatial scale coefficients");
	recipie.PrepareForAccess();
	int cSize=ConfigValue(sConfig + ".spatialscale.size", 2);

	for (int n=0; n<c; n++){
		SourceMemoryOf<Real,3> *psm = Fill(
			0.,
			Allocate(
				PointOf<3,int>(cSize),
				new SourceMemoryOf<Real,3>()));
			CopyInto(
				Aperature(
					PointOf<3,int>(0),
					recipie.LayerSpatialScale(n).GetPCoefficients()->Size(),
					psm),
				recipie.LayerSpatialScale(n).GetPCoefficients());

		recipie.LayerSpatialScale(n).SetCoefficients(HandoffPointer(psm));
	}
}
*/

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

	// allocate spatialscale
/*
	for (int n=0; n<c; n++){ 
		recipie.LayerSpatialScale(n).AllocateCoefficients(PointOf<3,Real>(1));
		Set(*recipie.LayerSpatialScale(n).GetPCoefficients(),0,0,0,0.);
	}
*/

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

	String sflSource=ConfigValue("apply.inputfile","../input/in.nifti");
	String sInputFormat=ConfigValue("apply.inputfile.format","nifti");

	String sdir = ConfigValue("apply.output.path","../output/congeal/");
	String sflPrefix = ConfigValue("apply.output.prefix","out");
	ScanVolume scan;

	Recipie_G3R_All_G3I recipie(1,&scan);


	if (sInputFormat == "nifti"){
		scan=DereferencePointer(
			RasterizeDownsample(			
				ConfigValue("apply.inputfile.downsample",0), 
				ReadNifti(sflSource)
			)
		);
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




#define speed_mid_layers(_layer)																									\
	_layer(Translate0,    SourceTransformTranslateOf,     ScanPixel, 3, Real, INPUT) \
		_layer(Rotate,        SourceTransformRotateOf,        ScanPixel, 3, Real, Translate0) \
		_layer(Translate1,    SourceTransformTranslateOf,     ScanPixel, 3, Real, Rotate) \
		_layer(Scale,         SourceTransformScaleOf,         ScanPixel, 3, Real, Translate1) 


#define speed_last_layer(_layer)																									\
	_layer(Warp, SourceTransformWarpOf, ScanPixel, 3, Real,Scale)

MAKE_RECIPIE(
	Recipie_G3R_Speed_G3I,
	ScanVolume,
	speed_mid_layers,
	speed_last_layer
);




							
//============================================================================
//============================================================================
void TestSpeed()
{
	int c=1;
	if (c==0){
		c=LinesInFile(ConfigValue("congeal.inputfiles.list",""));
	}
	
	ScanVolume vscan[c];


	String sConfig=String("congeal.schedule[0]");
	LoadAndScaleSources(c,vscan,sConfig);

	Recipie_G3R_Speed_G3I recipie(c,vscan);
	for (int n=0; n<c; n++){ 
		recipie.LayerWarp(n).AllocateWarpfield(PointOf<3,Real>(3),Cubic);
		Set(*recipie.LayerWarp(n).GetPWarpControlPoints(),0,0,0,Point3DReal(0.));
	}
	recipie.PrepareForAccess();

	ImageOf<ScanPixel,3> gv;
	gv.Allocate(recipie.Size());
	gv.PrepareForAccess();

	int cSamples = 10000000;
	Point3D* vpt= new Point3D[cSamples];

	D("GOT POINTS");
	for (int nSample=0; nSample<cSamples; nSample++){
		vpt[nSample]=NewRandomPoint(gv.Size());
	}
	D("GOT POINTS");
	TICK(dtmOptimize);

	int z=0;
	ScanPixel pxl;
	for (int nSample=0; nSample<cSamples; nSample++){
//		gv.GetPoint(pxl,vpt[nSample]);
			::GetPoint(*(recipie.PSource(0)),	pxl,vpt[nSample]);
//		recipie.PSource(0)->GetPoint(pxl,vpt[nSample]);
		z+=pxl;
	}

	UD("SPEED TEST = %g (%g)", TOCK(dtmOptimize), Real(cSamples)/TOCK(dtmOptimize));
	D("%d",z);
}


// JSD
void TestInterpolation()
{
	WritePGM(
		"out.pgm",
		Rasterize(
			GreyPixelRangeScale(
				1024,2048,
				Slice(
					DimensionZ,120,
					ScaleBy(
						Point3DReal(2.7),
						Rotate(
							28,37,53,
							ReadNifti("../input/NIH/uncompressed/deface_1453_V4_t1w.nii")))))));
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

//			TestInterpolation(); if(0)
		
		if (sTest==String("speed")){
			TestSpeed();
		}
		else if (sTest==String("congeal")){
			CongealSchedules();
		}
		else if (sTest==String("apply")){
			ApplyCongeal();
		}
		else{
			CONGEAL_ERROR("Unknown test:"+ sTest +"--");
		}

	}
	D("Objects leaked %d", 
		ObjectsOutstanding()	-cStaticObjects );

	CONGEAL_ASSERTf(	
		ObjectsOutstanding() == cStaticObjects, 
		"Objects leaked %d", 
		ObjectsOutstanding()	-cStaticObjects 
	);
}






template <class T>
String ToString(const T& t)
{
	return t.Describe();
}

String ToString(const Real& t)
{
	return String(t);
}

String ToString(const int& t)
{
	return String(t);
}


template<class SOURCE>
SOURCE* PrintImage(SOURCE* psrc)
{
	ClaimPointer(psrc);
	typedef typename SOURCE::type_point POINT;
	const int DIMENSIONALITY=SOURCE::type_dimensionality;
	typedef typename SOURCE::type_data DATA;

	psrc->PrepareForAccess();

	D(psrc->Describe());

	forpoint(POINT,pt,0.,psrc->Size()){
		DATA data;
		GetPoint(*psrc,data,pt);
		PX(ToString(data) + "\t");
		if (pt.Dim(0) == psrc->Size().Dim(0)-1){
			for (int n=0; n<DIMENSIONALITY; n++){
				if (pt.Dim(n) == psrc->Size().Dim(n)-1){
					PX("\n");
				}
			}
		}
	}
	ReleasePointer(psrc);
	return psrc;
}



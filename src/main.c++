
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
	DimensionType dtSlice=DimensionType(ConfigValue("congeal.output.dimension",int(DimensionY)));
	int nSlice=ConfigValue("congeal.output.slice",.5) * recipie.PSource(0)->Size().Dim(dtSlice);

	// shut up the compiler
	nSlice++;
	nSlice--;
	
	String sflPrefix = ConfigValue("congeal.output.prefix","../output/congeal/");

#if 0
	//----------------------------------------
	UD1("writing out representative slices of congealed volumes");
	{
		for (int n=0; n<recipie.CSources(); n++){
			WritePGM(
				StringF(sflPrefix + "out%03d.%03d-%03d-%03d.pgm",nSchedule,nProgress,n,nSlice),
				Rasterize(Slice(dtSlice,nSlice,recipie.PSource(n))));
		}
	}

	//----------------------------------------
	UD1("writing out representative slices of average of congealed volumes");
	{
		WritePGM(
			StringF(sflPrefix + "out%03d.%03d-average-%03d.pgm",nSchedule,nProgress,nSlice),
			Rasterize(
				Slice(
					dtSlice,nSlice,
					Average(recipie.VSources(),recipie.CSources()))));
	}
	//----------------------------------------
	UD1("writing out representative slices of average of congealed volumes");
	{
		WritePGM(
			StringF(sflPrefix + "out%03d.%03d-weightedaverage-%03d.pgm",nSchedule,nProgress,nSlice),
			Rasterize(
				Slice(
					dtSlice,nSlice,
					WeightedAverage(
						Sum(
							recipie.VSources(),
							recipie.CSources(),
							DistributionField10<typename Recipie_G3R_All_G3I::type_source>),
						recipie.VSources(),recipie.CSources()
					))));
	}

	//----------------------------------------
	UD1("writing out representative slices of max dimension of average of distribution over congealed volumes");
	{
		WritePGM(
			StringF(sflPrefix + "out%03d.%03d-maxdfield-%03d.pgm",nSchedule,nProgress,nSlice),
			Rasterize(
				Slice(
					dtSlice,nSlice,
					MaximumDimension(
						Average(
							recipie.VSources(),
							recipie.CSources(),
							DistributionField10<TypeOfSource(Recipie_G3R_All_G3I)>)))));
	}
#endif

/*

for (int n=0; n<recipie.CSources(); n++){
GreyImage* pgi=Rasterize(
Slice(
DimensionType(dtSlice),nSlice,
GreyPixelRangeScale(
1024,1024,
recipie.PSource(n))));
forpoint(Point2D, pt,0,pgi->Size()){
Pixel8BitGrey pxl;
GetPoint(*pgi,pxl,pt);
if (pxl!=127){
UD("%d %s: %d", n,pt.Describe().V(),pxl);
}
}
exit(1);
}
*/


	//----------------------------------------
	UD1("writing out overview congealed volumes");
	{
		const int dataMean=ConfigValue("congeal.output.colors.mid",2048);
		const int dataRange=ConfigValue("congeal.output.colors.range",1024);

		for (int dtSlice=0; dtSlice<3; dtSlice++){
			int nSlice=ConfigValue("congeal.output.slice",.5) * recipie.PSource(0)->Size().Dim(dtSlice);

			ScanLayout layoutInputs;
			int cLayout=min(recipie.CSources(),ConfigValue("congeal.output.sourcegrid",1024));
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
										(//										CubicInterpolation(
											recipie.PSource(n)))))));
				}
				layoutInputs.AutoArrange();

				UD("ARRANGE TIME (%g)", TOCK(dtmWriteVolume));
			}

			{
				TICK(dtmWriteVolume);
				WritePGM(
					StringF(sflPrefix + "out%03d.%03d-inputs%d-%03d.pgm",nSchedule,nProgress,dtSlice,nSlice),
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
					StringF(sflPrefix + "out%03d.%03d-average%d-%03d.pgm",nSchedule,nProgress,dtSlice,nSlice),
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
									(//									CubicInterpolation(
										Average(
											recipie.VSources(),
											recipie.CSources())))))));

				UD("WRITE AVERAGE TIME (%g)", TOCK(dtmWriteVolume));
			}
		}
	}
}


	//----------------------------------------
	// method 2
/*

UD1("writing out representative slices of max dimension of average of distribution over congealed volumes");
{
typedef TypeOfSource(Recipie_G3R_All_G3I) SOURCE;
typedef TypeOfData(Recipie_G3R_All_G3I) DATA;
typedef TypeOfPrecision(Recipie_G3R_All_G3I) PRECISION;
const int DIMENSIONALITY = TypeOfDimensionality(Recipie_G3R_All_G3I);

Real rSigmaSpatial = ConfigValue("congeal.output.spatialsigma",.2);

#define Extends(_type) TypeOfData(_type), TypeOfDimensionality(_type), TypeOfPrecision(_type), _type

typedef SourceAccessorDistributionFieldOf<
DistributionFieldInfo<TypeOfData(SOURCE)>::type,
DIMENSIONALITY,	
PRECISION,
SOURCE
> DField;

typedef SourceCombinePointwiseOperatorOf<
OperatorAverageOf<TypeOfData(DField),	TypeOfData(DField)>,
DIMENSIONALITY,	
PRECISION,
DField
>  AvgDField;
	
typedef SourceAccessorConvolutionOf<Extends(AvgDField) ProbabilityDensityFunction)> ;

ProbabilityDensityFunction* pgvPDF = Average(
recipie.VSources(),
recipie.CSources(),
DistributionField10<TypeOfSource(Recipie_G3R_All_G3I)>);

typedef SourceMixedLookupDimensionOf(
TypeOfData(ProbabilityDensityFunction),DIMENSIONS,PRECISION,SOURCE,ProbabilityDensityFunction
) Lookup;
Lookup vlooup

LookupDimension
WritePGM(
Rasterize(
Slice(
dtSlice,nSlice,
SelectSource(
IndexOfMaximum(
LookupDimension(
recipie.VSources(),
recipie.CSources(),
Blur(
pgvPDF)))))),
StringF(sflPrefix + "out%03d.%03d-maxl-%03d.pgm",nSchedule,nProgress,nSlice)
);
}
*/


/*

gvPDF = average(distrofield ( all ));

selectsource(indexofmax(blur(lookupdimension(each, gvPDF)),




SelectSource(
	IndexOfMaximum(
		Blur(
			LookupDimension(
				recipie.PSource(n)
				Average(
					recipie.VSources(),
					recipie.CSources(),
					DistributionField10()
				)
			)
		) [0...c]
	),
	recipie.PSource(n) [0...c]
)


lookupdimension of each


	{
		typedef TypeOfSource(Recipie_G3R_All_G3I) SOURCE;
		typedef TypeOfData(Recipie_G3R_All_G3I) DATA;
		typedef TypeOfPrecision(Recipie_G3R_All_G3I) PRECISION;
		const int DIMENSION = TypeOfDimension(Recipie_G3R_All_G3I);

		Real rSigmaSpatial = ConfigValue("congeal.output.spatialsigma",.2);


		Average(
			recipie.VSources(),
			recipie.CSources(),
			DistributionField10<TypeOfSource(Recipie_G3R_All_G3I)>);


*/


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
		ASSERTf(pfl,"could not open file: " + sflInputfiles);
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
			ERROR("not currently supported");
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

	String sflPrefix = ConfigValue("congeal.output.prefix","../output/congeal/");

	system("cp '" + GetConfigFile() + "' '" + sflPrefix + "config'");

	// allocate warpfield
	for (int n=0; n<c; n++){ 
		recipie.LayerWarp0(n).AllocateWarpfield(PointOf<3,Real>(1),Cubic);
		recipie.LayerWarp1(n).AllocateWarpfield(PointOf<3,Real>(1),Cubic);
		recipie.LayerWarp2(n).AllocateWarpfield(PointOf<3,Real>(1),Cubic);
		recipie.LayerWarp3(n).AllocateWarpfield(PointOf<3,Real>(1),Cubic);
		Set(*recipie.LayerWarp0(n).GetPWarpControlPoints(),0,0,0,Point3DReal(0.));
		Set(*recipie.LayerWarp1(n).GetPWarpControlPoints(),0,0,0,Point3DReal(0.));
		Set(*recipie.LayerWarp2(n).GetPWarpControlPoints(),0,0,0,Point3DReal(0.));
		Set(*recipie.LayerWarp3(n).GetPWarpControlPoints(),0,0,0,Point3DReal(0.));
	}

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
		String sflCache = StringF(sflPrefix + "out%03d.cache",nSchedule);

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
		SetConfigFile(vsCLA[1]);
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
		else{
			ERROR("Unknown test:"+ sTest +"--");
		}

	}
	D("Objects leaked %d", 
		ObjectsOutstanding()	-cStaticObjects );

	ASSERTf(	
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



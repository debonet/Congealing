#include "libPGM.h"
#include "libSourceOperations.h"
#include "SourceRegistry.h"

#include "libConfiguration.h"
#include "libBaseTypes.h"
#include "libAssistants.h"
#include "libSerialize.h"

#undef DEBUG 
#define DEBUG 3
#include "libDebug.h"


//============================================================================
//============================================================================
void SuperRes()
{
	typedef Point256DReal DPoint; //Distribution Point
	typedef ImageOf<DPoint,2> DField; //Distribution Field
	const Real rSigmaParzen = ConfigValue("parzen",3.);

	//--------------------------------------------------------
	D("reading input image");
	GreyImage *pi=ClaimPointer(ReadPGM(ConfigValue("inputfile","../data/incrop0.pgm")));

	//--------------------------------------------------------
	D("allocating result");
	const Real rScalefactor = ConfigValue("scalefactor",2.);
	const Point2DReal ptScalefactor(rScalefactor);
	GreyImage iOut;
	iOut.Allocate(pi->Size()*ptScalefactor);

	const Real rSigmaBlur = ConfigValue("blur",4.);
	const InterpolationType it=InterpolationType(ConfigValue("interpolation",0));
	const int cBlocksize = ConfigValue("blocksize", 8);

	D("exploding, blurring, maxing and converting back into a maximum a posteriori image");
	forpoint(Point2D,ptShift,0,pi->Size(),cBlocksize){
		UD("working on block: " + ptShift.Describe() + " / " + (pi->Size()).Describe());

		//--------------------------------------------------------
		D("clipping, exploding, interpolating, scaling, blurring");
		Point2D ptEnd = min(Point2D(cBlocksize),pi->Size()-ptShift);
		int cSupport=int(3*rSigmaBlur);
		int cSupport2=cSupport*2+1;

		DField* psrcBlur = ClaimPointer(
			Rasterize(
				GaussianBlurAlong(
					DimensionY,	rSigmaBlur, cSupport2,
					Aperature(
						Point2D(cSupport*rScalefactor),ptEnd*rScalefactor,
						Rasterize(
							GaussianBlurAlong(
								DimensionX, rSigmaBlur, cSupport2,
								ScaleBy(
									ptScalefactor,
									Interpolate(
										it,
										DistributionField(
											rSigmaParzen,
											Aperature(
												ptShift-cSupport,ptEnd+cSupport2,
												pi))))))))));

		//--------------------------------------------------------
		D("convert back into a maximum a posteriori image");
/*
		psrcBlur->PrepareForAccess();
		Point2D ptScaleShift=ptShift*ptScalefactor;
		forpoint(Point2D,pt,0.,psrcBlur->Size()){
			DPoint dpoint;
			GetPoint(*psrcBlur,dpoint,pt);
			SetPoint(iOut,pt+ptScaleShift,Pixel8BitGrey(dpoint.MaxDimension()));
		}

		ReleasePointer(psrcBlur);
*/
		psrcBlur->PrepareForAccess();
		CopyInto(
			Aperature(
				Point2D(ptShift*ptScalefactor),
				psrcBlur->Size(),
				&iOut),
			MaximumDimension(psrcBlur));

		ReleasePointer(psrcBlur);
				
			
	}

	//--------------------------------------------------------
	D("writing output");
	const char *sflOut=ConfigValue("outputfile","../output/superres/superres.pgm");
	WritePGM(sflOut,&iOut);
	system(String("convert ") +sflOut + " x:");

	//--------------------------------------------------------
	D("cleanup");
	ReleasePointer(pi);
}










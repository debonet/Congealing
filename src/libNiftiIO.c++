#include "libNiftiIO.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> // for fstat()
#include <stdarg.h>

#include "libAssistants.h"
#include "libUtility.h"
#include "libUtility.h"
#include "libConfiguration.h"

int strrcmp(const char *sfl1, const char*sfl2){
	int c1=strlen(sfl1);
	int c2=strlen(sfl2);

	if (c1>c2){
		return 1;
	}

	return strcmp(sfl1,&sfl2[c2-c1]);
}


String DescribeNifti(NiftiHeader &niih){
	return (
		String("NiftiHeader: ") 
		+ "\n\t\tcySizeHeader: " + niih.m_cySizeHeader  
		+ "\n\t\tvchUnused: " + niih.m_vchUnused  

		+ "\n\t\tvcSize: " + niih.m_vcSize[0]
		+ "\n\t\tvcSize: " + niih.m_vcSize[1]
		+ "\n\t\tvcSize: " + niih.m_vcSize[2]
		+ "\n\t\tvcSize: " + niih.m_vcSize[3]
		+ "\n\t\tvcSize: " + niih.m_vcSize[4]
		+ "\n\t\tvcSize: " + niih.m_vcSize[5]
		+ "\n\t\tvcSize: " + niih.m_vcSize[6]
		+ "\n\t\tvcSize: " + niih.m_vcSize[7]


		+ "\n\t\tvrIntent: " + niih.m_vrIntent[0]
		+ "\n\t\tvrIntent: " + niih.m_vrIntent[1]
		+ "\n\t\tvrIntent: " + niih.m_vrIntent[2]

		+ "\n\t\tnCodeIntent: " + niih.m_nCodeIntent  
		+ "\n\t\tnCodeData: " + niih.m_nCodeData  
		+ "\n\t\tcBitsPerPixel: " + niih.m_cBitsPerPixel   
		+ "\n\t\tnSliceStart: " + niih.m_nSliceStart  

		+ "\n\t\tvrScale: " + niih.m_vrScale[0]
		+ "\n\t\tvrScale: " + niih.m_vrScale[1]
		+ "\n\t\tvrScale: " + niih.m_vrScale[2]
		+ "\n\t\tvrScale: " + niih.m_vrScale[3]
		+ "\n\t\tvrScale: " + niih.m_vrScale[4]
		+ "\n\t\tvrScale: " + niih.m_vrScale[5]
		+ "\n\t\tvrScale: " + niih.m_vrScale[6]
		+ "\n\t\tvrScale: " + niih.m_vrScale[7]

		+ "\n\t\tnyDataStart: " + niih.m_nyDataStart  
		+ "\n\t\trDataScale: " + niih.m_rDataScale  
		+ "\n\t\tdrDataOffset: " + niih.m_drDataOffset  
		+ "\n\t\tnyDataEnd: " + niih.m_nyDataEnd  

		+ "\n\t\tnCodeSlice: " + ((int)(niih.m_nCodeSlice))

		+ "\n\t\tnCodeUnits: " + ((int)niih.m_nCodeUnits)
		+ "\n\t\trDataMax: " + niih.m_rDataMax  
		+ "\n\t\trDataMin: " + niih.m_rDataMin  
		+ "\n\t\tdtmSlice: " + niih.m_dtmSlice  
		+ "\n\t\tdtmOffset: " + niih.m_dtmOffset  

		+ "\n\t\tvchUnused2: " + niih.m_vchUnused2 

		+ "\n\t\tvchDescription: " + niih.m_vchDescription  
		+ "\n\t\tsflAux: " + niih.m_sflAux  

		+ "\n\t\tnCodeQForm: " + niih.m_nCodeQForm  
		+ "\n\t\tnCodeSForm: " + niih.m_nCodeSForm  

		+ "\n\t\tvrQuaternion: " + niih.m_vrQuaternion[0]
		+ "\n\t\tvrQuaternion: " + niih.m_vrQuaternion[1]
		+ "\n\t\tvrQuaternion: " + niih.m_vrQuaternion[2]
		+ "\n\t\tvrQuaternion: " + niih.m_vrQuaternion[3]
		+ "\n\t\tvrQuaternion: " + niih.m_vrQuaternion[4]
		+ "\n\t\tvrQuaternion: " + niih.m_vrQuaternion[5]

		+ "\n\t\tvrAffine: " + niih.m_vrAffine[0]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[1]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[2]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[3]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[4]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[5]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[6]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[7]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[8]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[9]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[10]
		+ "\n\t\tvrAffine: " + niih.m_vrAffine[11]

		+ "\n\t\tvsIntentName: " + niih.m_vsIntentName 

		+ "\n\t\tvchMagic: " + niih.m_vchMagic  
	);
}


//============================================================================
//============================================================================
void NormalizeNiftiInput_SimpleMinToMaxScale(
	float *vr,	int cSize,	NiftiDataVolume *pgv
){

	// scale data into bytes
	float rMin=vr[0];
	float rMax=vr[0];

	for (int n=1; n<cSize; n++){
		if (rMin>vr[n]){
			rMin=vr[n];
		}
		if (rMax<vr[n]){
			rMax=vr[n];
		}
	}
	float rScale=rMax-rMin;
	float rRange=DATARANGE;//Range(NiftiPixel);

	P("Volume max %g min %g scale %g range %g", rMax, rMin, rScale,rRange);

	for (int n=0; n<cSize; n++){
		pgv->WriteData()[n]=(vr[n]-rMin)/rScale*rRange;
		//		pgv->WriteData()[n]=vr[n];
	}
}

//============================================================================
//============================================================================
void BuildHistogram(
	int *vc, int c, float *vr,	int cr
){
	// TODO: consider memset()?
	for (int n=0; n<c; n++){
		vc[n]=0;
	}

	for (int n=0; n<cr; n++){
		vc[ Bound(int(vr[n]),0,c-1) ] ++;
	}

}

//============================================================================
//============================================================================
void NormalizeNiftiInput_ScaleToNthPercentiles(
	float *vr,	int cSize,	NiftiDataVolume *pgv
){

	int c=4096;
	int* vc=new int[c];
	BuildHistogram(vc,c,vr,cSize);

	// scale data into bytes
	float rMin=vr[0];
	float rMax=vr[0];

	Real rPercentMax = ConfigValue(
		"congeal.inputfiles.normalization.percentile.max",99.5
	)/100.;

	Real rPercentMaxStretch = ConfigValue(
		"congeal.inputfiles.normalization.percentile.maxstretch",
		1./(rPercentMax?rPercentMax:1)*1.5
	);


	int cMaxCumulative = cSize * rPercentMax;

	int cCumulative=0;
	for (int n=0; n<c; n++){
		cCumulative += vc[n];
		if (cCumulative >= cMaxCumulative){
			rMax=n * rPercentMaxStretch;
			break;
		}
	}

	Real rPercentMin = ConfigValue(
		"congeal.inputfiles.normalization.percentile.min",0.
	)/100.;

	int cMinCumulative = cSize * rPercentMin;

	cCumulative=0;
	for (int n=0; n<c; n++){
		cCumulative += vc[n];
		if (cCumulative >= cMinCumulative){
			rMin=n/(rPercentMin?rPercentMin:1);
			break;
		}
	}

	delete [] vc;

	float rScale=rMax-rMin;
	float rRange=DATARANGE;//Range(NiftiPixel);

	P(
		"Volume %g-percentile-max %g %g-percentile-min %g scale %g range %g", 
		rPercentMin,rMax,rPercentMax, rMin, rScale,rRange
	);

	for (int n=0; n<cSize; n++){
		pgv->WriteData()[n] = Bound(( vr[n]-rMin ) / rScale*rRange,(float)0.,rRange-1);
	}
}


//============================================================================
//============================================================================
void NormalizeNiftiInput_ThresholdEqualize(
	float *vr,	int cSize,	NiftiDataVolume *pgv
){
	D("building hist");

	int c = 4096;
	int* vc = new int[c];
	BuildHistogram(vc,c,vr,cSize);

	int* vcCum = new int[c];
	D("building cum");

	vcCum[0]=vc[0];
	for (int n=1; n<c; n++){
		vcCum[n] = vcCum[n-1]+vc[n];
	}


	int nThreshold = ConfigValue(
		"congeal.inputfiles.normalization.threshold",10
	);

	int cStart = vcCum[nThreshold];
	int d = (cSize - cStart) / DATARANGE;
	
	D("M %d %d",cStart,d);

	int* vcEq = new int[c];
	for (int n=0; n<nThreshold; n++){
		vcEq[n]=0;
	}
	for (int n=nThreshold; n<c; n++){
		vcEq[n]=(vcCum[n]-cStart) / d;
	}

	D("setting up eq");
	for (int n=0; n<cSize; n++){
		pgv->WriteData()[n] = vcEq[int(vr[n])];
	}

	delete [] vc;
	delete [] vcCum;
	delete [] vcEq;
}


//============================================================================
//============================================================================
NiftiVolume* ReadNifti(const char* sfl)
{
	NiftiHeader nifti;
	CONGEAL_ASSERT(sizeof(NiftiHeader)==348);

	// TODO: consider natively reading .gz files

	int fd=open(sfl,O_RDONLY);
	CONGEAL_ASSERTf(fd>=0, "Failed to open %s", sfl);
	
	read(fd,&nifti,sizeof(NiftiHeader));

//	D(DescribeNifti(*((NiftiHeader*)&nifti)));

/*

	D("Intent %d", nifti.m_nCodeIntent);
	D("Data %d", nifti.m_nCodeData);

	for (int n=0; n<8; n++){
		UD("DIM: %d %d", n, int(nifti.m_vcSize[n]));
	}

	for (int n=0; n<8; n++){
		UD("Scale: %d %f", n, double(nifti.m_vrScale[n]));
	}
*/


	CONGEAL_ASSERT(nifti.m_vcSize[0]==3);

	int cX=nifti.m_vcSize[1];
	int cY=nifti.m_vcSize[2];
	int cZ=nifti.m_vcSize[3];

	Real rX=nifti.m_vrScale[1];
	Real rY=nifti.m_vrScale[2];
	Real rZ=nifti.m_vrScale[3];

	int cSize=cX*cY*cZ;

	// only know how to do FLOAT32's right now
	CONGEAL_ASSERT(nifti.m_nCodeData==NIFTI_TYPE_FLOAT32);

	// check the size
	struct stat statbuf;
	fstat(fd, &statbuf);
	int c = statbuf.st_size;

	CONGEAL_ASSERT(c-nifti.m_nyDataStart == cSize * 4);

	// move to the data
	lseek(fd, nifti.m_nyDataStart, SEEK_SET);

	// read it
	float* vr=new float[cSize];
	read(fd,vr,cSize*sizeof(float));

	close(fd);

	// TODO: use rDataScale and rDataOffset if 
	// normalization isn't being used.
	// for (int n=0; n<cSize;n++){
	//   vr[n]=vr[n]*nifti.m_rDataScale + nifti.m_rDataOffset;
	// }

	NiftiDataVolume* pgv=new NiftiDataVolume;
	pgv->Allocate(Point3D(cX, cY, cZ));

	CONGEAL_ASSERT(pgv->CSize()==cSize);

	NormalizeNiftiInput_SimpleMinToMaxScale(vr,cSize,pgv);

	return Aperature(
		Point3D(0.),
		pgv->Size()*Point3DReal(rX,rY,rZ),
		ScaleBy(
			Point3DReal(rX,rY,rZ),
			NIFTI_INTERPOLANT_FUNCTION(
				HandoffPointer(pgv)
			)));
}





//============================================================================
//============================================================================
bool WriteNifti(const char* sfl, NiftiDataVolume* pv)
{
	NiftiHeader nifti;
	CONGEAL_ASSERT(sizeof(NiftiHeader)==348);

	int fd=open(sfl,O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IROTH);
	CONGEAL_ASSERTf(fd>=0, "Failed to open %s", sfl);

	nifti.m_cySizeHeader=348;
	nifti.m_vcSize[0]=3;
	nifti.m_vcSize[1]=pv->CX();
	nifti.m_vcSize[2]=pv->CY();
	nifti.m_vcSize[3]=pv->CZ();
	nifti.m_vcSize[4]=1;
	nifti.m_vcSize[5]=1;
	nifti.m_vcSize[6]=1;
	nifti.m_vcSize[7]=1;

	nifti.m_vchUnused[0]=0;

	// Unsure of what this is
	nifti.m_vrIntent[0]=0.; // intent parameters
	nifti.m_vrIntent[1]=0.; // intent parameters
	nifti.m_vrIntent[2]=0.; // intent parameters

	nifti.m_nCodeIntent=NIFTI_INTENT_NONE; // NIFTI_INTENT_* code?
	nifti.m_nCodeData=NIFTI_TYPE_FLOAT32; // NIFTI_TYPE_* code
	nifti.m_cBitsPerPixel=32;  

	
	nifti.m_nSliceStart=0; // First slice index.

	// we're storing what we have
	nifti.m_vrScale[0]=-1.0; // Grid spacings.
	nifti.m_vrScale[1]=1.0; // Grid spacings.
	nifti.m_vrScale[2]=1.0; // Grid spacings.
	nifti.m_vrScale[3]=1.0; // Grid spacings.
	nifti.m_vrScale[4]=0.0; // Grid spacings.
	nifti.m_vrScale[5]=1.0; // Grid spacings.
	nifti.m_vrScale[6]=1.0; // Grid spacings.
	nifti.m_vrScale[7]=1.0; // Grid spacings.

	// just the header then data
	nifti.m_nyDataStart=352; // Offset into .nii file

	nifti.m_rDataScale=0.0; // Data scaling: slope.
	nifti.m_drDataOffset=0.0; // Data scaling: offset.
	nifti.m_nyDataEnd=0; // Last slice index.

	// unsure
	nifti.m_nCodeSlice=NIFTI_SLICE_UNKNOWN; // NIFTI_SLICE_* code

	// lost, not stored in our object
  nifti.m_nCodeUnits=10; // NIFTI_UNITS_* code

	
	nifti.m_rDataMax=0; // Max display intensity
	nifti.m_rDataMin=0; // Min display intensity

	// unsure
	nifti.m_dtmSlice=0; // Time for slice.
	nifti.m_dtmOffset=0; // Time axis shift.

	//nifti.m_vchUnused2[8];
	nifti.m_vchUnused2[0]=0;

	nifti.m_vchDescription[0]=0;
	nifti.m_sflAux[0]=0;

	nifti.m_nCodeQForm=1;
	nifti.m_nCodeSForm=1;

	// are these needed?
	nifti.m_vrQuaternion[0]=0.; // Quaternion parameters
	nifti.m_vrQuaternion[1]=.7071; // Quaternion parameters
	nifti.m_vrQuaternion[2]=-0.7071; // Quaternion parameters
	nifti.m_vrQuaternion[3]=128.; // Quaternion parameters
	nifti.m_vrQuaternion[4]=-128.; // Quaternion parameters
	nifti.m_vrQuaternion[5]=128.; // Quaternion parameters

	nifti.m_vrAffine[0]=-1.; // affine transform
	nifti.m_vrAffine[1]=0.; // affine transform
	nifti.m_vrAffine[2]=0.; // affine transform
	nifti.m_vrAffine[3]=128.; // affine transform
	nifti.m_vrAffine[4]=0.; // affine transform
	nifti.m_vrAffine[5]=0.; // affine transform
	nifti.m_vrAffine[6]=1.; // affine transform
	nifti.m_vrAffine[7]=-128; // affine transform
	nifti.m_vrAffine[8]=0.; // affine transform
	nifti.m_vrAffine[9]=-1.; // affine transform
	nifti.m_vrAffine[10]=0.; // affine transform
	nifti.m_vrAffine[11]=128.; // affine transform
	nifti.m_vsIntentName[0]=0;

	nifti.m_vchMagic[0]='n';
	nifti.m_vchMagic[1]='+';
	nifti.m_vchMagic[2]='1';
	nifti.m_vchMagic[3]='\0';


	int c=pv->CSize();
	float* vr=new float[c];

	int n=0;
	forpoint(Point3D,pt,0, pv->Size()){
		NiftiPixel pxl;
		pv->GetPoint(pxl,pt);
		vr[n]=pxl;
		n++;
	}

	
	write(fd,&nifti,sizeof(NiftiHeader));

	char vyZeros[4]={0,0,0,0};
	write(fd,vyZeros,sizeof(vyZeros));
	write(fd,vr,c*sizeof(float));

	delete [] vr;

	ReleasePointer(ReadNifti(sfl));

	return true;
}




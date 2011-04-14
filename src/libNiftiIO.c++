#include "libNiftiIO.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> // for fstat()
#include <stdarg.h>

#include "libAssistants.h"

int strrcmp(const char *sfl1, const char*sfl2){
	int c1=strlen(sfl1);
	int c2=strlen(sfl2);

	if (c1>c2){
		return 1;
	}

	return strcmp(sfl1,&sfl2[c2-c1]);
}

//============================================================================
//============================================================================
NiftiVolume* ReadNifti(const char* sfl)
{
	NiftiHeader nifti;
	ASSERT(sizeof(NiftiHeader)==348);

	// TODO: consider natively reading .gz files

	int fd=open(sfl,O_RDONLY);
	ASSERTf(fd>=0, "Failed to open %s", sfl);
	
	read(fd,&nifti,sizeof(NiftiHeader));

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


	ASSERT(nifti.m_vcSize[0]==3);

	int cX=nifti.m_vcSize[1];
	int cY=nifti.m_vcSize[2];
	int cZ=nifti.m_vcSize[3];

	Real rX=nifti.m_vrScale[1];
	Real rY=nifti.m_vrScale[2];
	Real rZ=nifti.m_vrScale[3];

	int cSize=cX*cY*cZ;

	// only know how to do FLOAT32's right now
	ASSERT(nifti.m_nCodeData==NIFTI_TYPE_FLOAT32);

	// check the size
	struct stat statbuf;
	fstat(fd, &statbuf);
	int c = statbuf.st_size;

	ASSERT(c-nifti.m_nyDataStart == cSize * 4);

	// move to the data
	lseek(fd, nifti.m_nyDataStart, SEEK_SET);

	// read it
	float* vr=new float[cSize];
	read(fd,vr,cSize*sizeof(float));

	close(fd);

	NiftiDataVolume* pgv=new NiftiDataVolume;
	pgv->Allocate(Point3D(cX, cY, cZ));

	ASSERT(pgv->CSize()==cSize);

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
	P("MAX %g MIN %g", rMax, rMin);

//	float rScale=rMax-rMin;
	for (int n=0; n<cSize; n++){
//		pgv->WriteData()[n]=(vr[n]-rMin)/rScale*4095;
		pgv->WriteData()[n]=vr[n];
	}

	P("SIZE %d %d %d", cX,cY,cZ);
	P("SCALE %g %g %g", rX,rY,rZ);
	return ScaleBy(
		Point3DReal(rX,rY,rZ),
		NIFTI_INTERPOLANT_FUNCTION(
			HandoffPointer(pgv)
		)
	);

}





//============================================================================
//============================================================================
bool WriteNifti(const char* sfl, NiftiDataVolume* pv)
{
	NiftiHeader nifti;
	ASSERT(sizeof(NiftiHeader)==348);

	int fd=open(sfl,O_WRONLY);
	ASSERTf(fd>=0, "Failed to open %s", sfl);
	

	nifti.m_cySizeHeader=348;
	nifti.m_vcSize[0]=3;
	nifti.m_vcSize[1]=pv->CX();
	nifti.m_vcSize[2]=pv->CY();
	nifti.m_vcSize[3]=pv->CZ();


	// Unsure of what this is
	//nifti.m_vrIntent[3]; // intent parameters

	nifti.m_nCodeIntent=NIFTI_INTENT_NONE; // NIFTI_INTENT_* code?
	nifti.m_nCodeData=NIFTI_TYPE_FLOAT32; // NIFTI_TYPE_* code
	nifti.m_cBitsPerPixel=32;  

	
	nifti.m_nSliceStart=0; // First slice index.

	// we're storing what we have
	nifti.m_vrScale[0]=1.0; // Grid spacings.
	nifti.m_vrScale[1]=1.0; // Grid spacings.
	nifti.m_vrScale[2]=1.0; // Grid spacings.

	// just the header then data
	nifti.m_nyDataStart=348; // Offset into .nii file

	nifti.m_rDataScale=1.0; // Data scaling: slope.
	nifti.m_drDataOffset=0.0; // Data scaling: offset.
	nifti.m_nyDataEnd=pv->CSize()*sizeof(float); // Last slice index.

	// unsure
	nifti.m_nCodeSlice=NIFTI_SLICE_UNKNOWN; // NIFTI_SLICE_* code

	// lost, not stored in our object
  nifti.m_nCodeUnits=NIFTI_UNITS_MM; // NIFTI_UNITS_* code

	
	nifti.m_rDataMax=0; // Max display intensity
	nifti.m_rDataMin=4095; // Min display intensity

	// unsure
	nifti.m_dtmSlice=0; // Time for slice.
	nifti.m_dtmOffset=0; // Time axis shift.

	//nifti.m_vchUnused2[8];

	nifti.m_vchDescription[0]=0;
	nifti.m_sflAux[0]=0;

	nifti.m_nCodeQForm=NIFTI_XFORM_UNKNOWN; 
	nifti.m_nCodeSForm=NIFTI_XFORM_UNKNOWN; 

	// are these needed?
	//nifti.m_vrQuaternion[6]; // Quaternion parameters
	//nifti.m_vrAffine[12]; // affine transform
	//nifti.m_vsIntentName[16];

	nifti.m_vchMagic[0]='n';
	nifti.m_vchMagic[1]='i';
	nifti.m_vchMagic[2]='1';
	nifti.m_vchMagic[3]='\0';


	int c=pv->CSize();
	float* vr=new float[c];

	int n=0;
	forpoint(Point3D,pt,0, pt<pv->CSize(),1){
		NiftiPixel pxl;
		pv->GetPoint(pxl,pt);
		vr[n]=pxl;
		n++;
	}

	
	write(fd,&nifti,sizeof(NiftiHeader));
	write(fd,vr,c*sizeof(float));

	delete [] vr;

	return true;
}

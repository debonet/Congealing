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

	float rScale=rMax-rMin;
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

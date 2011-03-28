
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> // for fstat()
#include <stdarg.h>

#include "libBrainWebIO.h"
#include "Image.h"
#include "libAssistants.h"

//============================================================================
//============================================================================
BrainWebVolume* ReadBrainWeb(const char* sfl)
{
	FILE *pfl=fopen(sfl,"r");
	ASSERTf(pfl!=NULL, "could not open file %s", sfl);

	char s[4096];
	int nDims;
	int cX=-1;
	int cY=-1;
	int cZ=-1;
	double rX=1.0;
	double rY=1.0;
	double rZ=1.0;
	char sflData[4096]={'\0'};

	while(fgets(s,sizeof(s)-1,pfl) != NULL){
		sscanf(s,"NDims = %d", &nDims);
		sscanf(s,"DimSize = %d %d %d", &cX, &cY, &cZ);
		sscanf(s,"ElementSpacing = %lf %lf %lf", &rX, &rY, &rZ);
		sscanf(s,"ElementDataFile = %s", sflData);
	}
	fclose(pfl);

	ASSERTf(nDims == 3, "Must be 3D");
	ASSERTf(cX!=-1,"Must provide dimension");
	ASSERTf(cY!=-1,"Must provide dimension");
	ASSERTf(cZ!=-1,"Must provide dimension");

	char sflDataConstructed[4096]={'\0'};
	sprintf(sflDataConstructed, "%.*s.raw", (int)(strrchr(sfl,'.')-sfl),sfl);
	if (strcmp(sflData,sflDataConstructed)){
		WARNING("Datafile name does not match headerfile name");
		int fd=open(sflDataConstructed,O_RDONLY);
		if (fd!=-1){
			WARNING(
				"Datafile '%s' constructed from headerfile name"
				" exists. Using constructed datafile name", sflDataConstructed
			);
			strcpy(sflData,sflDataConstructed);
		}
	}


	D1("datafile %s", sflData);
	int fd=open(sflData,O_RDONLY);

	struct stat statbuf;
	fstat(fd, &statbuf);
	int c = statbuf.st_size;
	D1("Size %d %d %d scale %g %g %g", cX, cY, cZ, rX, rY, rZ);

	ASSERT(c >= cX*cY*cZ);

	if (c<2*cX*cY*cZ){
		// WORKAROUND: check for filesize bug, and adjust cZ accordingly
		cZ=c/(cX*cY);
	}

	GreyVolume* pgv;
	pgv=new GreyVolume;
	pgv->Allocate(Point3D(cX, cY, cZ));

	int cy=pgv->CSize()*sizeof(Pixel8BitGrey);


	if (c==2*pgv->CSize()){
		D1("resampling");
		byte* vdata=new byte[pgv->CSize()*2];
		read(fd,vdata, cy*2);
		for (int n=0; n<cy; n++){
			pgv->WriteData()[n]=(vdata[n*2]*16+vdata[n*2+1]/16);
		}
		delete [] vdata;
	}

	else if (c==4*pgv->CSize()){
		D("resampling ints");
		byte* vdata=new byte[pgv->CSize()*4];
		read(fd,vdata, cy*4);

		unsigned int *pz=(unsigned int*)vdata;
		unsigned int nMin=0;
		unsigned int nMax=0;
		for (int n=0; n<cy; n++){
			if (pz[n]>0){
				nMin=(nMin>0)?min(nMin,pz[n]):pz[n];
				nMax=(nMax>0)?max(nMax,pz[n]):pz[n];
			}
		}

		D("%d %d", nMin, nMax);
		for (int n=0; n<cy; n++){
			pgv->WriteData()[n]=double((pz[n]-nMin))/(nMax-nMin)*256;
		}

		delete [] vdata;
	}
	else{
		read(fd,pgv->WriteData(), cy);
	}

	close(fd);
	

	return ScaleBy(
		Point3DReal(rX,rY,rZ),
		BRAIN_WEB_INTERPOLANT_FUNCTION(
			HandoffPointer(pgv)
		)
	);
}

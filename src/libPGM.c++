#include "Image.h"
#include <stdio.h>
#include <stdarg.h>

//============================================================================
//============================================================================
void StripPNMComment(FILE *pfl){
	int chT=fgetc(pfl);
	if (chT=='#'){
		char sComment[4096];
		fgets(sComment,sizeof(sComment),pfl);
	}
	else{
		ungetc(chT,pfl);
	}
}

//============================================================================
//============================================================================
GreyImage* ReadPGM(const char *sfl){
	GreyImage* pi=new GreyImage;

	int cX;
	int cY;
	int cMaxVal;
	int c;
	Pixel8BitGrey *p;

	FILE *pfl;
	pfl=fopen(sfl,"r");

	if (pfl==NULL){
		goto error;
	}

	// header
	char ch1,ch2;
	if (fscanf(pfl,"%c%c ",&ch1,&ch2)!=2){
		goto error;
	}
	if (ch1!='P' || ch2!='5'){
		goto error;
	}
	StripPNMComment(pfl);

	// width
	if (fscanf(pfl,"%d ",&cX)!=1){
		goto error;
	}
	StripPNMComment(pfl);

	// height
	if (fscanf(pfl,"%d ",&cY)!=1){
		goto error;
	}
	StripPNMComment(pfl);

	// depth
	if (fscanf(pfl,"%d ",&cMaxVal)!=1){
		goto error;
	}
	if (cMaxVal > 255){
		fclose(pfl);
		goto error;
	}
	StripPNMComment(pfl);


	// pixel data
	pi->Allocate(Point2D(cX,cY));
	c=pi->CSize();
	p=pi->WriteData();
	for (int n=0; n<c; n++){
		p[n]=fgetc(pfl);
	}
	fclose(pfl);
	HandoffPointer(pi);
	return pi;

	error:{
		fprintf(stderr,"LoadPGM ERROR\n");
		fclose(pfl);
		HandoffPointer(pi);
		return pi;
	}
}


//============================================================================
//============================================================================
GreyImage* WritePGM( const char *sfl, GreyImage* pi)
{
	ClaimPointer(pi);
	FILE *pfl;
	pfl=fopen(sfl,"w");

	fprintf(
		pfl,
		"P5\n#written by libPGM\n%d %d\n%d\n",
		pi->CSize(0),pi->CSize(1),255
	);

	fwrite(pi->ReadData(),sizeof(Pixel8BitGrey),pi->CSize(),pfl);

	fclose(pfl);
	return HandoffPointer(pi);
}



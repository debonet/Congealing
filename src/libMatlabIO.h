

//template<class T>
//void WriteMatlab(T& t);


//============================================================================
// WriteMatlab for Real
//============================================================================
void WriteMatlab(
	String &s, 
	const Real&t
)
{
	s+=StringF("%.15g",t);
}
//============================================================================
// WriteMatlab for int
//============================================================================
void WriteMatlab(
	String &s, 
	const int&t
)
{
	s+=t;
}

//============================================================================
// WriteMatlab for Point2D
//============================================================================
template<class T>
void WriteMatlab(
	String &s, 
	const PointOf<2,T>& t
)
{
	WriteMatlab(s,t.X());
	s+=" ";
	WriteMatlab(s,t.Y());
}

//============================================================================
// WriteMatlab for Point3D
//============================================================================
template<class T>
void WriteMatlab(
	String &s, 
	const PointOf<3,T>& t
)
{
	WriteMatlab(s,t.X());
	s+=" ";
	WriteMatlab(s,t.Y());
	s+=" ";
	WriteMatlab(s,t.Z());
}

//============================================================================
// WriteMatlab for SourceMemory
//============================================================================
template<class DATA, int DIMENSIONALITY>
void WriteMatlab(
	String &s, 
	const SourceMemoryOf<DATA, DIMENSIONALITY>& t
)
{
//	s+="{'data' []}";

/*
	//D("writing SourceMemoryOf");
	s+="reshape([";
	int c=t.CSize();
	const DATA *pdata=t.ReadData();
	for (int n=0; n<c; n++){
		WriteMatlab(s,pdata[n]);
		s+=" ";
	}
	s+="]";
	for (int n=0; n<DIMENSIONALITY; n++){
		// needs a comma because dimensions are listed as arguments not as array
		s+=", "; 
		WriteMatlab(s,t.Size().Dim(n));
	}
	s+=")";
*/
}


//============================================================================
// WriteMatlab for Recipies
//============================================================================
template<class SOURCE> 
void WriteMatlab(
	String &s, 
	const RecipieOf<SOURCE>& t
)
{
	//D("writing RecipieOf");
	int c=t.CSources();
	s+="{...\n";
	for (int n=0; n<c; n++){
		s+="{...\n";
		WriteMatlab(s,*(t.PSource(n)));
		s+="} ...\n";
	}
	s+="}";
}


//============================================================================
// WriteMatlab for Translations
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
void WriteMatlab(
	String &s, 
	const SourceTransformTranslateOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>& t
)
{
	//D("writing Translations");
	s+="{'translate' [";
	WriteMatlab(s,t.Translate());
	s+="]} ...\n";

	WriteMatlab(s,*(t.PSource()));
}

//============================================================================
// WriteMatlab for 2D rotations
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
void WriteMatlab(
	String &s, 
	const SourceTransformRotateOf<DATA, 2, PRECISION, SOURCE>& t
)
{
	//D("writing rotations");
	s+="{'rotate2d' [";
	WriteMatlab(s,t.GetAngle());
	s+="]} ...\n";

	WriteMatlab(s,*(t.PSource()));
}



//============================================================================
// WriteMatlab for 3D rotations
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
void WriteMatlab(
	String &s, 
	const SourceTransformRotateOf<DATA, 3, PRECISION, SOURCE>& t
)
{
	//D("writing rotations");
	s+="{'rotate' [";
	WriteMatlab(s,t.GetAngleX());
	s+=" ";
	WriteMatlab(s,t.GetAngleY());
	s+=" ";
	WriteMatlab(s,t.GetAngleZ());
	s+="]} ...\n";

	WriteMatlab(s,*(t.PSource()));
}




//============================================================================
// WriteMatlab for Scale
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
void WriteMatlab(
	String &s, 
	const SourceTransformScaleOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>& t
)
{
	//D("writing scale");
	s+="{'scale' [";
	WriteMatlab(s,t.Scale());
	s+="]} ...\n";

	WriteMatlab(s,*(t.PSource()));
}

//============================================================================
// WriteMatlab for Warp
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
void WriteMatlab(
	String &s, 
	const SourceTransformWarpOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>& t
)
{
	//D("writing bspline");
	s+="{'bspline' ";
//	WriteMatlab(s,*t.PWarpControlPoints());

	const WarpControlPointsOf<DIMENSIONALITY> &t2=*t.PWarpControlPoints();
	s+="reshape([";
	int c=t2.CSize();
	const Point3DReal *pdata=t2.ReadData();
	for (int n=0; n<c; n++){
		WriteMatlab(s,pdata[n]);
		s+=" ";
	}
	s+="]";
	for (int n=0; n<DIMENSIONALITY; n++){
		// needs a comma because dimensions are listed as arguments not as array
		s+=", "; 
		WriteMatlab(s,t2.Size().Dim(n));
	}
	// add the last array dimension for the 3 displacement values
	s+=",3)";


	s+="} ...\n";

	WriteMatlab(s,*(t.PSource()));
}

//============================================================================
// SaveMatlab 
//============================================================================
template<class T>
void SaveMatlab(
	const char *sfl,
	const char *sfxn,
	const T& t
)
{
	//D("saving matlab file");
	String s;

	s+=StringF("function result = %s()\n",sfxn);
	s+="## this is an automatically generated function which returns data from Congeal\n\n";

	WriteMatlab(s,t);
	s+=";";

	FILE *pfl=fopen(sfl,"w");
	if (!pfl){
		CONGEAL_ERROR("could not open file %s",sfl);
	}
	else{
		fprintf(pfl,s.VCH());
		fclose(pfl);
	}
}


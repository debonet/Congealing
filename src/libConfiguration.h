#ifndef __LIBCONFIGURATION_H__
#define __LIBCONFIGURATION_H__

#include "HashTable.h"
#include "String.h"


class Configuration {
	static String g_sfl;
	static bool g_bRead;
	static HashSS g_h;

private:
	static bool IsWhitespace(char ch){
		return ch==' ' || ch=='\n' || ch=='\r' || ch=='\t';
	}

	static void ReadConfigFile(){
		UD("READING CONFIGURATION FILE: %s", g_sfl.V());
		g_bRead=true;

		FILE *pfl=fopen(g_sfl,"r");
		char s[4096];
		while(fgets(s,sizeof(s)-1,pfl) != NULL){
			// trim off trailing whitespace
			int nch=strlen(s)-1; 
			while(nch>=0 && IsWhitespace(s[nch])){
				nch--;
			}
			s[nch+1]=0;

			// split
			char sParam[4096];
			int cch;
			if (sscanf(s," %4095s %n", sParam,&cch)==1){
				if (sParam[0] != '#'){
					String sKey = ConfigEvalString(sParam);
					String sVar = ConfigEvalString(&s[cch]);
					g_h.Set(sKey,sVar);
				}
			}
		}
		fclose (pfl);
	}

	static String ConfigEvalStringInternal(const char *&pch) __attribute__((noinline))
	{
		// TODO: consider speeding this up using strrchr to quickly jump through the string
		VectorOf<char> vecch;
		while(*pch != 0){
			if (*pch == '{'){
				pch++;
				String sT=ConfigEvalStringInternal(pch);
				if (sT.V()[0]=='$'){
					sT=g_h.Get(String(&sT.V()[1]),"");
				}
				else if (sT.V()[0]=='+'){ 
          ASSERT(sT.V()[1]=='+'); // assume ++
					String sVal=g_h.Get(String(&sT.V()[2]),"");
					sVal=String(sVal.AsInt()+1);
					g_h.Set(String(&sT.V()[2]),sVal);
					sT=sVal+"";
				}
/*
	TODO: implement functionals

	else if (sT.V()[0]=='+'){
	vecs=ListFromString(String(&sT.V()[1]));
	Real r=0;
	for (int n=0; n<vecs.C(); n++){
	r=r+AsNumber(vecs[n]);
	}
	sT=String(r);
	}
*/
				vecch.Append(sT.V(), sT.C());
			}
			else if (*pch == '}'){
				vecch.Append('\0');
				return String(vecch.V());
			}
			else{
				vecch.Append(*pch);
			}
			pch++;
		}
		vecch.Append('\0');
		return String(vecch.V());
	}

	static String ConfigEvalString(const char *pch)
	{
		return ConfigEvalStringInternal(pch);
	}


public:
	static String Describe()
	{
		if (!g_bRead){
			ReadConfigFile();
		}

		String s;
		String sKey;
		String sVal;

		forhash(sKey,sVal,g_h){
			s+=_LINE + StringF("%60s",sKey.V()) + " := " + sVal + LINE_;
		}

		return s;
	}

	static void SetConfigFile(String sfl){
		g_sfl=sfl;
		g_bRead=false;
	}

	static String GetConfigFile(){
		return g_sfl;
	}

	static const char* ConfigValue(const char *s, const char* sDefault){
		if (!g_bRead){
			ReadConfigFile();
		}

		String sOut;

		if(g_h.GetInto(s,&sOut)){
			return sOut;
		}
		return sDefault;
	}

	static Real ConfigValue(const char *s, const double& rDefault){
		if (!g_bRead){
			ReadConfigFile();
		}

		String sOut;

		if( g_h.GetInto(s,&sOut)){
			return sOut.AsReal();
		}
		return rDefault;
	}

	static Real ConfigValue(const char *s, const float& rDefault){
		if (!g_bRead){
			ReadConfigFile();
		}

		String sOut;

		if( g_h.GetInto(s,&sOut)){
			return sOut.AsReal();
		}
		return rDefault;
	}
	

	static int ConfigValue(const char *s, const int& zDefault){
		if (!g_bRead){
			ReadConfigFile();
		}

		String sOut;

		if( g_h.GetInto(s,&sOut) ){
			return sOut.AsInt();
		}
		return zDefault;
	}
	

	static bool ConfigValue(const char *s, const bool& bDefault){
		if (!g_bRead){
			ReadConfigFile();
		}

		String sOut;

		if( g_h.GetInto(s,&sOut) ){
			return sOut=="true" || sOut=="yes";
		}
		return bDefault;
	}
};

inline void SetConfigFile(String sfl){
	Configuration::SetConfigFile(sfl);
}

inline String GetConfigFile(){
	return Configuration::GetConfigFile();
}

inline String DescribeConfiguration(){
	return Configuration::Describe();
}


inline const char* ConfigValue(const char *s, const char* tDefault){
	return Configuration::ConfigValue(s,tDefault);
}
template<class T>
inline T ConfigValue(const char *s, const T& tDefault){
	return Configuration::ConfigValue(s,tDefault);
}


#endif //__LIBCONFIGURATION_H__

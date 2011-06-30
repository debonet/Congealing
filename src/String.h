#ifndef __STRING_H__
#define __STRING_H__

#include <string.h>
#include "libRefCount.h"
#include <stdarg.h>


#undef DEBUG
#define DEBUG -1
#include "libDebug.h"

#ifdef _WIN32
#  define ALWAYS_INLINE
#else 
#define ALWAYS_INLINE __attribute__((always_inline))
#endif



#include "Stream.h"
typedef StreamOf<char> StCh;

class String
	: public StCh
{
protected:
	// private constructor to use wrap a 
	String(char* vch, int cch, bool bPrivate)
		: StCh(vch,cch,bPrivate)
	{}


public:
	String()  ALWAYS_INLINE
		: StCh()
	{}

	String(const String& s)   ALWAYS_INLINE
		: StCh(s)
	{}

	String(const char* vch)  ALWAYS_INLINE
		: StCh(vch,strlen(vch)+1)
	{
		SetSizeUnsafe(C()-1);
	}

	String(const int& n) ALWAYS_INLINE
		: StCh() 
	{
		EnsureSpace(32);
		SetSize(congeal_min(31,snprintf(V(),32,"%d",n)));
	}

	String(const Real& r)  ALWAYS_INLINE
		: StCh()
	{
		EnsureSpace(32);
		SetSize(congeal_min(31,snprintf(V(),32,"%.4lf",r)));
	}

	Real AsReal() // TODO: consider throwing up on failure
	{
		Real r=0;
		sscanf(V(),SFMT_REAL,&r);
		return r;
	}

	int AsInt() // TODO: consider throwing up on failure
	{ 
		int z=0;
		sscanf(V(),"%d",&z);
		return z;
	}

	bool operator ==(const String& s) const {
		return strcmp(V(),s.V())==0;
	}
	bool operator ==(const char *s) const {
		return strcmp(V(),s)==0;
	}
	bool operator !=(const char *s) const {
		return strcmp(V(),s)!=0;
	}
	bool operator >(const char *s) const {
		return strcmp(V(),s)>0;
	}
	bool operator <(const char *s) const {
		return strcmp(V(),s)<0;
	}
	bool operator >=(const char *s) const {
		return strcmp(V(),s)>=0;
	}
	bool operator <=(const char *s) const {
		return strcmp(V(),s)<=0;
	}

	char* VCH() 
	{
		return V();
	}

	const char* VCH() const
	{
		return V();
	}

 	String operator + (const int& t) const
	{
//		UD("adding %d to %s", t, V());
		return *this+String(t);
	}
 	String operator + (const byte& t) const
	{
//		UD("adding %d to %s", t, V());
		return *this+String(t);
	}
 	String operator + (const short& t) const
	{
//		UD("adding %d to %s", t, V());
		return *this+String(t);
	}
 	String operator + (const double& t) const
	{
//		UD("adding %lf to %s", t, V());
		return *this+String(t);
	}
 	String operator + (const float& t) const
	{
//		UD("adding %f to %s", t, V());
		return *this+String(t);
	}

	String operator + (char* const vch) const
	{
//		UD("adding %s to %s", vch, V());

		int cch=strlen(vch);

		int cchNew=cch+C();

		String s;
		s.EnsureSpace(m_nStart+cchNew+1);

		if (C() > 0){
			memcpy(s.V(),V(),C());
		}
		if (cch > 0){
			memcpy(&s[C()],vch,cch);
		}
		s[cchNew]=0;
		s.SetSizeUnsafe(m_nStart+cchNew);

		return s;
	}

	String operator + (const String& sIn) const
	{
//		UD("adding %s to %s", sIn.V(), V());
		int c=C();
		int cIn=sIn.C();
		int cNew=c+cIn;

		String s;
		s.EnsureSpace(m_nStart+cNew+1);

		if (c > 0){
			memcpy(s.V(),V(),c);
		}
		if (cIn > 0){
			memcpy(&s[c],sIn.V(),cIn);
		}
		s[cNew]=0;
		s.SetSizeUnsafe(m_nStart+cNew);

		return s;
	}


	int ReadInt()
	{
		CONGEAL_ASSERT(C()>0);
		int c=0;
		int z=0;
		CONGEAL_ASSERT(sscanf(V(),"%d%n",&z,&c)==1);
		m_nStart+=c;
		return z;
	}

	Real ReadReal()
	{
		CONGEAL_ASSERT(C()>0);
		int c=0;
		Real r=0;
		CONGEAL_ASSERT(sscanf(V(),SFMT_REAL "%n",&r,&c)==1);
		m_nStart+=c;
		return r;
	}

	String ReadString()
	{
		CONGEAL_ASSERT(C()>0);
		int c=0;
		char sT[4096];
		CONGEAL_ASSERT(sscanf(V(),"%4095s%n",sT,&c)==1);
		m_nStart+=c;
		return String(sT);
	}


	void Skip(const char* vch)
	{
		CONGEAL_ASSERT(C()>0);
		int c=strlen(vch);
		CONGEAL_ASSERT(strncmp(V(),vch,c)==0);
		m_nStart+=c;
	}

	String& operator += (const String &s)
	{
		Append(s.V(),s.C());
		EnsureSpace(m_nStart+C()+1);
		V()[C()]=0;
		return *this;
	}

	static String Indent(int c){
		return Repeat("\t",c);
	}

	static String Repeat(const String& s,int c){
		String sOut;
		sOut.EnsureSpace(s.C()*c+1);
		for (int n=0; n<c; n++){
			sOut+=s;
		}
		sOut[sOut.C()]=0;
		return sOut;
	}

	static String IndentRidgedly(const String& s, int c=1)
	{
		int cchOut=c;
		int cch=s.C();

		for (int nch=0; nch<cch; nch++){
			cchOut+=1;
			if (s[nch]=='\n' && nch!=cch-1){
				cchOut+=c;
			}
		}

		String sOut;
		sOut.EnsureSpace(cchOut+1);
		int nchOut=0;
		for (int n=0; n<c; n++){
			sOut[nchOut]='\t';
			nchOut++;
		}

		for (int nch=0; nch<cch; nch++){
			sOut[nchOut]=s[nch];
			nchOut++;
			if (s[nch]=='\n' && nch!=cch-1){
				for (int n=0; n<c; n++){
					sOut[nchOut]='\t';
					nchOut++;
				}
			}
		}
		sOut[nchOut]=0;
		sOut.SetSizeUnsafe(cchOut);
		return sOut;
	}
};

class StringIndent
{
	int m_c;

	friend String operator + (const StringIndent &s1, const char* &s2);
	friend String operator + (const StringIndent &s1, const String& s2);

public:
	StringIndent(int c)
	{
		m_c=c;
	}
};




static const StringIndent _INDENT(1);
static const StringIndent _INDENT2(2);
static const String _LINE;
static const String LINE_("\n");
static const String INDENT("\t");

inline String operator + (const StringIndent &s1, const String& s2){
	return String::IndentRidgedly(s2,s1.m_c);
}


inline String operator + (const StringIndent &s1, const char* &s2){
	return String::IndentRidgedly(String(s2),s1.m_c);
}

inline String operator + (const char *s1, const String& s2){
	return String(s1) + s2;
}

inline String operator + (const String& s1, const char* s2){
	return s1+(char* const)s2;
}

inline String operator + (int z, const String& s2){
	return String(z) + s2;
}

inline String operator + (byte y, const String& s2){
	return String(y) + s2;
}

inline String operator + (short yy, const String& s2){
	return String(yy) + s2;
}

inline String operator + (double r, const String& s2){
	return String(r) + s2;
}

inline String operator + (float r, const String& s2){
	return String(r) + s2;
}


template <class T>
inline String operator + (const T& t, const String& s2){
	return t.Describe() + s2;
}

template <class T>
inline String operator + (const String& s1, const T& t){
	return s1 + t.Describe();
}



String StringF(const char* sFmt,...);

#endif

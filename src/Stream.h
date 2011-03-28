#ifndef __STREAM_H__
#define __STREAM_H__

#include <string.h>
#include "libRefCount.h"
#include <stdarg.h>

#include <fcntl.h> // for O_RDONLY etc.
#include <sys/stat.h> // for fstat()
#include <unistd.h>

#undef DEBUG
#define DEBUG -1
#include "libDebug.h"


#include "Vector.h"
typedef VectorOf<char> VecCh;

template <class T>
class StreamOf
	: public VectorOf<T>
{
protected:
	int m_nStart;

	// private constructor to use wrap a 
	StreamOf(T* vt, int ct, bool bPrivate)
		: VectorOf<T>(vt,ct,bPrivate)
		, m_nStart(0)
	{	}

public:
	StreamOf()  __attribute__((always_inline))
		: VectorOf<T>()
		, m_nStart(0)
	{}

	StreamOf(const StreamOf<T>& st)   __attribute__((always_inline))
		: VectorOf<T>(st)
		, m_nStart(st.m_nStart)
	{}

	StreamOf(const T* vt, int c)  __attribute__((always_inline))
		: VectorOf<T>(vt,c)
		, m_nStart(0)
	{}

	StreamOf<T>& operator = (const StreamOf<T>& st) __attribute__((always_inline))
	{
		VectorOf<T>::operator=(st);
		m_nStart=st.m_nStart;
		return *this;
	}

	bool operator ==(const StreamOf<T>& st) const {
		return C()==st.C() && memcmp(V(),st,C())==0;
	}
	bool operator !=(const StreamOf<T>& st) const {
		return C()!=st.C() || memcmp(V(),st,C())!=0;
	}
	bool operator >(const StreamOf<T>& st) const {
		return C()>st.C() || memcmp(V(),st,C())>0;
	}
	bool operator <(const StreamOf<T>& st) const {
		return C()<st.C() || memcmp(V(),st,C())<0;
	}
	bool operator >=(const StreamOf<T>& st) const {
		return C()>=st.C() || memcmp(V(),st,C())>=0;
	}
	bool operator <=(const StreamOf<T>& st) const {
		return C()<=st.C() || memcmp(V(),st,C())<=0;
	}

	T* V() 
	{
		return &VectorOf<T>::V()[m_nStart];
	}

	const T* V() const
	{
		return &VectorOf<T>::V()[m_nStart];
	}

	operator T*()
	{
		return V();
	}

	operator const T *() const
	{
		return V();
	}

	int C() const
	{
		return VectorOf<T>::C()-m_nStart;
	}

	StreamOf operator + (const StreamOf<T>& stIn) const
	{
		int c=C();
		int cIn=stIn.C();
		int cNew=c+cIn;

		StreamOf<T> st;
		st.EnsureSpace(m_nStart+cNew+1);

		if (c > 0){
			memcpy(st.V(),V(),c);
		}
		if (cIn > 0){
			memcpy(&st[c],stIn.V(),cIn);
		}
		st[cNew]=0;
		st.SetSizeUnsafe(m_nStart+cNew);

		return st;
	}

	T& Read()
	{
		ASSERT(C()>0);
		T& t=V()[0];
		m_nStart++;
		return t;
	}

	void Skip(int d)
	{
		ASSERT(C()>=d);
		m_nStart+=d;
	}

	void Skip(const VectorOf<T> &v)
	{
		ASSERT(C()>=v.C());
		ASSERT(memcmp(V(),v.V(),v.C())==0);
		m_nStart+=v.C();
	}

	void Read(T* vt, int c)
	{
		ASSERT(C()>=c);
		memcpy(vt,V(),c);
		m_nStart+=c;
	}

	VectorOf<T> Read(int c)
	{
		ASSERT(C()>=c);
		VectorOf<T> vect;
		vect.Allocate(c);
		memcpy(vect.V(),V(),c);
		m_nStart+=c;
		return vect;
	}

	StreamOf<T>& operator += (const StreamOf<T>& st)
	{
		Append(st.V(),st.C());
		return *this;
	}
};


typedef StreamOf<byte> Stream;



//============================================================================
//============================================================================
inline void WriteStream(const Stream& st, const char* sfl)
{
	int fd = open(sfl, O_WRONLY | O_CREAT, 0666);
	ASSERT(fd>=0);
	write(fd,st.V(),st.C());
	close(fd);
}

//============================================================================
//============================================================================
inline bool ReadStream(Stream& st, const char* sfl)
{
	int fd = open(sfl, O_RDONLY);
	if (fd<0){
		return false;
	}
	else{
		struct stat statbuf;
		fstat(fd, &statbuf);
		int c = statbuf.st_size;

		st.EnsureSpace(st.C()+c);
			
		read(fd,&st.V()[st.C()], c);
		st.SetSize(st.C()+c);
		close(fd);
		return true;
	}
}
#endif

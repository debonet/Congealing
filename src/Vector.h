#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <string.h> //for memcpy()
#include "libRefCount.h"

#undef DEBUG
#define DEBUG 0
#include "libDebug.h"

#ifdef _WIN32
#  define ALWAYS_INLINE
#else 
#define ALWAYS_INLINE __attribute__((always_inline))
#endif


template <class T>
class VectorOf
	: public Counted
{
protected:
	T* m_vt;
	int m_ctAlloc;
	int m_ct;

protected:
	// private constructor to use wrap a 
	VectorOf(T* vt, int ct, bool bPrivate)
		: Counted()
	{
		m_vt=vt;
		m_ct=ct;
		m_ctAlloc=m_ct;
	}

	void Release(){
		if (AboutToDie()){
			SafeDeleteArray(m_vt);
		}
	}

	// this leaves undefined values at the end if c>m_ct
	void SetSizeUnsafe(int c)
	{
		CONGEAL_ASSERT(c<m_ctAlloc);
		m_ct=c;
	}

public:
	VectorOf()
		: Counted()
	{
		m_ctAlloc=0;
		m_ct=0;
		m_vt=NULL;
	}

	~VectorOf() ALWAYS_INLINE
	{
		Release();
	}


	VectorOf(const T* vt, int ct)  ALWAYS_INLINE
		: Counted()
	{
		m_ctAlloc = congeal_max(1<<int(ceil(log2(ct))),32);
		m_ct=ct;
		m_vt = new T [m_ctAlloc];
		if (vt!=NULL){
			memcpy(m_vt,vt,m_ct*sizeof(T));
		}
	}

	VectorOf(const VectorOf& vec)   ALWAYS_INLINE
		: Counted(vec)
	{
		m_vt=vec.m_vt;
		m_ct=vec.m_ct;
		m_ctAlloc=vec.m_ctAlloc;
	}

	VectorOf& operator = (const VectorOf& vec) ALWAYS_INLINE
	{
		Release();
		Counted::operator=(vec);
		m_vt=vec.m_vt;
		m_ct=vec.m_ct;
		m_ctAlloc=vec.m_ctAlloc;
		return *this;
	}

	// TODO: this might not be safe if multiple holders of object?
	void Clear()
	{
		SafeDeleteArray(m_vt);
		m_vt=NULL;
		m_ctAlloc=0;
		m_ct=0;
	}

	const int& C() const {
		return m_ct;
	}

	T* V(){
		return m_vt;
	}

	const T* V() const{
		return m_vt;
	}

	const T& Get(const int& n) const{
		return m_vt[n];
	}

	T& Get(const int& n) {
		return m_vt[n];
	}

	operator T*()
	{
		return m_vt;
	}

	operator const T *() const
	{
		return m_vt;
	}

	const T& operator[](const int& n) const{
		return m_vt[n];
	}

	T& operator[](const int& n) {
		return m_vt[n];
	}


	VectorOf<T> operator + (const VectorOf<T>& vec)
	{
		VectorOf<T> vecOut;
		vecOut.EnsureSpace(C()+vec.C());
		vecOut.Append(V(),C());
		vecOut.Append(vec.V(),vec.C());
		return vecOut;
	}

	VectorOf<T>& operator += (const VectorOf<T>& vec)
	{
		Append(vec.V(),vec.C());
		return *this;
	}

	void Append(const T* vt, int c)
	{
		if (vt==NULL){
			CONGEAL_ASSERTf(
				c<=0,
				"trying to append null pointer which is reported as not empty %d", 
				c);
			return;
		}
		if (c<=0){
			return;
		}

		EnsureSpace(m_ct + c);
		memcpy(&m_vt[m_ct],vt,c*sizeof(T));
		m_ct+=c;
	}

	void Append(const T& t)
	{
		EnsureSpace(m_ct+1);
		m_vt[m_ct]=t;
		m_ct++;
	}

	void EnsureSpace(int c)
	{
		if (c>m_ctAlloc){
			m_ctAlloc = congeal_max(1<<int(ceil(log2(c))),32);
			T* vtT = new T [m_ctAlloc];
			if (m_vt!=NULL){
				memcpy(vtT,m_vt,m_ct*sizeof(T));
				delete [] m_vt;
			}
			m_vt=vtT;
		}
	}

	// this leaves undefined values at the end if c>m_ct
	void SetSize(int c)
	{
		EnsureSpace(c);
		m_ct=c;
	}
};

#endif

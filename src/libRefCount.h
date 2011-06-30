#ifndef __REFERENCECOUNTING_H__
#define __REFERENCECOUNTING_H__

#include "libUtility.h"

#undef DEBUG
#define DEBUG 0
#include "libDebug.h"

#ifdef _WIN32
#  define ALWAYS_INLINE
#else 
#define ALWAYS_INLINE __attribute__((always_inline))
#endif


class Counted;
class String;

class CountedReferece //ref
{
	static int m_cRefObjects;

	friend int ObjectsOutstanding();
	friend class Counted;
	friend class String;

protected:
	int m_cReferences;
	Counted* m_pcounted;

public:
	CountedReferece(Counted* pcounted)
	{
		DEBUGONLY(m_cRefObjects++);
		UD1("REFCREATE: reference 0x%x created. objects outstanding: %d", (int)this, m_cRefObjects);

		m_cReferences=0;
		m_pcounted=pcounted;
	}

	~CountedReferece()
	{
		DEBUGONLY(m_cRefObjects--);
		UD1("REFDESTROY: reference 0x%x destroyed. objects outstanding: %d", (int)this, m_cRefObjects);
	}
};

inline int ObjectsOutstanding(){
#if DEBUG >= 0
	return CountedReferece::m_cRefObjects;
#else
	return 0;
#endif

}

class Counted // count
{
	template<class COUNTED>
	friend COUNTED*	ClaimPointer(COUNTED *pcount);

	template<class COUNTED>
	friend COUNTED*	HandoffPointer(COUNTED *pcount);
	friend void ReleasePointer(Counted *pcount);

	friend class String;

protected:	
	int m_cPointerRefs;

public:
	mutable CountedReferece* m_pref;

	Counted() 
	{
		m_pref=new CountedReferece(this);
		UD2("CREATE: counted created 0x%x with reference 0x%x", (int)this, (int)m_pref);
		m_cPointerRefs=1;
		Claim();
	}

	Counted(const Counted& refc)
	{
		m_pref=refc.m_pref;
		UD2("CREATE: counted created 0x%x with copied reference 0x%x", (int)this, (int)m_pref);
		m_cPointerRefs=refc.m_cPointerRefs;
		Claim();
	}

	virtual ~Counted() ALWAYS_INLINE
	{
		UD2("DESTROY: counted destroyed 0x%x with reference 0x%x", (int)this, (int)m_pref);
		m_cPointerRefs--;
//		WARNIF(m_cPointerRefs==0,"non-zero pointer ref");
		Release();
	}

#ifndef _WIN32
	Counted& operator = (const Counted& refc)
	{
		if (m_pref != refc.m_pref){
			Release();
			m_pref=refc.m_pref;
			Claim();
		}
		UD2("COPY: counted copied  0x%x from reference 0x%x", (int)this, (int)m_pref);
		// dont change m_cPointerRefs;
		return *this;
	}
#endif

	Counted& operator = (Counted& refc)
	{
		if (m_pref != refc.m_pref){
			Release();
			m_pref=refc.m_pref;
			Claim();
		}
		UD2("COPY: counted copied  0x%x from reference 0x%x", (int)this, (int)m_pref);
		// dont change m_cPointerRefs;
		return *this;
	}

	int References() const {
		return m_pref->m_cReferences;
	}

	int PointerReferences() const {
		return m_cPointerRefs;
	}

	void Claim() const {
		m_pref->m_cReferences++;
		UD2("CLAIM: counted 0x%x reference 0x%x increased to %d", (int)this, (int)m_pref, m_pref->m_cReferences);
	}

	void Release() const ALWAYS_INLINE
	{
		m_pref->m_cReferences--;
		UD2("RELEASE: counted 0x%x reference 0x%x decreased to %d", (int)this, (int)m_pref, m_pref->m_cReferences);
		if (m_pref->m_cReferences==0){
			UD2("KILL: counted 0x%x reference 0x%x destroyed", (int)this, (int)m_pref);
			delete m_pref;
		}
		UD2("RELEASE: done ");
	}

	bool AboutToDie(){
		return (m_pref->m_cReferences==1);
	}

};



//============================================================================
// pointer claiming:
//
//   Unlike object claiming, handled via the Claimed class above, which 
//   ensures the proper management of an object's dynamically allocated content,
//   pointer claiming manages the lifetime of dynamically created Claimed-derived 
//   pointers.
//
//   a pointer which is created via new Claimed() begins as a pointer-claim 
//   on it 
//
//   a pointer which has been claimed via ClaimPointer, or created via new
//   must, be released by either calling ReleasePointer() or HandoffPointer() 
//   or TakePointer()
//
//   a pointer which has been released via HandoffPointer() must be claimed
//   via ClaimPointer(), and then handled as above
// 
//   a pointer released via ReleasePointer() may be destroyed, if thre are
//   no other claims on it
// 
//   a pointer released via TakePointer() may be destroyed, if thre are
//   no other claims on it, and an object claim is made and passed back as
//   a stack-allocated object
//
//   when all claims are released via ReleasePointer() or TakePointer() the 
//   pointer is deleted
// 
//============================================================================

#if MULTITHREADPROTECTION
#  include <pthread.h>
namespace {
	pthread_mutex_t g_mutex;
	int Init(){
		D("INITIALIZING REFCOUNT MUTEX");
		pthread_mutex_init(&g_mutex,NULL);
	}
	int g_ignore=Init();
}

#  define REFCOUNT_LOCK	pthread_mutex_lock(&g_mutex);
#  define REFCOUNT_UNLOCK	pthread_mutex_unlock(&g_mutex);

#else
#  define REFCOUNT_LOCK	
#  define REFCOUNT_UNLOCK	
#endif

//============================================================================
//============================================================================
// take ownership of a pointer. This commits you to either 
// calling ReleasePointer() or HandoffPointer()
template<class COUNTED>
inline COUNTED*	ClaimPointer(COUNTED *pcount) ALWAYS_INLINE;
template<class COUNTED>
inline COUNTED*	ClaimPointer(COUNTED *pcount)
{
	REFCOUNT_LOCK;
	CONGEAL_ASSERT(pcount != NULL);
	pcount->m_cPointerRefs++;
	REFCOUNT_UNLOCK;
	return pcount;
}


//============================================================================
//============================================================================
// dismiss ownership of a pointer, but do not free
template<class COUNTED>
inline COUNTED*	HandoffPointer(COUNTED *pcount) ALWAYS_INLINE;
template<class COUNTED>
inline COUNTED*	HandoffPointer(COUNTED *pcount)
{
	REFCOUNT_LOCK;
	CONGEAL_ASSERT(pcount != NULL);
	CONGEAL_ASSERT(pcount->m_cPointerRefs > 0);
	pcount->m_cPointerRefs--;
	REFCOUNT_UNLOCK;
	return pcount;
}


//============================================================================
//============================================================================
// dismiss ownership of a pointer, potentially freeing
inline void ReleasePointer(Counted *pcount) ALWAYS_INLINE;
inline void ReleasePointer(Counted *pcount)
{
	if(pcount == NULL){
		return;
	}

	REFCOUNT_LOCK;

	UD4("PRELEASE: about to release pointer 0x%x %d/%d", 
			(int)pcount, 
			pcount->m_pref->m_cReferences,
			pcount->m_cPointerRefs
	);

	UD1("PRELEASE: releasing pointer 0x%x %d", (int)pcount, pcount->m_cPointerRefs);
	if (pcount->m_cPointerRefs == 1){
		delete pcount;
		UD1("PKILL: deleteing pointer 0x%x", (int)pcount);
	}
	else{
		pcount->m_cPointerRefs--;
		UD1("PRELEASE: releasing pointer 0x%x %d", (int)pcount, pcount->m_cPointerRefs);
	}

	REFCOUNT_UNLOCK;
}

//============================================================================
//============================================================================
// release the current pointer and take claim of a new one, unless the
// pointers are the same, in which case, do nothing
template <class COUNTED> 
inline void ChangePointer(COUNTED*& pcount1,  COUNTED*& pcount2) ALWAYS_INLINE;
template <class COUNTED> 
inline void ChangePointer(COUNTED*& pcount1,  COUNTED*& pcount2) 
{
	REFCOUNT_LOCK;
	if (pcount1 != pcount2){
		ReleasePointer(pcount1);
		pcount1=pcount2;
		ClaimPointer(pcount2);
	}
	REFCOUNT_UNLOCK;
}


//============================================================================
//============================================================================
// release the claim to the pointer, and return copy as a stack-based object
template<class COUNTED>
inline COUNTED DereferencePointer(COUNTED* pcounted){
	COUNTED counted=*pcounted;
	ReleasePointer(pcounted);
	counted.Release();
	return counted;
}


#endif //#ifndef __REFERENCEPCOUNTING_H__

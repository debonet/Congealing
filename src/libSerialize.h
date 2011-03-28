#ifndef __LIBSERIALIZE_H__
#define __LIBSERIALIZE_H__

#include "String.h"

//==========================================================================
//==========================================================================

template<class T>
inline void SerializePointer(Stream& st,T* const& pt)
{
	pt->Serialize(st);
}

template<class T>
inline void DeserializePointer(Stream& st, T* pt)
{
	pt->Deserialize(st);
}


inline void Serialize(Stream& st, const int& t)
{
	byte* vb=(byte*)&t;
	st.Append(vb[0]);
	st.Append(vb[1]);
	st.Append(vb[2]);
	st.Append(vb[3]);
}

inline void Deserialize(Stream& st, int& t)
{
	byte* vb=(byte*)&t;
	vb[0]=st.Read();
	vb[1]=st.Read();
	vb[2]=st.Read();
	vb[3]=st.Read();
}

inline void Serialize(Stream&st, const Real& t)
{
#if REAL_AS_DOUBLE
	byte* vb=(byte*)&t;
	st.Append(vb[0]);
	st.Append(vb[1]);
	st.Append(vb[2]);
	st.Append(vb[3]);
	st.Append(vb[4]);
	st.Append(vb[5]);
	st.Append(vb[6]);
	st.Append(vb[7]);
#else
	byte* vb=(byte*)&t;
	st.Append(vb[0]);
	st.Append(vb[1]);
	st.Append(vb[2]);
	st.Append(vb[3]);
#endif
}

inline void Deserialize(Stream& st, Real& t)
{
#if REAL_AS_DOUBLE
	byte* vb=(byte*)&t;
	vb[0]=st.Read();
	vb[1]=st.Read();
	vb[2]=st.Read();
	vb[3]=st.Read();
	vb[4]=st.Read();
	vb[5]=st.Read();
	vb[6]=st.Read();
	vb[7]=st.Read();
#else
	byte* vb=(byte*)&t;
	vb[0]=st.Read();
	vb[1]=st.Read();
	vb[2]=st.Read();
	vb[3]=st.Read();
#endif
}


template<class T>
inline void Serialize(Stream& st, const T&t)
{
	t.Serialize(st);
}

template<class T>
inline void Deserialize(Stream& st, T&t)
{
	t.Deserialize(st);
}



#endif // #ifndef __LIBSERIALIZE_H__

#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include "libRefCount.h"
#include <string.h> // for memset_pattern
#include <limits.h> // for INT_MAX
template <class KEY, class VALUE>
class HashTableOf
	: public Counted
{
	friend int Hash(int z);

protected:
	static const int EMPTY = INT_MAX;
	static const int SKIP = INT_MAX-1;

private:
	KEY* m_vKey;
	int* m_vzHash;
	VALUE* m_vValue;
	
	int m_cSize;
	int m_cElts;

	__attribute__((noinline))	int GetHashIndex(const KEY& key, bool bCreate, bool bFillSkip=false)
	{
		bool bHitSkip=false;

		int nHash=Hash(key);
		int c=0;

		int n=(abs(nHash))%m_cSize;
		do{
			if (m_vzHash[n]==EMPTY){
				if (bCreate){
					if (bHitSkip){
						return GetHashIndex(key,bCreate,true);
					}
					else{
						goto insertkey;
					}
				}
				else{
					return -1;
				}
			}
			else if (m_vzHash[n]==SKIP){
				if (bCreate && bFillSkip){
					goto insertkey;
				}
				bHitSkip=true;
			}
			else if (m_vzHash[n]==nHash){
				if (m_vKey[n]==key){
					return n;
				}
			}

			n=(n + 13)%m_cSize;

			c++;
		}
		while(1);

		insertkey: {
			m_vKey[n]=key;
			m_vzHash[n]=nHash;
			m_cElts++;
			// TODO: check for resize required
			return n;
		}
	}



	void EmptyTable(){
		for (int n=0; n<m_cSize; n++){
			m_vzHash[n]=EMPTY;
		}

		// Apple only?
		// WARNING: assumes 4 byte ints!
		// int z=EMPTY;
		// memset_pattern4(m_vzHash,&z,m_cSize);
	}

public:
	HashTableOf()
		: Counted()
	{
		m_cSize=4096;
		m_cElts=0;
		m_vzHash = new int[m_cSize];
		m_vKey = new KEY[m_cSize];
		m_vValue = new VALUE[m_cSize];

		EmptyTable();
	}

	HashTableOf(int c)
		: Counted()
	{
		m_cSize=c;
		m_cElts=0;

		m_vzHash = new int[m_cSize];
		m_vKey = new KEY[m_cSize];
		m_vValue = new VALUE[m_cSize];

		EmptyTable();
	}

	void Set(const KEY& key, const VALUE& value)
	{
		int n=GetHashIndex(key,true);
		m_vValue[n]=value;
	}

	void Unset(const KEY& key)
	{
		int n=GetHashIndex(key,true);
		if (n!=-1){
			m_vzHash[n]=SKIP;
			m_cElts--;
		}
	}

	const VALUE& Get(const KEY& key, const VALUE& valueDefault)
	{
		int n=GetHashIndex(key,false);

		if (n==-1){
			return valueDefault;
		}

		return m_vValue[n];
	}

	bool GetInto(const KEY& key, VALUE* pvalue)
	{
		int n=GetHashIndex(key,false);

		if (n==-1){
			return false;
		}

		*pvalue=m_vValue[n];
		return true;
	}

	bool GetNext(int* pn, KEY *pkey, VALUE* pvalue){
		while((*pn) < m_cSize) {
			if (m_vzHash[*pn] == EMPTY || m_vzHash[*pn] == SKIP){
				(*pn)++;
			}
			else{
				*pkey=m_vKey[*pn];
				*pvalue=m_vValue[*pn];
				(*pn)++;
				return true;
			}
		}

		return false;
	}

};

#define forhash(_key,_value,_hash)																						\
	for (																																				\
		 int _forhash##__line__=0;																								\
		_hash.GetNext(&_forhash##__line__,&_key,&_value);													\
	)

#define forhash2(_keytype,_key,_valuetype,_value,_hash,_code)									\
	{																																						\
		_keytype _key;																														\
		_valuetype _value;																												\
		int _forhash##__line__=0;																									\
		while(_hash.GetNext(&_forhash##__line__,&_key,&_value)){									\
			_code;																																	\
		}																																					\
	}


//============================================================================
// STRING HASH TABLES
//============================================================================
/*
djb2

this algorithm (k=33) was first reported by dan bernstein many years ago in
comp.lang.c. another version of this algorithm (now favored by bernstein) uses
xor: hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33 (why it works
better than many other constants, prime or not) has never been adequately
explained.

Adapted from: http://www.cse.yorku.ca/~oz/hash.html
*/
inline int Hash(const char* pch)
{
	unsigned long zHash = 5381;
	int ch;

	while ((ch = *pch++) != 0){
		zHash = ((zHash << 5) + zHash) + ch; /* zHash * 33 + c */
	}

	return zHash;
}

class String;
typedef HashTableOf<String,int> HashSZ;
typedef HashTableOf<String,String> HashSS;
typedef HashTableOf<String,Real> HashSR;
typedef HashTableOf<String,byte> HashSY;
typedef HashTableOf<String,bool> HashSB;


//============================================================================
// INT HASH TABLES
//============================================================================
inline int Hash(int z)
{
	if (z==HashTableOf<int,int>::EMPTY || z==HashTableOf<int,int>::SKIP){
		z-=20;
	}

	return z;
}

typedef HashTableOf<int,int> HashZZ;
typedef HashTableOf<int,String> HashZS;
typedef HashTableOf<int,Real> HashZR;
typedef HashTableOf<int,byte> HashZY;
typedef HashTableOf<int,bool> HashZB;


//============================================================================
// REAL HASH TABLES
//============================================================================
inline int Hash(Real r)
{
	int z=r*123345;
	return Hash(z);
}

typedef HashTableOf<Real,int> HashRZ;
typedef HashTableOf<Real,String> HashRS;
typedef HashTableOf<Real,Real> HashRR;
typedef HashTableOf<Real,byte> HashRY;
typedef HashTableOf<Real,bool> HashRB;



#endif //#ifndef __HASHTABLE_H__

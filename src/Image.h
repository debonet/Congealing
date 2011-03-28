#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "Point2D.h"
#include "SourceMemory.h"

template <class DATA, int DIMENSIONALITY>
class ImageOf 
	: public SourceMemoryOf<DATA,DIMENSIONALITY> {
public:

};


template <class DATA>
class ImageOf<DATA,2>
 : public SourceMemoryOf<DATA,2> 
{
	typedef int PRECISION;
public:
	virtual String Describe() const
	{
		return (
			_LINE + "Image2D" + LINE_ +
			this->DescribeCommon()
		);
	}

	SOURCE_ACTUALS_2D;

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	inline void Get(DATA& dataOut, const int &nX, const int& nY) const 
	{
		const int cX=this->m_ptSize.X();
		const int cY=this->m_ptSize.Y();

		if (nX<0 || nX>cX-1 || nY<0 || nY>cY-1){
		  dataOut=DATA(0);
		}
		else{
		  dataOut = this->m_vdata[nY*cX + nX];
		}

		//int nX2=::Bound(nX,0,cX-1);
		//int nY2=::Bound(nY,0,cY-1);

		//		dataOut = this->m_vdata[nY2*cX + nX2];
	}


	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	virtual int CX() const
	{
		return this->Size().X();
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	virtual int CY() const
	{
		return this->Size().Y();
	}

};




template <class DATA>
class ImageOf<DATA,3>
 : public SourceMemoryOf<DATA,3> 
{
	typedef int PRECISION;
public:
	ImageOf()
		: SourceMemoryOf<DATA,3>()
	{
	}

	ImageOf(const ImageOf<DATA,3> &i)
		: SourceMemoryOf<DATA,3>(i)
	{
		D3("COPY?");
	}

	virtual String Describe() const
	{
		return (
			_LINE + "Image3D" +	LINE_ +
			this->DescribeCommon()
		);
	}

	SOURCE_ACTUALS_3D;

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	inline void Get(DATA& dataOut, const int& nX, const int& nY, const int& nZ) const
	{
		const int& cX=this->m_ptSize.X();
		const int& cY=this->m_ptSize.Y();
		const int& cZ=this->m_ptSize.Z();

		if (nX<0 || nX>cX-1 || nY<0 || nY>cY-1 || nZ<0 || nZ>cZ-1){
		  dataOut=DATA(0);
		}
		else{
		  dataOut = this->m_vdata[(nZ*cY+nY)*cX + nX];
		}

		/*
		const int nX2=::Bound(nX,0,cX-1);
		const int nY2=::Bound(nY,0,cY-1);
		const int nZ2=::Bound(nZ,0,cZ-1);
		const int n=(nZ2*cY+nY2)*cX + nX2;

		dataOut = this->m_vdata[n];
		*/
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	virtual int CX() const
	{
		return this->Size().X();
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	virtual int CY() const
	{
		return this->Size().Y();
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	virtual int CZ() const
	{
		return this->Size().Z();
	}

};


#include "Pixels.h"
typedef ImageOf<Pixel8BitGrey,2> GreyImage;
typedef ImageOf<Pixel8BitGrey,3> GreyVolume;
typedef ImageOf<Pixel12BitGrey,2> Grey12Image;
typedef ImageOf<Pixel12BitGrey,3> Grey12Volume;

#endif //#ifndef __IMAGE_H__

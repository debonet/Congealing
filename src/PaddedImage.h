#ifndef __PADDEDIMAGE_H__
#define __PADDEDIMAGE_H__

#include "Point2D.h"
#include "SourceMemory.h"

template <class DATA, int DIMENSIONALITY>
class PaddedImageBaseOf 
	: public SourceMemoryOf<DATA,DIMENSIONALITY> 
{
protected:
	int m_cPad;
	typedef PointOf<DIMENSIONALITY,int> POINT;

private:

	//--------------------------------------------------------------------------
	// deactivated methods from SourceMemoryOf<> class
	//--------------------------------------------------------------------------
	void Allocate(const PointOf<DIMENSIONALITY,int>& ptSize){}
	void Fill(const DATA& data){}
	const DATA *ReadData() const {return NULL;}
	DATA *WriteData() {return NULL;}

public:
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	virtual String Describe() const
	{
		return (
			_LINE + "Image2D" + LINE_ +
			(_INDENT + "padding:" + m_cPad + LINE_) + 
			this->DescribeCommon()
		);
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	template < class DATA_IN, class PRECISION>
	void Pad(
		const SourceOf<DATA_IN, DIMENSIONALITY, PRECISION> &srcIn, 
		int cPad,
		PaddingType pad=Repeat, 
		bool bPrepared=false
	){
		m_cPad=cPad;

		POINT ptStorage=srcIn.Size()+(2*m_cPad);

		D("allocating %d", ptStorage.CVolume()); ptStorage.Describe();

		SourceMemoryOf<DATA,DIMENSIONALITY>::Allocate(ptStorage);

		// fix the size
		this->m_ptSize=srcIn.Size();


		POINT ptStart=POINT(-m_cPad);
		POINT ptEnd=srcIn.Size()+m_cPad;

		switch (pad){
		case Zero:{
			forpoint(POINT,pt,ptStart,ptEnd){
				if (pt>=0 || pt < srcIn.Size()){
					DATA_IN data;
					srcIn.GetPoint(data,pt);
					this->SetPoint(pt,data);
				}
				else{
					this->SetPoint(pt,DATA(0));
				}
			}
			break;
		}

		case Repeat:{
			forpoint(POINT,pt,ptStart,ptEnd){
				DATA_IN data;
				POINT ptIn=pt.Bound(PointOf<DIMENSIONALITY,PRECISION>(0),srcIn.Size());
				srcIn.GetPoint(data,ptIn);
				this->SetPoint(pt,data);
			}
			break;
		}

		case Wrap:{
			forpoint(POINT,pt,ptStart,ptEnd){
				DATA_IN data;
				POINT ptIn=AbsModulo(pt,srcIn.Size());
				srcIn.GetPoint(data,ptIn);
				this->SetPoint(pt,data);
			}
			break;
		}

		default:{
			CONGEAL_ERROR("not implemented yet");
			break;
		}
		}
	}

};


template <class DATA, int DIMENSIONALITY>
class PaddedImageOf 
	: public PaddedImageBaseOf<DATA,DIMENSIONALITY> 
{
private:
	PaddedImageOf(){}; // no creating one of these!
};


template <class DATA>
class PaddedImageOf<DATA,2>
	: public PaddedImageBaseOf<DATA,2>
{
	typedef int PRECISION;
public:
	SOURCE_ACTUALS_2D;

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void Get(DATA& dataOut, int nX, int nY) const
	{
		int cX=this->m_ptSize.X()+2*this->m_cPad;

		dataOut = this->m_vdata[(nY+this->m_cPad)*cX + (nX+this->m_cPad)];
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void Set(int nX, int nY, const DATA& data) const
	{
		int cX=this->m_ptSize.X()+2*this->m_cPad;

		this->m_vdata[(nY+this->m_cPad)*cX + (nX+this->m_cPad)]=data;
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
class PaddedImageOf<DATA,3>
	: public PaddedImageBaseOf<DATA,3>
{
	typedef int PRECISION;
public:
	SOURCE_ACTUALS_3D;

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void Get(DATA& dataOut, int nX, int nY, int nZ) const
	{
		int cX=this->m_ptSize.X()+2*this->m_cPad;
		int cY=this->m_ptSize.Y()+2*this->m_cPad;

		dataOut = this->m_vdata[((nZ+this->m_cPad)*cY + (nY+this->m_cPad))*cX + (nX+this->m_cPad)];
	}

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	void Set(int nX, int nY, int nZ, const DATA& data) const
	{
		int cX=this->m_ptSize.X()+2*this->m_cPad;
		int cY=this->m_ptSize.Y()+2*this->m_cPad;

		this->m_vdata[((nZ+this->m_cPad)*cY + (nY+this->m_cPad))*cX + (nX+this->m_cPad)]=data;
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
typedef PaddedImageOf<Pixel8BitGrey,2> GreyPaddedImage;
typedef PaddedImageOf<Pixel8BitGrey,3> GreyPaddedVolume;

#endif //#ifndef __PADDEDIMAGE_H__

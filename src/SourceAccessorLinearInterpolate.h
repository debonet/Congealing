// $Id$

#ifndef __SOURCEACCESSORLINEARINTERPOLATE_H__
#define __SOURCEACCESSORLINEARINTERPOLATE_H__

#include "SourceGenerics.h"

#include "Promotion.h"
#include <math.h>

//============================================================================
// forward declarations of interpolation functions
//============================================================================
template <class DATA>
inline PROMOTION(Real,DATA) LinearInterpolate (
	const Real& x, 
	const DATA& data0, const DATA& data1
);

//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorLinearInterpolateOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};


//============================================================================
// 2D
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorLinearInterpolateOf<DATA, 2, PRECISION, SOURCE>
	: public SourceAccessorOf<DATA, 2, PRECISION, SOURCE>
{
public:
	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorLinearInterpolateOf(2D)" + LINE_ + 
			this->DescribeCommon()
		);
	}

private:
	PROMOTION(DATA,PRECISION) InterpolateOnY(
		const PRECISION &rX, const PRECISION &rY
	) const
	{
		// note center is at pxl11, not pxl00
		DATA data0; ::Get(*(this->m_psource),data0,(PRECISION)((int)(rX)),(PRECISION)((int)(rY)));
		DATA data1; ::Get(*(this->m_psource),data1,(PRECISION)((int)(rX+1)),(PRECISION)((int)(rY)));
		return LinearInterpolate(rX-int(rX),	data0,data1);
	}

public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		PROMOTION(DATA,PRECISION) data0 = InterpolateOnY(rX,rY  );
		PROMOTION(DATA,PRECISION) data1 = InterpolateOnY(rX,rY+1);

		dataOut=LinearInterpolate(rY-int(rY),	data0,data1);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		ERROR("NOT IMPLEMENTED");
	}

};





//============================================================================
// 3d
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorLinearInterpolateOf<DATA, 3, PRECISION, SOURCE>
	: public SourceAccessorOf<DATA, 3, PRECISION, SOURCE>
{
public:
	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorLinearInterpolateOf(3D)" + LINE_ + 
			this->DescribeCommon() 
		);
	}

private:
	PROMOTION(DATA,Real) InterpolateOnYZ(
		const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ
	) const
	{
		// note center is at pxl11, not pxl00
		DATA data0; ::Get(*(this->m_psource),	data0, (PRECISION)((int)(rX)),(PRECISION)((int)(rY)),(PRECISION)((int)(rZ)));
		DATA data1; ::Get(*(this->m_psource),	data1, (PRECISION)((int)(rX+1)),(PRECISION)((int)(rY)),(PRECISION)((int)(rZ)));

		return LinearInterpolate(rX-int(rX),	data0,data1);
	}


	PROMOTION(DATA,Real) InterpolateOnZ(
		const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ
	) const
	{
		PROMOTION(DATA,Real) data0 = InterpolateOnYZ(rX,rY  ,rZ);
		PROMOTION(DATA,Real) data1 = InterpolateOnYZ(rX,rY+1,rZ);

		return LinearInterpolate(rY-int(rY),	data0,data1);
	}


public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		PROMOTION(DATA,Real) data0 = InterpolateOnZ(rX,rY,rZ  );
		PROMOTION(DATA,Real) data1 = InterpolateOnZ(rX,rY,rZ+1);

		dataOut = LinearInterpolate(rZ-int(rZ), data0, data1);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		ERROR("NOT IMPLEMENTED");
	}
};

template <class DATA>
inline PROMOTION(Real,DATA) LinearInterpolate (
	const Real& x, 
	const DATA& data0, const DATA& data1
) 
{
	return (Real(1.0)-x)*data0 + x * data1;
}

MAKE_INTERPOLANT_ASSISTANT( LinearInterpolation, SourceAccessorLinearInterpolateOf );


#endif // __SOURCEACCESSORLINEARINTERPOLATE_H__

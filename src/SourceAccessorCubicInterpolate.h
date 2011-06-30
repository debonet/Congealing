// $Id$

#ifndef __SOURCEACCESSORCUBICINTERPOLATE_H__
#define __SOURCEACCESSORCUBICINTERPOLATE_H__

#include "SourceGenerics.h"

#include "Promotion.h"
#include <math.h>

bool DoSpecialDebug();

//============================================================================
// forward declarations of interpolation functions
//============================================================================
template <class DATA>
inline PROMOTION(Real,DATA) CubicInterpolate (
	const Real& x, 
	const DATA& data0, const DATA& data1, const DATA& data2, const DATA& data3
);

//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorCubicInterpolateOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};


//============================================================================
// 2D
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorCubicInterpolateOf<DATA, 2, PRECISION, SOURCE>
	: public SourceAccessorOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorCubicInterpolateOf (2D)" + LINE_ +
			this->DescribeCommon()
		);
	}

private:
	inline PROMOTION(DATA,PRECISION) InterpolateOnY(
		const PRECISION &rX, const PRECISION &rY
	) const
	{
		// note center is at pxl11, not pxl00
		DATA data0;		::Get(*(this->m_psource),	data0, (PRECISION)((int)(rX-1)),(PRECISION)((int)(rY)));
		DATA data1;		::Get(*(this->m_psource),	data1, (PRECISION)((int)(rX+0)),(PRECISION)((int)(rY)));
		DATA data2;		::Get(*(this->m_psource),	data2, (PRECISION)((int)(rX+1)),(PRECISION)((int)(rY)));
		DATA data3;		::Get(*(this->m_psource),	data3, (PRECISION)((int)(rX+2)),(PRECISION)((int)(rY)));

		return CubicInterpolate(rX-int(rX),	data0,data1,data2,data3);
	}

public:

	inline void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		PROMOTION(DATA,PRECISION) data0 = InterpolateOnY((PRECISION)((int)(rX)),(PRECISION)((int)(rY-1)));
		PROMOTION(DATA,PRECISION) data1 = InterpolateOnY((PRECISION)((int)(rX)),(PRECISION)((int)(rY))  );
		PROMOTION(DATA,PRECISION) data2 = InterpolateOnY((PRECISION)((int)(rX)),(PRECISION)((int)(rY+1)));
		PROMOTION(DATA,PRECISION) data3 = InterpolateOnY((PRECISION)((int)(rX)),(PRECISION)((int)(rY+2)));

		dataOut=CubicInterpolate(rY-int(rY),	data0,data1,data2,data3);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		CONGEAL_ERROR("NOT IMPLEMENTED");
	}

};





//============================================================================
// 3d
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorCubicInterpolateOf<DATA, 3, PRECISION, SOURCE>
	: public SourceAccessorOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

	virtual String Describe() const
	{
		return (
			_LINE + "SourceAccessorCubicInterpolateOf (3D)" + LINE_ +
			this->DescribeCommon()
		);
	}

private:
	inline PROMOTION(DATA,Real) InterpolateOnYZ(
		const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ
	) const 
	{
		// note center is at pxl11, not pxl00
		DATA data0;		::Get( *(this->m_psource), data0, (PRECISION)((int)(rX-1)), (PRECISION)((int)(rY)), (PRECISION)((int)(rZ)));
		DATA data1;		::Get( *(this->m_psource), data1, (PRECISION)((int)(rX+0)), (PRECISION)((int)(rY)), (PRECISION)((int)(rZ)));
		DATA data2;		::Get( *(this->m_psource), data2, (PRECISION)((int)(rX+1)), (PRECISION)((int)(rY)), (PRECISION)((int)(rZ)));
		DATA data3;		::Get( *(this->m_psource), data3, (PRECISION)((int)(rX+2)), (PRECISION)((int)(rY)), (PRECISION)((int)(rZ)));

		return CubicInterpolate(rX-int(rX),	data0,data1,data2,data3);
	}


	inline PROMOTION(DATA,Real) InterpolateOnZ(
		const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ
	) const 
	{
		PROMOTION(DATA,Real) data0 = InterpolateOnYZ(rX,rY-1,rZ);
		PROMOTION(DATA,Real) data1 = InterpolateOnYZ(rX,rY  ,rZ);
		PROMOTION(DATA,Real) data2 = InterpolateOnYZ(rX,rY+1,rZ);
		PROMOTION(DATA,Real) data3 = InterpolateOnYZ(rX,rY+2,rZ);

		return CubicInterpolate(rY-int(rY),	data0,data1,data2,data3);
	}


public:
	inline void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const 
	{
		PROMOTION(DATA,Real) data0 = InterpolateOnZ(rX,rY,rZ-1);
		PROMOTION(DATA,Real) data1 = InterpolateOnZ(rX,rY,rZ  );
		PROMOTION(DATA,Real) data2 = InterpolateOnZ(rX,rY,rZ+1);
		PROMOTION(DATA,Real) data3 = InterpolateOnZ(rX,rY,rZ+2);

		dataOut= CubicInterpolate(rZ-int(rZ), data0, data1, data2, data3);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		CONGEAL_ERROR("NOT IMPLEMENTED");
	}
};


// using formulae from
//  http://en.wikipedia.org/wiki/Cubic_interpolation#Interpolation_on_the_unit_interval_without_exact_derivatives
// computes cubic interpolation of 4 points at point x,y
// note: variable indicies off by +1, so centered at data1 == f(x=0)
template <class DATA>
inline PROMOTION(Real,DATA) CubicInterpolate (
	const Real& x,
	const DATA& data0, const DATA& data1, const DATA& data2, const DATA& data3
)  __attribute__((always_inline));

template <class DATA>
inline PROMOTION(Real,DATA) CubicInterpolate (
	const Real& x,
	const DATA& data0, const DATA& data1, const DATA& data2, const DATA& data3
)
{
	const PROMOTION(Real,DATA) x2 = x * x;

	return Real(.5) * (
		+ (x*((2-x)*x-1))     * data0
		+ (x2*(3*x-5)+2)      * data1
		+ (x*((4-3*x)*x+1))   * data2
		+ ((x-1)*x2)          * data3);

}



MAKE_INTERPOLANT_ASSISTANT( CubicInterpolation, SourceAccessorCubicInterpolateOf );


#endif // __SOURCEACCESSORCUBICINTERPOLATE_H__

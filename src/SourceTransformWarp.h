// $Id$

#ifndef __SOURCETRANSFORMWARP_H__
#define __SOURCETRANSFORMWARP_H__

#include "SourceGenerics.h"
#include "SourceMemory.h"
#include "SourceTransform.h"
#include "SourceTransformScale.h"
#include "SourceAccessorAperature.h"
#include "SourceAccessorNearestNeighbor.h"
#include "SourceAccessorLinearInterpolate.h"
#include "SourceAccessorCubicInterpolate.h"
#include <math.h>
#include "libInterpolatedSource.h"

#undef DEBUG
#define DEBUG 0
#include "libDebug.h"


//============================================================================
//============================================================================
template <int DIMENSIONALITY>
class WarpControlPointsBaseOf
	: public SourceMemoryOf<PointOf<DIMENSIONALITY,Real>,DIMENSIONALITY> 
{
public:
};


//============================================================================
//============================================================================
template <int DIMENSIONALITY>
class WarpControlPointsOf
	: public WarpControlPointsBaseOf<DIMENSIONALITY>
{
};


//============================================================================
//============================================================================
template <>
class WarpControlPointsOf<2>
	: public WarpControlPointsBaseOf<2>
{
	typedef Point2DReal DATA;
	typedef int PRECISION;

public:
	SOURCE_ACTUALS_2D;

	virtual String Describe() const
	{
		String s;
		Real cX=this->m_ptSize.X();
		Real cY=this->m_ptSize.Y();
		if (cX * cY < 32){
			for (int nY=0; nY<cY; nY++){
				s=s+_LINE;
				for (int nX=0; nX<cX; nX++){
					Point2DReal pt;
					this->Get(pt,nX,nY);
					s=s+ pt.X() + "," + pt.Y() + INDENT;
				}
				s=s+LINE_;
			}
		}

		return (
			_LINE + "WarpControlPoints" + LINE_ + 
			this->DescribeCommon() +
			(_INDENT + s)
		);
	}


	void Get(Point2DReal& dataOut, Real rX, Real rY) const
	{
		Real cX=this->m_ptSize.X();
		Real cY=this->m_ptSize.Y();

		if ( (rX<0) || (rY<0) || (rX>=cX) || (rY>=cY)){
			dataOut=Point2DReal(0.);
			return;
		}

		SourceMemoryOf<Point2DReal,2>::Get(dataOut,rX,rY);
	}

	void Set(Real rX, Real rY, const Point2DReal& data)
	{
		Real cX=this->m_ptSize.X();
		Real cY=this->m_ptSize.Y();

		if ( (rX<0) || (rY<0) || (rX>=cX) || (rY>=cY)){
			return;
		}

		return SourceMemoryOf<Point2DReal,2>::Set(rX,rY,data);
	}
};

//============================================================================
//============================================================================
template <>
class WarpControlPointsOf<3>
	: public WarpControlPointsBaseOf<3>
{
	typedef Point3DReal DATA;
	typedef int PRECISION;

public:
	SOURCE_ACTUALS_3D;

	virtual String Describe() const
	{
		String s;
		Real cX=this->m_ptSize.X();
		Real cY=this->m_ptSize.Y();
		Real cZ=this->m_ptSize.Z();

		if (cX * cY *cZ < 128){
			for (int nZ=0; nZ<cZ; nZ++){
				s=s+_LINE;
				for (int nY=0; nY<cY; nY++){
				s=s+_LINE;
					for (int nX=0; nX<cX; nX++){
						Point3DReal pt;
						this->Get(pt,nX,nY,nZ);
						s=s+ pt.X() + "," + pt.Y() + "," + pt.Z() + INDENT;
					}
					s=s+LINE_;
				}
				if (nZ!=cZ-1){
					s=s+LINE_;
				}
			}
		}

		return (
			_LINE + "WarpControlPoints(3D)" + LINE_ + 
			this->DescribeCommon() + 
			(_INDENT2 + s)
		);
	}

	void Get(Point3DReal& dataOut, int rX, int rY, int rZ) const
	{
		int cX=this->m_ptSize.X();
		int cY=this->m_ptSize.Y();
		int cZ=this->m_ptSize.Z();

		if ( (rX<0) || (rY<0) || (rX>=cX) || (rY>=cY) || (rZ<0) || (rZ>=cZ)){
			dataOut=Point3DReal(0.);
			return;
		}

		SourceMemoryOf<Point3DReal,3>::Get(dataOut,rX,rY,rZ);
	}

	void Set(int rX, int rY, int rZ, const Point3DReal& data)
	{
		int cX=this->m_ptSize.X();
		int cY=this->m_ptSize.Y();
		int cZ=this->m_ptSize.Z();

		if ( (rX<0) || (rY<0) || (rX>=cX) || (rY>=cY) || (rZ<0) || (rZ>=cZ)){
			return;
		}

		return SourceMemoryOf<Point3DReal,3>::Set(rX,rY,rZ,data);
	}

};

//============================================================================
//============================================================================
typedef WarpControlPointsOf<2> WarpControlPoints2D;
typedef WarpControlPointsOf<3> WarpControlPoints3D;

typedef SourceAccessorCubicInterpolateOf<Point2DReal, 2, int, WarpControlPoints2D > WarpfieldCubicSpline2D;
typedef SourceAccessorCubicInterpolateOf<Point2DReal, 3, int, WarpControlPoints3D > WarpfieldCubicSpline3D;

typedef SourceAccessorCubicInterpolateOf<Point2DReal, 2, Real, WarpControlPoints2D > WarpfieldCubicSpline2DReal;
typedef SourceAccessorCubicInterpolateOf<Point2DReal, 3, Real, WarpControlPoints3D > WarpfieldCubicSpline3DReal;




//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformWarpBaseOf
	: public SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	typedef PointOf<DIMENSIONALITY, Real> WARPPOINT;
	typedef WarpControlPointsOf<DIMENSIONALITY> WARPCONTROLPOINTS;
	typedef SourceAccessorCubicInterpolateOf<WARPPOINT, DIMENSIONALITY, Real,WARPCONTROLPOINTS> WARPINTERPOLATEDCONTROLPOINTS;
	typedef SourceTransformScaleOf<WARPPOINT, DIMENSIONALITY, PRECISION, WARPINTERPOLATEDCONTROLPOINTS> WARPINTERPOLATEDCONTROLPOINTSSCALED;
	typedef SourceAccessorAperatureOf<WARPPOINT, DIMENSIONALITY, PRECISION, WARPINTERPOLATEDCONTROLPOINTSSCALED> WARPFIELD;

	mutable WARPFIELD* m_pwarpfield;
	mutable bool m_bDeactivated;

public:
	typedef PointOf<DIMENSIONALITY, int> POINT;
	typedef PointOf<DIMENSIONALITY, Real> WarpPoint;
	typedef WarpControlPointsOf<DIMENSIONALITY> WarpControlPoints;
	typedef SourceAccessorCubicInterpolateOf<WARPPOINT, DIMENSIONALITY, Real,WARPCONTROLPOINTS> WarpInterpolatedControlPoints;
	typedef SourceTransformScaleOf<WARPPOINT, DIMENSIONALITY, PRECISION, WARPINTERPOLATEDCONTROLPOINTS> Warpfield;

	SourceTransformWarpBaseOf()
		: SourceTransformOf<DATA, DIMENSIONALITY,PRECISION, SOURCE> ()
		, m_pwarpfield(NULL)
	{
		AllocateWarpfield(PointOf<DIMENSIONALITY,int>(1),Cubic);
	}

	virtual ~SourceTransformWarpBaseOf()
	{
		if (Counted::AboutToDie()){
			ReleasePointer(m_pwarpfield);
		}
	}

	void AllocateWarpfield(PointOf<DIMENSIONALITY,int> ptSizeWarp, InterpolationType it=Cubic)
	{
		WarpControlPointsOf<DIMENSIONALITY> *pcp=new WarpControlPointsOf<DIMENSIONALITY>;
		pcp->Allocate(ptSizeWarp);

		UD("ORIG %s",pcp->Size().Describe().VCH()) ;
		pcp->Fill(PointOf<DIMENSIONALITY,Real>(0.));

		ReleasePointer(m_pwarpfield);
		m_pwarpfield=ClaimPointer(
			Aperature(
				POINT(0),
				POINT(1),
				ScaleTo(
					POINT(1.),
					CubicInterpolation(pcp))));

		ReleasePointer(pcp);
	}

	void SetWarpfieldInterpolation(InterpolationType it=Cubic)
	{
		WARPCONTROLPOINTS* pcp=PWarpControlPoints();
		ClaimPointer(pcp);

		ReleasePointer(m_pwarpfield);
		m_pwarpfield=ClaimPointer(
			Aperature(
				POINT(0),
				POINT(1),
				ScaleTo(
					POINT(1.),
					CubicInterpolation(pcp))));
		ReleasePointer(pcp);
	}

	void SetWarpControlPoints(WARPCONTROLPOINTS* pwarpcontrolpoints)
	{
		m_pwarpfield->PSource()->PSource()->SetSource(pwarpcontrolpoints);
	}

	WARPCONTROLPOINTS* PWarpControlPoints() const
	{
		return m_pwarpfield->PSource()->PSource()->PSource();
	}


	WARPINTERPOLATEDCONTROLPOINTS* PInterpolatedWarpControlPoints() const
	{
		return m_pwarpfield->PSource()->PSource();
	}

	virtual String Describe() const
	{
		return (
			_LINE + "SourceTransformWarp" + LINE_ + 
			(_INDENT + "with field: " + LINE_)  +
			(_INDENT2 + m_pwarpfield->Describe() ) + 
			this->DescribeCommon() 
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		m_pwarpfield->RegisterDataAsParameters(reg);
	}

	void PrepareForAccessAction() const
	{
		CONGEAL_ASSERTf(m_pwarpfield,"Null warpfield");
		// prepare the warp for access to allow all of the warp's
		// sources to compute their final sizes
		m_pwarpfield->PrepareForAccess();

		// set the aperature
		m_pwarpfield->SetAperature(
			POINT(0),
			POINT(this->PSource()->Size())
		);

		// then adjust the scaling of the warp
		m_pwarpfield->PSource()->SetScale(
			PointOf<DIMENSIONALITY,Real>(this->PSource()->Size())
			/ PWarpControlPoints()->Size()
		);

		// and allow it to readjust
		m_pwarpfield->PrepareForAccess();

		// and finally set our own size;
		this->m_ptSize=this->PSource()->Size();

//		UD("%s",PWarpControlPoints()->Size().Describe().VCH());
		this->m_bDeactivated=PWarpControlPoints()->Size().X()<4;
//		UD("WARPFIELD: %s", this->m_bDeactivated?"inactive" : "ACTIVE");
	}


	static String SerializationId()
	{
		return String("SourceTransformWarp(") + DIMENSIONALITY +"D)";
	}

	void SerializeSelf(Stream& st) const
	{ 
		::SerializePointer(st,m_pwarpfield->PSource());
	}

	void DeserializeSelf(Stream& st)
	{
		::DeserializePointer(st,m_pwarpfield->PSource());
	}
};







//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformWarpOf 
	: public SourceTransformWarpBaseOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};



//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformWarpOf<DATA,2,PRECISION,SOURCE> 
	: public SourceTransformWarpBaseOf<DATA, 2, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_2D;

private:
	typedef PointOf<DIMENSIONALITY, Real> WARPPOINT;
	typedef SourceOf<WARPPOINT, DIMENSIONALITY, PRECISION> WARPCONTROLPOINTS;
	typedef SourceTransformScaleOf<WARPPOINT, DIMENSIONALITY, PRECISION, WARPCONTROLPOINTS> WARPFIELD;

public:
	void Get(DATA& dataOut, const PRECISION& rX, const PRECISION& rY) const
	{
		if (this->m_pwarpfield->Size().X() < 4){
			return ::Get(
				*(this->m_psource),
				dataOut, 
				rX,
				rY
			);
		}

		WARPPOINT dpt;
		this->m_pwarpfield->Get(dpt,rX,rY);

		::Get(
			*(this->m_psource),
			dataOut, 
			(rX - dpt.X()*this->Size().X()),
			(rY - dpt.Y()*this->Size().Y())
		);
	}

	void Set(const PRECISION& rX, const PRECISION& rY, const DATA& data)
	{
		WARPPOINT dpt;
		this->m_pwarpfield->Get(dpt,rX,rY);

		::Set(
			*(this->m_psource),
			(rX - dpt.X()*this->Size().X()),
			(rY - dpt.Y()*this->Size().Y()),
			data
		);
	}
};

//============================================================================
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformWarpOf<DATA, 3, PRECISION, SOURCE>
	: public SourceTransformWarpBaseOf<DATA, 3, PRECISION, SOURCE>
{
public:
	SOURCE_ACTUALS_3D;

private:
	typedef PointOf<DIMENSIONALITY, Real> WARPPOINT;
	typedef SourceOf<WARPPOINT, DIMENSIONALITY, PRECISION> WARPCONTROLPOINTS;
	typedef SourceTransformScaleOf<WARPPOINT, DIMENSIONALITY, PRECISION, WARPCONTROLPOINTS> WARPFIELD;

public:
	void Get(DATA& dataOut, const PRECISION& rX, const PRECISION& rY, const PRECISION& rZ) const
	{
		// if deactivated, pass right through without doing calculations
		if (this->m_bDeactivated){
			return ::Get(
				*(this->m_psource),
				dataOut, 
				rX,
				rY,
				rZ
			);
		}

		WARPPOINT dpt;
		this->m_pwarpfield->Get(dpt,rX,rY,rZ);

		::Get(
			*(this->m_psource),
			dataOut, 
			(rX - dpt.X()*this->Size().X()),
			(rY - dpt.Y()*this->Size().Y()),
			(rZ - dpt.Z()*this->Size().Z())
		);
	}

	void Set(const PRECISION& rX, const PRECISION& rY, const PRECISION& rZ, const DATA& data)
	{
		WARPPOINT dpt;
		this->m_pwarpfield->Get(dpt,rX,rY,rZ);

		::Set(
			*(this->m_psource),
			(rX - dpt.X()*this->Size().X()),
			(rY - dpt.Y()*this->Size().Y()),
			(rZ - dpt.Z()*this->Size().Z()),
			data
		);
	}

};



//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
void IncreaseWarpfieldControlPointResolution(
	SourceTransformWarpOf<DATA,DIMENSIONALITY,PRECISION,SOURCE> &srcWarp, 
	const PointOf<DIMENSIONALITY,Real> &ptSize, 
	int nInterpType=3
){
	UD("increasing resolution of warpfield control points");
	srcWarp.PrepareForAccess();
	srcWarp.SetWarpControlPoints(
		RasterizeInto(
			HandoffPointer(new typename SourceTransformWarpOf<DATA,DIMENSIONALITY,PRECISION,SOURCE>::WarpControlPoints),
			Aperature(
				PointOf<DIMENSIONALITY,Real>(0),
				ptSize,
				ScaleTo(
					ptSize,
					srcWarp.PInterpolatedWarpControlPoints()))));

	srcWarp.SetWarpfieldInterpolation(
		InterpolationType(nInterpType));
}

#endif // __SOURCETRANSFORMWARP_H__

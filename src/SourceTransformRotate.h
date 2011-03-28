// $Id$

#ifndef __SOURCETRANSFORMROTATE_H__
#define __SOURCETRANSFORMROTATE_H__

#include "SourceGenerics.h"
#include <math.h>

#undef DEBUG 
#define DEBUG 0
#include "libDebug.h"

//============================================================================
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceTransformRotateOf 
	: public SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{};




//============================================================================
// 2d
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformRotateOf<DATA,2,PRECISION,SOURCE>
	: public SourceTransformOf<DATA, 2, PRECISION, SOURCE>
{
	Real m_rAngle;
	mutable Real m_rCos;
	mutable Real m_rSin;
	mutable Real m_rXCenter;
	mutable Real m_rYCenter;

public:
	SOURCE_ACTUALS_2D;

	SourceTransformRotateOf()
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
		, m_rAngle(0)
	{}

	virtual String Describe() const
	{
		return (
			_LINE + SerializationId() + LINE_ + 
			(_INDENT + "degrees: " + m_rAngle + LINE_) + 
			this->DescribeCommon()
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		reg.Register(&m_rAngle);
	}


	Real GetAngle() const
	{
		return m_rAngle;
	}

	void SetAngle(Real r)
	{
		m_rAngle=r;
	}

	static String SerializationId() 
	{
		return "SourceTransformRotateOf(2D)";
	}

	void SerializeSelf(Stream& st) const
	{ 
		::Serialize(st,m_rAngle);
	}

	void DeserializeSelf(Stream &st)
	{
		Deserialize(st,m_rAngle);
	}

	void Get(DATA& dataOut, const PRECISION& rX, const PRECISION& rY) const
	{
		Real dX=rX-m_rXCenter;
		Real dY=rY-m_rYCenter;

		::Get(
			*(this->m_psource),
			dataOut,
			(( m_rCos * dX + m_rSin * dY) + m_rXCenter),
			((-m_rSin * dX + m_rCos * dY) + m_rYCenter)
		);
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		Real dX=rX-m_rXCenter;
		Real dY=rY-m_rYCenter;

		::Set(
			*this->m_psource,
			(( m_rCos * dX + m_rSin * dY) + m_rXCenter),
			((-m_rSin * dX + m_rCos * dY) + m_rYCenter),
			data
		);
	}

	void PrepareForAccessAction() const
	{
		this->m_ptSize = this->PSource()->Size();

		m_rSin=sin(-m_rAngle / 360 * 2 * PI);
		m_rCos=cos(-m_rAngle / 360 * 2 * PI);

		m_rXCenter=Real(this->Size().X())/2;
		m_rYCenter=Real(this->Size().Y())/2;
	}


};

//============================================================================
// 3d
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceTransformRotateOf<DATA,3,PRECISION,SOURCE>
	: public SourceTransformOf<DATA, 3, PRECISION, SOURCE>
{
	Real m_rAngleX;
	Real m_rAngleY;
	Real m_rAngleZ;
	mutable Real m_rCosX;
	mutable Real m_rSinX;
	mutable Real m_rCosY;
	mutable Real m_rSinY;
	mutable Real m_rCosZ;
	mutable Real m_rSinZ;
	mutable Real m_rXCenter;
	mutable Real m_rYCenter;
	mutable Real m_rZCenter;

public:
	SOURCE_ACTUALS_3D;

	SourceTransformRotateOf() 
		: SourceTransformOf<DATA, DIMENSIONALITY, PRECISION, SOURCE> ()
		, m_rAngleX(0)
		, m_rAngleY(0)
		, m_rAngleZ(0)
	{}

	virtual String Describe() const
	{
		return (
			_LINE + SerializationId() + LINE_ + 
			(_INDENT + "degrees: " + m_rAngleX + ", " + m_rAngleY + ", " + m_rAngleZ +  LINE_) + 
			this->DescribeCommon()
		);
	}

	virtual void RegisterParameters(RegistryOfParameters& reg)
	{
		reg.Register(&m_rAngleX);
		reg.Register(&m_rAngleY);
		reg.Register(&m_rAngleZ);
	}

	Real GetAngleX() const
	{
		return m_rAngleX;
	}
	Real GetAngleY() const
	{
		return m_rAngleY;
	}
	Real GetAngleZ() const
	{
		return m_rAngleZ;
	}

	void SetAngle(Real rX, Real rY, Real rZ)
	{
		m_rAngleX=rX;
		m_rAngleY=rY;
		m_rAngleZ=rZ;
	}

	static String SerializationId()
	{
		return "SourceTransformRotateOf(3D)";
	}

	void SerializeSelf(Stream &st) const
	{ 
		::Serialize(st,m_rAngleX);
		::Serialize(st,m_rAngleY);
		::Serialize(st,m_rAngleZ);
	}

	void DeserializeSelf(Stream& st)
	{
		Deserialize(st,m_rAngleX);
		Deserialize(st,m_rAngleY);
		Deserialize(st,m_rAngleZ);
	}

	/* rotation matricies in each separate dimension

	   s* == sin of *
     c* == cos of *

        in X
					 cx -sx  0
					 sx  cx  0
					 0   0   1

				in Y
					 cy  0   sy
					 0   1   0
					-sy  0   cy

				in Z
					1    0   0
					0    cz  -sz
					0    sz   cz

					from 

					http://en.wikipedia.org/wiki/Rotation_matrix
	*/

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		Real dX0 =   rX - m_rXCenter;
		Real dY0 =   rY - m_rYCenter;
		Real dZ0 =   rZ - m_rZCenter;

		Real dX1 =   m_rCosX * dX0 - m_rSinX * dY0                 ;
		Real dY1 =   m_rSinX * dX0 + m_rCosX * dY0                 ;
		Real&dZ1 =                                             dZ0 ;

		Real dX2 =   m_rCosY * dX1                 + m_rSinY * dZ1 ;
		Real&dY2 =                             dY1                 ;
		Real dZ2 = - m_rSinY * dX1                 + m_rCosY * dZ1 ;

		Real&dX3 =             dX2                                 ;
		Real dY3 =                   m_rCosZ * dY2 - m_rSinZ * dZ2 ;
		Real dZ3 =                   m_rSinZ * dY2 + m_rCosZ * dZ2 ;

		Real rXOut = dX3+m_rXCenter;
		Real rYOut = dY3+m_rYCenter;
		Real rZOut = dZ3+m_rZCenter;

		::Get(
			*(this->m_psource),
			dataOut,
			rXOut,
			rYOut,
			rZOut
		);

	}

	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		Real dX0 =   rX - m_rXCenter;
		Real dY0 =   rY - m_rYCenter;
		Real dZ0 =   rZ - m_rZCenter;

		Real dX1 =   m_rCosX * dX0 - m_rSinX * dY0                 ;
		Real dY1 =   m_rSinX * dX0 + m_rCosX * dY0                 ;
		Real&dZ1 =                                             dZ0 ;

		Real dX2 =   m_rCosY * dX1                 + m_rSinY * dZ1 ;
		Real&dY2 =                             dY1                 ;
		Real dZ2 = - m_rSinY * dX1                 + m_rCosY * dZ1 ;

		Real&dX3 =             dX2                                 ;
		Real dY3 =                   m_rCosZ * dY2 - m_rSinZ * dZ2 ;
		Real dZ3 =                   m_rSinZ * dY2 + m_rCosZ * dZ2 ;

		Real rXOut = dX3+m_rXCenter;
		Real rYOut = dY3+m_rYCenter;
		Real rZOut = dZ3+m_rZCenter;

		::Set(
			*(this->m_psource),
			rXOut,
			rYOut,
			rZOut,
			data
		);
	}

	void PrepareForAccessAction() const
	{
		this->m_ptSize=this->PSource()->Size();

		m_rSinX=sin(-m_rAngleX / 360 * 2 * PI);
		m_rCosX=cos(-m_rAngleX / 360 * 2 * PI);

		m_rSinY=sin(-m_rAngleY / 360 * 2 * PI);
		m_rCosY=cos(-m_rAngleY / 360 * 2 * PI);

		m_rSinZ=sin(-m_rAngleZ / 360 * 2 * PI);
		m_rCosZ=cos(-m_rAngleZ / 360 * 2 * PI);

		m_rXCenter=Real(this->Size().X())/2;
		m_rYCenter=Real(this->Size().Y())/2;
		m_rZCenter=Real(this->Size().Z())/2;
	}


};

//============================================================================
// rotate assistant
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	Rotate, 
	SourceTransformRotateOf, 
	TypeOfData(SOURCE),
	TypeOfDimensionality(SOURCE),
	Real,
	{ psrcOut->SetAngle(rAngle); }, 
	const Real& rAngle,
);

MAKE_TYPECHANGE_ASSISTANT(
	Rotate, 
	SourceTransformRotateOf, 
	TypeOfData(SOURCE),
	TypeOfDimensionality(SOURCE),
	Real,
	{ psrcOut->SetAngle(rAngleX,rAngleY,rAngleZ); }, 
	const Real& rAngleX,
	const Real& rAngleY,
	const Real& rAngleZ,
);


#endif // __SOURCETRANSFORMROTATE_H__

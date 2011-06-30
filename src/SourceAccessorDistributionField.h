// $Id$

#ifndef __SOURCEACCESSORDISTRIBUTIONFIELD_H__
#define __SOURCEACCESSORDISTRIBUTIONFIELD_H__

#include "SourceGenerics.h"

#include "Promotion.h"
#include <math.h>

#include "libGaussian.h"


//============================================================================
// DistributionFieldInfo
//============================================================================
// controls how a pixel of some random type is converted into a 
// distribution field
template <class T>
class DistributionFieldInfo
{
public:
	// TODO: Must be overridden for each DistributionField type
	//	typedef xxx type;
	//	static const int range=xxx;
	//	static const int maxvolume=xxx;

	// static int PositionIndex(const Pixel8BitGrey &pxl);
	// static void ComputeDistributionFieldParzen;
};



//============================================================================
// DistributionFieldInfo for Pixel8BitGrey
//============================================================================
NEW_POINT_TYPE(256); // create a 256 dimensional point type

template <>
class DistributionFieldInfo<Pixel8BitGrey>
{
public:
	typedef Point256DReal type;
	static const int range=256;
	static const int maxvolume=256;

	static int PositionIndex(const Pixel8BitGrey &pxl){
		return (int)pxl;
	}

	static void ComputeDistributionFieldParzen(
		Point256DReal* vdata, const Real& rSigmaParzen
	){
		static Point256DReal vdataCache[256];
		static Real rSigmaParzenLast=-1;
		
		if (rSigmaParzenLast != rSigmaParzen){
			rSigmaParzenLast=rSigmaParzen;
			//--------------------------------------------------------
			D("precomputing Parzen 'explosion' for each possible value with sigma %g", rSigmaParzen);
			for (int n=0; n<256; n++){
				for (int n2=0; n2<256; n2++){
					vdataCache[n].Dim(n2)=Gaussian(Real(n2),Real(n),rSigmaParzen);
				}
			}
		}

		memcpy(vdata,vdataCache,sizeof(Point256DReal)*256);
	}
};


//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorDistributionFieldBaseOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
protected:
	Real m_rSigmaParzen;

	typedef DistributionFieldInfo<typename SOURCE::type_data> INFO;

	mutable DATA m_vdataParzen[INFO::maxvolume];

public:
	SourceAccessorDistributionFieldBaseOf()
		: SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>()
		, m_rSigmaParzen(0)
	{
	}

	void SetSigmaParzen(Real rSigmaParzen)
	{
		m_rSigmaParzen=rSigmaParzen;
	}

	virtual String Describe() const
	{
		return (
			_LINE + SerializationId() + LINE_ + 
			(_INDENT + "with parzen sigma of: " + m_rSigmaParzen + LINE_ ) + 
			this->DescribeCommon()
		);
	}

	void PrepareForAccessAction() const
	{
		this->m_ptSize=this->m_psource->Size();

		INFO::ComputeDistributionFieldParzen(m_vdataParzen,m_rSigmaParzen);
	}

	static String SerializationId()
	{
		return String("SourceAccessorDistributionFieldOf(") + DIMENSIONALITY + "D)";
	}

	void SerializeSelf(Stream& st) const
	{ 
		Serialize(st,m_rSigmaParzen);
	}

	void DeserializeSelf(Stream &st)
	{
		Deserialize(st,m_rSigmaParzen);
	}

};

//============================================================================
// base class
//============================================================================
template<class DATA, int DIMENSIONALITY, class PRECISION, class SOURCE>
class SourceAccessorDistributionFieldOf 
	: public SourceAccessorOf<DATA, DIMENSIONALITY, PRECISION, SOURCE>
{
};


//============================================================================
// 2D
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorDistributionFieldOf<DATA, 2, PRECISION, SOURCE>
	: public SourceAccessorDistributionFieldBaseOf<DATA, 2, PRECISION, SOURCE>
{
	typedef DistributionFieldInfo<typename SOURCE::type_data> INFO;
public:
	SOURCE_ACTUALS_2D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY) const
	{
		typename SOURCE::type_data dataSource;

		::Get(*this->PSource(), dataSource, rX, rY);

		dataOut=this->m_vdataParzen[INFO::PositionIndex(dataSource)];
	}

	void Set(const PRECISION &rX, const PRECISION &rY, const DATA& data)
	{
		CONGEAL_ERROR("not implemented");
	}

};


//============================================================================
// 3d
//============================================================================
template<class DATA, class PRECISION, class SOURCE>
class SourceAccessorDistributionFieldOf<DATA, 3, PRECISION, SOURCE>
	: public SourceAccessorDistributionFieldBaseOf<DATA, 3, PRECISION, SOURCE> 
{
	typedef DistributionFieldInfo<typename SOURCE::type_data> INFO;

public:
	SOURCE_ACTUALS_3D;

	void Get(DATA& dataOut, const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ) const
	{
		typename SOURCE::type_data dataSource;

		::Get(*this->PSource(), dataSource, rX, rY, rZ);

		dataOut=this->m_vdataParzen[INFO::PositionIndex(dataSource)];
	}


	void Set(const PRECISION &rX, const PRECISION &rY, const PRECISION &rZ, const DATA& data)
	{
		CONGEAL_ERROR("not implemented");
	}

};





//============================================================================
// DistributionField
//============================================================================
MAKE_TYPECHANGE_ASSISTANT(
	DistributionField,
	SourceAccessorDistributionFieldOf,
	typename DistributionFieldInfo<TypeOfData(SOURCE)>::type,
	TypeOfDimensionality(SOURCE),
	TypeOfPrecision(SOURCE),
	{ psrcOut->SetSigmaParzen(rSigmaParzen); },
	const Real& rSigmaParzen,
);



#endif // __SOURCEACCESSORDISTRIBUTIONFIELD_H__

#ifndef __LIBNIFTIIO_H__
#define __LIBNIFTIIO_H__

#include "SourceRegistry.h"

#define USE_CUBIC_IO 0
#if USE_CUBIC_IO
//#  define NIFTI_INTERPOLANT SourceAccessorCubicInterpolateOf
//#  define NIFTI_INTERPOLANT_FUNCTION CubicInterpolation
#  define NIFTI_INTERPOLANT SourceAccessorLinearInterpolateOf
#  define NIFTI_INTERPOLANT_FUNCTION LinearInterpolation
#else
#  define NIFTI_INTERPOLANT SourceAccessorNearestNeighborOf
#  define NIFTI_INTERPOLANT_FUNCTION NearestNeighbor
#endif

typedef Pixel12BitGrey NiftiPixel;
typedef Grey12Volume NiftiDataVolume;

//typedef Pixel8BitGrey NiftiPixel;
//typedef GreyVolume NiftiDataVolume;

typedef SourceTransformScaleOf<
	NiftiPixel,3,Real,
	NIFTI_INTERPOLANT<NiftiPixel,3,Real,NiftiDataVolume> > 
NiftiVolume;


//============================================================================
//============================================================================
NiftiVolume* ReadNifti(const char* sfl);

// nifti header based on code sample from
//
//   http://nifti.nimh.nih.gov/pub/dist/src/niftilib/nifti1.h

typedef struct  {
	int   m_cySizeHeader; // MUST be 348
	char  m_vchUnused[36]; 

	short m_vcSize[8]; // Data array dimensions
	float m_vrIntent[3]; // intent parameters

	short m_nCodeIntent; // NIFTI_INTENT_* code
	short m_nCodeData; // NIFTI_TYPE_* code
	short m_cBitsPerPixel;  
	short m_nSliceStart; // First slice index.
	float m_vrScale[8]; // Grid spacings.
	float m_nyDataStart; // Offset into .nii file
	float m_rDataScale; // Data scaling: slope.
	float m_drDataOffset; // Data scaling: offset.
	short m_nyDataEnd; // Last slice index.
	char m_nCodeSlice; // NIFTI_SLICE_* code

	char  m_nCodeUnits; // NIFTI_UNITS_* code
	float m_rDataMax; // Max display intensity
	float m_rDataMin; // Min display intensity
	float m_dtmSlice; // Time for slice.
	float m_dtmOffset; // Time axis shift.

	char m_vchUnused2[8];

	char  m_vchDescription[80]; 
	char  m_sflAux[24]; // auxiliary filename.

	short m_nCodeQForm; // NIFTI_XFORM_* code.
	short m_nCodeSForm; // NIFTI_XFORM_* code.

	float m_vrQuaternion[6]; // Quaternion parameters

	float m_vrAffine[12]; // affine transform

	char m_vsIntentName[16];

	char m_vchMagic[4]; // MUST be "ni1\0" or "n+1\0".
} NiftiHeader;


#define NIFTI_TYPE_UINT8           2
#define NIFTI_TYPE_INT16           4
#define NIFTI_TYPE_INT32           8
#define NIFTI_TYPE_FLOAT32        16
#define NIFTI_TYPE_COMPLEX64      32
#define NIFTI_TYPE_FLOAT64        64
#define NIFTI_TYPE_RGB24         128
#define NIFTI_TYPE_INT8          256
#define NIFTI_TYPE_UINT16        512
#define NIFTI_TYPE_UINT32        768
#define NIFTI_TYPE_INT64        1024
#define NIFTI_TYPE_UINT64       1280
#define NIFTI_TYPE_FLOAT128     1536
#define NIFTI_TYPE_COMPLEX128   1792
#define NIFTI_TYPE_COMPLEX256   2048
#define NIFTI_TYPE_RGBA32       2304


#define NIFTI_INTENT_NONE        0
#define NIFTI_INTENT_CORREL      2
#define NIFTI_INTENT_TTEST       3
#define NIFTI_INTENT_FTEST       4
#define NIFTI_INTENT_ZSCORE      5
#define NIFTI_INTENT_CHISQ       6
#define NIFTI_INTENT_BETA        7
#define NIFTI_INTENT_BINOM       8
#define NIFTI_INTENT_GAMMA       9
#define NIFTI_INTENT_POISSON    10
#define NIFTI_INTENT_NORMAL     11
#define NIFTI_INTENT_FTEST_NONC 12
#define NIFTI_INTENT_CHISQ_NONC 13
#define NIFTI_INTENT_LOGISTIC   14
#define NIFTI_INTENT_LAPLACE    15
#define NIFTI_INTENT_UNIFORM    16
#define NIFTI_INTENT_TTEST_NONC 17
#define NIFTI_INTENT_WEIBULL    18
#define NIFTI_INTENT_CHI        19
#define NIFTI_INTENT_INVGAUSS   20
#define NIFTI_INTENT_EXTVAL     21
#define NIFTI_INTENT_PVAL       22
#define NIFTI_INTENT_LOGPVAL    23
#define NIFTI_INTENT_LOG10PVAL  24
#define NIFTI_FIRST_STATCODE     2
#define NIFTI_LAST_STATCODE     24
#define NIFTI_INTENT_ESTIMATE  1001
#define NIFTI_INTENT_LABEL     1002
#define NIFTI_INTENT_NEURONAME 1003
#define NIFTI_INTENT_GENMATRIX 1004
#define NIFTI_INTENT_SYMMATRIX 1005
#define NIFTI_INTENT_DISPVECT  1006
#define NIFTI_INTENT_VECTOR    1007
#define NIFTI_INTENT_POINTSET  1008
#define NIFTI_INTENT_TRIANGLE  1009
#define NIFTI_INTENT_QUATERNION 1010
#define NIFTI_INTENT_DIMLESS    1011
#define NIFTI_INTENT_TIME_SERIES  2001
#define NIFTI_INTENT_NODE_INDEX   2002
#define NIFTI_INTENT_RGB_VECTOR   2003
#define NIFTI_INTENT_RGBA_VECTOR  2004
#define NIFTI_INTENT_SHAPE        2005


#define NIFTI_SLICE_UNKNOWN   0
#define NIFTI_SLICE_SEQ_INC   1
#define NIFTI_SLICE_SEQ_DEC   2
#define NIFTI_SLICE_ALT_INC   3
#define NIFTI_SLICE_ALT_DEC   4
#define NIFTI_SLICE_ALT_INC2  5
#define NIFTI_SLICE_ALT_DEC2  6
#define NIFTI_UNITS_UNKNOWN 0

#define NIFTI_UNITS_METER   1
#define NIFTI_UNITS_MM      2
#define NIFTI_UNITS_MICRON  3
#define NIFTI_UNITS_SEC     8
#define NIFTI_UNITS_MSEC   16
#define NIFTI_UNITS_USEC   24
#define NIFTI_UNITS_HZ     32
#define NIFTI_UNITS_PPM    40
#define NIFTI_UNITS_RADS   48

#define NIFTI_XFORM_UNKNOWN      0
#define NIFTI_XFORM_SCANNER_ANAT 1
#define NIFTI_XFORM_ALIGNED_ANAT 2
#define NIFTI_XFORM_TALAIRACH    3
#define NIFTI_XFORM_MNI_152      4


#endif //__LIBNIFTIIO_H__

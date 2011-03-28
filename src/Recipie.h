#ifndef __RECIPIE_H__
#define __RECIPIE_H__

#undef DEBUG
#define DEBUG 0
#include "libDebug.h"

//============================================================================
//=================================================p===========================
template<class SOURCE> 
class RecipieOf :
	public Counted
{
	typedef typename SOURCE::type_point POINT;
	typedef typename SOURCE::type_data DATA;
	typedef typename SOURCE::type_precision PRECISION;
public:
	typedef typename SOURCE::type_point type_point;
	typedef typename SOURCE::type_data type_data;
	static const int type_dimensionality = SOURCE::type_dimensionality;
	typedef typename SOURCE::type_precision type_precision;
	typedef SOURCE type_source;

private:
	int m_csrc;
	SOURCE* m_vsrcFinal;

protected:
	// must call
	RecipieOf()
		: Counted()
		, m_csrc(0)
		, m_vsrcFinal(NULL)
	{
	}

	void SetSource(int c, SOURCE* vsrc)
	{
		m_csrc=c;
		m_vsrcFinal=vsrc;
	}

public:
	// must reimplement these
	virtual void RegisterParameters( 
		RegistryOfParameters& regParam
	) =0;

	virtual void RegisterInitialSteps( 
		RegistryOfInitialSteps& regSteps
	) =0;

	virtual void Normalize() =0;

public:
	virtual ~RecipieOf() __attribute__((always_inline))
	{
	}

	// other methods
	virtual const POINT& Size() const 
	{
		return m_vsrcFinal[0].Size();
	}

	virtual PRECISION CSize() const
	{
		return m_vsrcFinal[0].CSize();
	}

	virtual void GetPoint(DATA* vdataOut, const POINT& pt) const
	{
		for (int n=0; n<m_csrc; n++){
			::GetPoint(m_vsrcFinal[n],vdataOut[n],pt);
		}
	}

	virtual void PrepareForAccess() const {
		for (int n=0; n<m_csrc; n++){
			m_vsrcFinal[n].PrepareForAccess();
		}
	}

	virtual void PrepareForAccess(int n) const {
		m_vsrcFinal[n].PrepareForAccess();
	}

	virtual String Describe() const
	{
		String s;
		for (int n=0; n<m_csrc; n++){
			s=s+(
				_LINE + "Source " + n + LINE_ + 
				(_INDENT + m_vsrcFinal[n].Describe()) 
			);
		}

		return (
			_LINE + "Recipie with " + m_csrc + LINE_
			+ s
		);
	}

	virtual int CSources() const
	{
		return m_csrc;
	}

	virtual SOURCE* PSource(int n){
		return &m_vsrcFinal[n];
	}

	virtual SOURCE* VSources(){
		return m_vsrcFinal;
	}

	template <class SOURCE_OUT>
	void AllocatedCopies(SOURCE_OUT* viOut)
	{
		int c=this->CSources();
		for (int n=0; n<c; n++){
			::AllocatedCopy(viOut[n], m_vsrcFinal[n]);
		}
	}

	static String SerializationId()
	{
		return "Recipie";
	}

	void Serialize(Stream& st) const
	{ 
		for (int n=0; n<m_csrc; n++){
			::Serialize(st,m_vsrcFinal[n]);
		}
	}

	void Deserialize(Stream &st)
	{
		for (int n=0; n<m_csrc; n++){
			::Deserialize(st,m_vsrcFinal[n]);
		}
	}

};






//=============================================================================
// MAKE RECIPIE PARTS
//=============================================================================
// warning: these are "unclean" macros in that they make use of variables
// within the MAKE_RECIPIE macro below (e.g. c, n)
#define _RECIPIE_TYPES(_name,_class,_data,_dims,_precision,_source)						\
	typedef _class<_data,_dims,_precision,LAYER_##_source> LAYER_##_name; 

#define _RECIPIE_STORAGE_TYPE(_name,_class,_data,_dims,_precision,_source)		\
	PRIVATE::LAYER_##_name

//	_class<_data,_dims,_precision,PRIVATE::LAYER_##_source> 

#define _RECIPIE_STORAGE(_name,_class,_data,_dims,_precision,_source)					\
	PRIVATE::LAYER_##_name* m_vsrc##_name;																			\
	bool m_bOptimize##_name;

#define _RECIPIE_ACCESSOR(_name,_class,_data,_dims,_precision,_source)				\
	PRIVATE::LAYER_##_name& Layer##_name(int n){	return m_vsrc##_name[n];	}		\
	void OptimizeLayer##_name(bool b){	m_bOptimize##_name=b;	}									

#define _RECIPIE_OPTIMIZELAYERDEFAULT(_name,_class,_data,_dims,_precision,_source) \
	m_bOptimize##_name=true;

#define _RECIPIE_ALLOCATELAYER(_name,_class,_data,_dims,_precision,_source)		\
	m_vsrc##_name = new PRIVATE::LAYER_##_name[c];

#define _RECIPIE_DELETELAYER(_name,_class,_data,_dims,_precision,_source)			\
	for (int n=CSources()-1; n>=0; n--){																				\
		m_vsrc##_name[n].WipeSource();																						\
	}																																						\
	delete [] m_vsrc##_name;																										

#define _RECIPIE_SETSOURCE(_name,_class,_data,_dims,_precision,_source)				\
	m_vsrc##_name[n].SetSource(&m_vsrc##_source[n]);

#define _RECIPIE_SETLASTSOURCE(_name,_class,_data,_dims,_precision,_source)		\
	this->SetSource(c,m_vsrc##_name);

#define _RECIPIE_REGISTER_PARAMS(_name,_class,_data,_dims,_precision,_source)	\
	if (m_bOptimize##_name){																										\
		Layer##_name(n).RegisterParameters(regParam);															\
	}																																						

#define _RECIPIE_REGISTER_INITIALSTEPS(_name,_class,_data,_dims,_precision,_source)	\
	if (m_bOptimize##_name){																										\
		::RegisterInitialSteps(Layer##_name(n), regSteps);												\
	}

#define _RECIPIE_NORMALIZE(_name,_class,_data,_dims,_precision,_source)				\
	if (m_bOptimize##_name){																										\
		NormalizeGroup(this->CSources(),m_vsrc##_name);														\
	}

#define _RECIPIE_COUNT(_name,_class,_data,_dims,_precision,_source)						\
	+1

#define _RECIPIE_DESCRIBE(_name,_class,_data,_dims,_precision,_source)				\
	Layer##_name(n).Describe();

#define CONCAT2(x,y) x ## y
#define CONCAT(x,y) CONCAT2(x,y)

#define PRIVATE CONCAT(_privateNS,__LINE__)

//=============================================================================
// MAKE RECIPIE
//=============================================================================
#define MAKE_RECIPIE(																													\
	_recipie,																																		\
	_source,																																		\
	_do_mid_layers,																															\
	_do_last_layer																															\
)																																							\
	namespace PRIVATE {																													\
		typedef _source LAYER_INPUT;																							\
		_do_mid_layers(_RECIPIE_TYPES);																						\
		_do_last_layer(_RECIPIE_TYPES);																						\
	}																																						\
																																							\
	class _recipie																															\
		: public RecipieOf<_do_last_layer(_RECIPIE_STORAGE_TYPE)>									\
	{																																						\
		protected:																																\
																																							\
		PRIVATE::LAYER_INPUT* m_vsrcINPUT;																				\
		_do_mid_layers(_RECIPIE_STORAGE);																					\
		_do_last_layer(_RECIPIE_STORAGE);																					\
																																							\
		public:																																		\
		_recipie(int c, _source* vsrcInput)																				\
		: RecipieOf<_do_last_layer(_RECIPIE_STORAGE_TYPE)>()											\
		{																																					\
			m_vsrcINPUT=vsrcInput;																									\
			_do_mid_layers(_RECIPIE_ALLOCATELAYER);																	\
			_do_last_layer(_RECIPIE_ALLOCATELAYER);																	\
																																							\
			_do_mid_layers(_RECIPIE_OPTIMIZELAYERDEFAULT);													\
			_do_last_layer(_RECIPIE_OPTIMIZELAYERDEFAULT);													\
																																							\
			for (int n=0; n<c; n++){																								\
				_do_mid_layers(_RECIPIE_SETSOURCE);																		\
				_do_last_layer(_RECIPIE_SETSOURCE);																		\
			}																																				\
																																							\
			_do_last_layer(_RECIPIE_SETLASTSOURCE);																	\
		}																																					\
																																							\
		~_recipie()																																\
		{																																					\
			if (AboutToDie()){																											\
				_do_mid_layers(_RECIPIE_DELETELAYER);																	\
				_do_last_layer(_RECIPIE_DELETELAYER);																	\
			}																																				\
		}																																					\
																																							\
		int CLayers() const																												\
		{																																					\
			return (																																\
				_do_mid_layers(_RECIPIE_COUNT)																				\
				_do_last_layer(_RECIPIE_COUNT)																				\
			);																																			\
		}																																					\
																																							\
		virtual String Describe()	const																						\
		{																																					\
			return (																																\
				_LINE +  #_recipie + LINE_ +																					\
				(																																			\
					_INDENT +																														\
					RecipieOf<_do_last_layer(_RECIPIE_STORAGE_TYPE)>::Describe()				\
				)																																			\
			);																																			\
		}																																					\
																																							\
		void RegisterParameters(																									\
			RegistryOfParameters& regParam																					\
		)																																					\
		{																																					\
			int c=this->CSources();																									\
			for (int n=0; n<c; n++){																								\
				RegisterParameters(n,regParam);																				\
			}																																				\
		}																																					\
																																							\
		void RegisterParameters(																									\
			int n,																																	\
			RegistryOfParameters& regParam																					\
		)																																					\
		{																																					\
			_do_mid_layers(_RECIPIE_REGISTER_PARAMS);																\
			_do_last_layer(_RECIPIE_REGISTER_PARAMS);																\
		}																																					\
																																							\
		void RegisterInitialSteps(																								\
			RegistryOfInitialSteps& regSteps																				\
		)																																					\
		{																																					\
			int c=this->CSources();																									\
																																							\
			for (int n=0; n<c; n++){																								\
				RegisterInitialSteps(n,regSteps);																			\
			}																																				\
		}																																					\
																																							\
		void RegisterInitialSteps(																								\
			int n,																																	\
			RegistryOfInitialSteps& regSteps																				\
		)																																					\
		{																																					\
			_do_mid_layers(_RECIPIE_REGISTER_INITIALSTEPS);													\
			_do_last_layer(_RECIPIE_REGISTER_INITIALSTEPS);													\
		}																																					\
																																							\
		PRIVATE::LAYER_INPUT* LayerINPUT(){	return m_vsrcINPUT;	}									\
		_do_mid_layers(_RECIPIE_ACCESSOR);																				\
		_do_last_layer(_RECIPIE_ACCESSOR);																				\
																																							\
		void Normalize()																													\
		{																																					\
			_do_mid_layers(_RECIPIE_NORMALIZE);																			\
			_do_last_layer(_RECIPIE_NORMALIZE);																			\
		}																																					\
	};																																					


/* Example usage



// A congeal recipie for a translate, rotate, warp of a volume
#define my_mid_layers(_layer)																											\
	_layer(Interp,       SourceAccessorLinearInterpolateOf,  Pixel8BitGrey, 3, Real, INPUT)	\
		_layer(Translate0, SourceTransformTranslateOf,         Pixel8BitGrey, 3, Real, Interp) \
		_layer(Rotate,     SourceTransformRotateOf,            Pixel8BitGrey, 3, Real, Translate0) \
		_layer(Translate1, SourceTransformTranslateOf,         Pixel8BitGrey, 3, Real, Rotate) 


#define my_last_layer(_layer)																									\
	_layer(Warp, SourceTransformWarpOf, Pixel8BitGrey, 3, Real,Translate1)

MAKE_RECIPIE(
	Recipie_G3R_All_G3I,
	GreyVolume,
	my_mid_layers,
	my_last_layer
);

*/


#endif //#ifndef __RECIPIE_H__

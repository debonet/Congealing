#undef D0
#undef DX0
#undef ID0
#undef IDX0
#undef UD0
#undef UDX0
#undef UID0
#undef UIDX0

//==========================================================================
//==========================================================================
// indented debug no newline
#define IDX0(_c,_s...)																					\
	do {																													\
		static volatile int _d##__LINE__=0;													\
		if (_d##__LINE__ < 10) {																		\
			printf("%s:%d: ",__FILE__ ,__LINE__);											\
			for (int _n=0; _n<_c; _n++){															\
				printf("\t");																						\
			};																												\
			printf(_s);																								\
		}																														\
		else if (IsIntegral(log2((_d##__LINE__)))) {								\
			printf("%s:%d: [%d] ",__FILE__ ,__LINE__, _d##__LINE__);	\
			for (int _n=0; _n<_c; _n++){															\
				printf("\t");																						\
			};																												\
			printf(_s);																								\
		}																														\
		_d##__LINE__++;																							\
	} while(0)

//==========================================================================
//==========================================================================
// indented debug
#define ID0(_c,_s...)																						\
	do {																													\
		static volatile int _d##__LINE__=0;													\
		int _dt##__LINE__ = _d##__LINE__;														\
		if (_dt##__LINE__ < 10) {																		\
			printf("%s:%d: ",__FILE__ ,__LINE__);											\
			for (int _n=0; _n<_c; _n++){															\
				printf("\t");																						\
			};																												\
			printf(_s);																								\
			printf("\n");																							\
		}																														\
		else if (IsIntegral(log2((_dt##__LINE__)))) {								\
			printf("%s:%d: [%d] ",__FILE__ ,__LINE__, _dt##__LINE__);	\
			for (int _n=0; _n<_c; _n++){															\
				printf("\t");																						\
			};																												\
			printf(_s);																								\
			printf("\n");																							\
		}																														\
		_d##__LINE__++;																							\
	} while(0)


//==========================================================================
//==========================================================================
#define DX0(_s...) do {IDX0(0,_s);} while(0)

//==========================================================================
//==========================================================================
// debug
#define D0(_s...)	do {ID0(0,_s);} while(0)

//==========================================================================
//==========================================================================
// unconditional indented debug with no newline
#define UIDX0(_c,_s...)													\
	do {																					\
		printf("%s:%d: ",__FILE__ ,__LINE__);				\
		for (int _n=0; _n<_c; _n++){								\
			printf("\t");															\
		};																					\
		printf(_s);																	\
	} while(0)
//==========================================================================
//==========================================================================
// unconditional indented debug
#define UID0(_c,_s...) do {UIDX0(_c,_s); printf("\n");} while(0)

//==========================================================================
//==========================================================================
// unconditional debug with no newline
#define UDX0(_s...) do {UIDX0(0,_s);} while(0)


//==========================================================================
//==========================================================================
// unconditional debug
#define UD0(_s...) do {UDX(_s); printf("\n"); } while(0)



//==========================================================================
//==========================================================================
#undef D
#undef DX
#undef ID
#undef IDX
#undef UD
#undef UDX
#undef UID
#undef UIDX
#undef DEBUGONLY
#if DEBUG >= 0
# define DEBUGONLY(x) x
# define D(x...) D0(x)
# define DX(x...) DX0(x)
# define ID(x...) ID0(x)
# define IDX(x...) IDX0(x)
# define UD(x...) UD0(x)
# define UDX(x...) UDX0(x)
# define UID(x...) UID0(x)
# define UIDX(x...) UIDX0(x)
#else
# define DEBUGONLY(x)
# define D(x...) 
# define DX(x...)
# define ID(x...)
# define IDX(x...)
# define UD(x...)
# define UDX(x...)
# define UID(x...)
# define UIDX(x...)
#endif


//==========================================================================
//==========================================================================
#undef D1
#undef DX1
#undef ID1
#undef IDX1
#undef UD1
#undef UDX1
#undef UID1
#undef UIDX1
#if DEBUG >= 1
# define D1(x...) D0(x)
# define DX1(x...) DX0(x)
# define ID1(x...) ID0(x)
# define IDX1(x...) IDX0(x)
# define UD1(x...) UD0(x)
# define UDX1(x...) UDX0(x)
# define UID1(x...) UID0(x)
# define UIDX1(x...) UIDX0(x)
#else
# define D1(x...) 
# define DX1(x...)
# define ID1(x...)
# define IDX1(x...)
# define UD1(x...)
# define UDX1(x...)
# define UID1(x...)
# define UIDX1(x...)
#endif


//==========================================================================
//==========================================================================
#undef D2
#undef DX2
#undef ID2
#undef IDX2
#undef UD2
#undef UDX2
#undef UID2
#undef UIDX2
#if DEBUG >= 2
# define D2(x...) D0(x)
# define DX2(x...) DX0(x)
# define ID2(x...) ID0(x)
# define IDX2(x...) IDX0(x)
# define UD2(x...) UD0(x)
# define UDX2(x...) UDX0(x)
# define UID2(x...) UID0(x)
# define UIDX2(x...) UIDX0(x)
#else
# define D2(x...) 
# define DX2(x...)
# define ID2(x...)
# define IDX2(x...)
# define UD2(x...)
# define UDX2(x...)
# define UID2(x...)
# define UIDX2(x...)
#endif



//==========================================================================
//==========================================================================
#undef D3
#undef DX3
#undef ID3
#undef IDX3
#undef UD3
#undef UDX3
#undef UID3
#undef UIDX3
#if DEBUG >= 3
# define D3(x...) D0(x)
# define DX3(x...) DX0(x)
# define ID3(x...) ID0(x)
# define IDX3(x...) IDX0(x)
# define UD3(x...) UD0(x)
# define UDX3(x...) UDX0(x)
# define UID3(x...) UID0(x)
# define UIDX3(x...) UIDX0(x)
#else
# define D3(x...) 
# define DX3(x...)
# define ID3(x...)
# define IDX3(x...)
# define UD3(x...)
# define UDX3(x...)
# define UID3(x...)
# define UIDX3(x...)
#endif


//==========================================================================
//==========================================================================
#undef D4
#undef DX4
#undef ID4
#undef IDX4
#undef UD4
#undef UDX4
#undef UID4
#undef UIDX4
#if DEBUG >= 4
# define D4(x...) D0(x)
# define DX4(x...) DX0(x)
# define ID4(x...) ID0(x)
# define IDX4(x...) IDX0(x)
# define UD4(x...) UD0(x)
# define UDX4(x...) UDX0(x)
# define UID4(x...) UID0(x)
# define UIDX4(x...) UIDX0(x)
#else
# define D4(x...) 
# define DX4(x...)
# define ID4(x...)
# define IDX4(x...)
# define UD4(x...)
# define UDX4(x...)
# define UID4(x...)
# define UIDX4(x...)
#endif

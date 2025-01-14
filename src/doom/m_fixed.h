#ifndef __M_FIXED__
#define __M_FIXED__

#include <stdlib.h>
#include "doomtype.h"

// Unfortunately, the Scale function still gets badly handled on 32 bit x86 platforms so it's the last remaining piece of inline assembly 

#if defined(__GNUG__)
    #define __forceinline __attribute__((always_inline)) inline
#endif

// GCC inlines
#if defined(__GNUC__) && defined(__i386__) && !defined(__clang__) && !defined(__PIC__)
#ifndef alloca
// MinGW does not seem to come with alloca defined.
#define alloca __builtin_alloca
#endif

static inline int32_t Scale(int32_t a, int32_t b, int32_t c)
{
	int32_t result, dummy;

	asm volatile
		("imull %3\n\t"
			"idivl %4"
			: "=a,a,a,a,a,a" (result),
			"=&d,&d,&d,&d,d,d" (dummy)
			: "a,a,a,a,a,a" (a),
			"m,r,m,r,d,d" (b),
			"r,r,m,m,r,m" (c)
			: "cc"
			);

	return result;
}

// MSVC inlines
#elif defined(_MSC_VER) && defined(_M_IX86)
#pragma warning (disable: 4035)

__forceinline int32_t Scale(int32_t a, int32_t b, int32_t c)
{
	__asm mov eax, a
	__asm imul b
	__asm idiv c
}

#pragma warning (default: 4035)
#else

static __forceinline int32_t Scale(int32_t a, int32_t b, int32_t c)
{
	return (int32_t)(((int64_t)a*b) / c);
}

#endif

// Modern compilers are smart enough to do these multiplications intelligently.
__forceinline int32_t MulScale14(int32_t a, int32_t b) { return (int32_t)(((int64_t)a * b) >> 14); } // only used by R_DrawVoxel
__forceinline int32_t MulScale30(int32_t a, int32_t b) { return (int32_t)(((int64_t)a * b) >> 30); } // only used once in the node builder
__forceinline int32_t MulScale32(int32_t a, int32_t b) { return (int32_t)(((int64_t)a * b) >> 32); } // only used by R_DrawVoxel

__forceinline uint32_t UMulScale16(uint32_t a, uint32_t b) { return (uint32_t)(((uint64_t)a * b) >> 16); } // used for sky drawing

__forceinline int32_t DMulScale3(int32_t a, int32_t b, int32_t c, int32_t d) { return (int32_t)(((int64_t)a*b + (int64_t)c*d) >> 3); } // used for setting up slopes for Build maps
__forceinline int32_t DMulScale6(int32_t a, int32_t b, int32_t c, int32_t d) { return (int32_t)(((int64_t)a*b + (int64_t)c*d) >> 6); } // only used by R_DrawVoxel
__forceinline int32_t DMulScale10(int32_t a, int32_t b, int32_t c, int32_t d) { return (int32_t)(((int64_t)a*b + (int64_t)c*d) >> 10); } // only used by R_DrawVoxel
__forceinline int32_t DMulScale18(int32_t a, int32_t b, int32_t c, int32_t d) { return (int32_t)(((int64_t)a*b + (int64_t)c*d) >> 18); } // only used by R_DrawVoxel
__forceinline int32_t DMulScale32(int32_t a, int32_t b, int32_t c, int32_t d) { return (int32_t)(((int64_t)a*b + (int64_t)c*d) >> 32); } // used by R_PointOnSide.

// Sadly, for divisions this is not true but these are so infrequently used that the C versions are just fine, despite not being fully optimal.
__forceinline int32_t DivScale6(int32_t a, int32_t b) { return (int32_t)(((int64_t)a << 6) / b); } // only used by R_DrawVoxel
__forceinline int32_t DivScale21(int32_t a, int32_t b) { return (int32_t)(((int64_t)a << 21) / b); } // only used by R_DrawVoxel
__forceinline int32_t DivScale30(int32_t a, int32_t b) { return (int32_t)(((int64_t)a << 30) / b); } // only used once in the node builder

__forceinline void fillshort(void *buff, unsigned int count, uint16_t clear)
{
	int16_t *b2 = (int16_t *)buff;
	for (unsigned int i = 0; i != count; ++i)
	{
		b2[i] = clear;
	}
}

#include "xs_Float.h"

inline int32_t FixedDiv (int32_t a, int32_t b) 
{ 
	if ((uint32_t)abs(a) >> (31-16) >= (uint32_t)abs (b)) 
		return (a^b)<0 ? FIXED_MIN : FIXED_MAX; 

	return (int32_t)(((int64_t)a << 16) / b);
}

__forceinline int32_t FixedMul(int32_t a, int32_t b) 
{ 
	return (int32_t)(((int64_t)a * b) >> 16); 
}

inline fixed_t FloatToFixed(double f)
{
	return xs_Fix<16>::ToFix(f);
}

inline double FixedToFloat(fixed_t f)
{
	return f / 65536.;
}

inline unsigned FloatToAngle(double f)
{
	return xs_CRoundToInt((f)* (0x40000000 / 90.));
}

inline double AngleToFloat(unsigned f)
{
	return f * (90. / 0x40000000);
}

inline double AngleToFloat(int f)
{
	return f * (90. / 0x40000000);
}

#define FLOAT2FIXED(f)		FloatToFixed(f)
#define FIXED2FLOAT(f)		float(FixedToFloat(f))
#define FIXED2DBL(f)		FixedToFloat(f)

#define ANGLE2DBL(f)		AngleToFloat(f)

#endif

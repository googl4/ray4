#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

#define PI 3.14159265359

#define DEG_TO_RAD( n ) ( ( (float)n / 180.0f ) * PI )

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

typedef float f32;
typedef double f64;

/*
#define DEF_VEC2( type ) typedef type type##x2 __attribute__((ext_vector_type(2),aligned(8)));
#define DEF_VEC3( type ) typedef type type##x3 __attribute__((ext_vector_type(3),aligned(16)));
#define DEF_VEC4( type ) typedef type type##x4 __attribute__((ext_vector_type(4),aligned(16)));
*/
#define DEF_VEC2( type ) typedef type type##x2 __attribute__((ext_vector_type(2)));
#define DEF_VEC3( type ) typedef type type##x3 __attribute__((ext_vector_type(3)));
#define DEF_VEC4( type ) typedef type type##x4 __attribute__((ext_vector_type(4)));

#define DEF_VEC( type ) DEF_VEC2( type ) DEF_VEC3( type ) DEF_VEC4( type )

DEF_VEC( u8 )
DEF_VEC( s8 )
DEF_VEC( u16 )
DEF_VEC( s16 )
DEF_VEC( u32 )
DEF_VEC( s32 )
DEF_VEC( u64 )
DEF_VEC( s64 )
DEF_VEC( f32 )
DEF_VEC( f64 )

#endif

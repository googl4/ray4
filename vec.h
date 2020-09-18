#ifndef _VEC_H
#define _VEC_H

#include "types.h"

static inline float lerp( float a, float b, float t ) {
	return a * ( 1.0f - t ) + b * t;
}

static inline float step( float edge, float n ) {
	return n < edge ? 0 : 1;
}

static inline float smoothstep( float n ) {
	return n * n * ( 3.0f - 2.0f * n );
}

static inline float smootherstep( float n ) {
	return n * n * n * ( n * ( n * 6.0f - 15.0f ) + 10.0f );
}

static inline float min( float a, float b ) {
	return a < b ? a : b;
}

static inline float max( float a, float b ) {
	return a > b ? a : b;
}

static inline float clamp( float n, float a, float b ) {
	return max( a, min( b, n ) );
}

static inline float fract( float n ) {
	return n - floorf( n );
}

static inline float sign( float n ) {
	return n >= 0.0f;
}

typedef f32x2 vec2;
typedef f32x3 vec3;
typedef f32x4 vec4;

static inline float length( vec3 v ) {
	return sqrtf( v.x * v.x + v.y * v.y + v.z * v.z );
}

static inline vec3 normalize( vec3 v ) {
	return v / length( v );
}

static inline float dot( vec3 a, vec3 b ) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec3 cross( vec3 a, vec3 b ) {
	return a.yzx * b.zxy - b.yzx * a.zxy;
}

static inline float distance( vec3 a, vec3 b ) {
	return length( a - b );
}

static inline vec3 reflect( vec3 i, vec3 n ) {
	return i - 2.0f * dot( n, i ) * n;
}

static inline vec3 refract( vec3 i, vec3 n, float eta ) {
	float k = 1.0f - eta * eta * ( 1.0f - dot( n, i ) * dot( n, i ) );
	if( k < 0.0f ) {
		return (vec3)( 0.0f );
	} else {
		return eta * i - ( eta * dot( n, i ) + sqrtf( k ) ) * n;
	}
}

static inline float sum( vec3 v ) {
	return v.x + v.y + v.z;
}

static inline vec3 vpow( vec3 v, float p ) {
	return (vec3){ powf( v.x, p ), powf( v.y, p ), powf( v.z, p ) };
}

static inline vec3 vclamp( vec3 v, float a, float b ) {
	return (vec3){ clamp( v.x, a, b ), clamp( v.y, a, b ), clamp( v.z, a, b ) };
}

static inline vec3 vmin( vec3 a, vec3 b ) {
	return (vec3){ min( a.x, b.x ), min( a.y, b.y ), min( a.z, b.z ) };
}

static inline vec3 vmax( vec3 a, vec3 b ) {
	return (vec3){ max( a.x, b.x ), max( a.y, b.y ), max( a.z, b.z ) };
}

static inline vec3 vabs( vec3 a ) {
	return (vec3){ fabsf( a.x ), fabsf( a.y ), fabsf( a.z ) };
}

static inline vec3 vsign( vec3 a ) {
	return (vec3){ sign( a.x ), sign( a.y ), sign( a.z ) };
}

static inline vec3 vstep( vec3 a, vec3 b ) {
	return (vec3){ step( a.x, b.x ), step( a.y, b.y ), step( a.z, b.z ) };
}

typedef vec3 mat3[3];
typedef vec4 mat4[4];

static inline void mat4_identity( mat4* m ) {
	(*m)[0] = (vec4){ 1, 0, 0, 0 };
	(*m)[1] = (vec4){ 0, 1, 0, 0 };
	(*m)[2] = (vec4){ 0, 0, 1, 0 };
	(*m)[3] = (vec4){ 0, 0, 0, 1 };
}

static inline void mat4_perspective( mat4* m, float fovY, float aspect, float n, float f ) {
	float a = 1.0f / tanf( fovY / 2.0f );
	(*m)[0] = (vec4){ a / aspect, 0, 0,                                 0 };
	(*m)[1] = (vec4){ 0,          a, 0,                                 0 };
	(*m)[2] = (vec4){ 0,          0, -( ( f + n ) / ( f - n ) ),        0 };
	(*m)[3] = (vec4){ 0,          0, -( ( 2.0f * f * n ) / ( f - n ) ), 0 };
}

static inline vec4 mat4_mul_vec4( mat4 m, vec4 v ) {
	return m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
}

static inline vec3 mat3_mul_vec3( mat3 m, vec3 v ) {
	return m[0] * v.x + m[1] * v.y + m[2] * v.z;
}

#endif

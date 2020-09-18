#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <time.h>

#include "tigr.h"

#include "types.h"
#include "vec.h"

static inline vec3 intersectTri( vec3 origin, vec3 dir, vec3 v0, vec3 v1, vec3 v2 ) {
	vec3 v10 = v1 - v0;
	vec3 v20 = v2 - v0;
	vec3 rov0 = origin - v0;
	vec3 n = cross( v10, v20 );
	vec3 q = cross( rov0, dir );
	float d = 1.0f / dot( dir, n );
	float u = d * dot( -q, v20 );
	float v = d * dot( q, v10 );
	float t = d * dot( -n, rov0 );
	if( u < 0.0f || u > 1.0f || v < 0.0f || ( u + v ) > 1.0f ) {
		t = -1.0f;
	}
	return (vec3){ t, u, v };
}

static inline vec2 intersectBox( vec3 origin, vec3 dir, vec3 boxPos, vec3 boxSize, vec3* normal ) {
	vec3 roo = ( origin - boxPos );
	vec3 rdd = dir;
	vec3 m = 1.0f / rdd;
	vec3 n = m * roo;
	vec3 k = vabs( m ) * boxSize;
	vec3 t1 = -n - k;
	vec3 t2 = -n + k;
	float tN = max( max( t1.x, t1.y ), t1.z );
	float tF = min( min( t2.x, t2.y ), t2.z );
	if( tN > tF || tF < 0.0f ) {
		return (vec2){ -1.0f, -1.0f };
	}
	*normal = -vsign( rdd ) * vstep( t1.yzx, t1.xyz ) * vstep( t1.zxy, t1.xyz );
	return (vec2){ tN, tF };
}

u32 xorwow( u32* state ) {
	u32 s, t = state[3];
	t ^= t >> 2;
	t ^= t << 1;
	state[3] = state[2]; state[2] = state[1]; state[1] = s = state[0];
	t ^= s;
	t ^= s << 4;	
	state[0] = t;
	return t + ( state[4] += 362437 );
}

float frand( u32* state ) {
	u32 f = 0x3F800000 | ( xorwow( state ) >> 9 );
	return ( *(float*)&f - 1.5f ) * 2.0f;
	//return (float)xorwow( state ) / UINT32_MAX * 2.0f - 1.0f;
}

/*
vec3 verts[12] = {
	{ -0.282092,  0.270908,  0.141301 },
	{ -0.120911,  0.538408, -0.080902 },
	{ -0.191232,  0.002111, -0.116384 },
	{ -0.030052,  0.269610, -0.338587 },
	{  0.053579,  0.215221,  0.317748 },
	{  0.214759,  0.482720,  0.095544 },
	{  0.144439, -0.053577,  0.060063 },
	{  0.305619,  0.213922, -0.162141 },
	{ -0.988236, -0.013149,  0.989580 },
	{  1.011764, -0.013149,  0.989580 },
	{ -0.988236, -0.013149, -1.010420 },
	{  1.011764, -0.013149, -1.010420 }
};

int faces[45] = {
	1, 4, 3,
	4, 7, 3,
	8, 5, 7,
	6, 1, 5,
	7, 1, 3,
	4, 6, 8,
	1, 2, 4,
	4, 8, 7,
	8, 6, 5,
	6, 2, 1,
	7, 5, 1,
	4, 2, 6,
	10, 11, 9,
	10, 12, 11
};

int numFaces = 15;
*/

vec3* verts = NULL;
int* faces = NULL;

int numVerts = 0;
int numFaces = 0;

void loadOBJ( const char* fn ) {
	verts = malloc( sizeof( vec3 ) * 16384 * 24 );
	faces = malloc( sizeof( int ) * 32768 * 24 );
	
	FILE* f = fopen( fn, "r" );
	int c;
	while( ( c = fgetc( f ) ) != EOF ) {
		if( c == 'v' ) {
			//if( fgetc( f ) == 'n' ) {
			if( fgetc( f ) != ' ' ) {
				while( c != '\n' && c != EOF ) {
					c = fgetc( f );
				}
				
			} else {
				float x, y, z;
				fscanf( f, " %f %f %f\n", &x, &y, &z );
				
				verts[numVerts] = (vec3){ x, y, z };
				numVerts++;
				
				//printf( "%f, %f, %f\n", x, y, z );
			}
			
		} else if( c == 'f' ) {
			int v0, v1, v2;
			//fscanf( f, " %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d\n", &v0, &v1, &v2 );
			fscanf( f, " %d%*[^ ] %d%*[^ ] %d%*[^\n]\n", &v0, &v1, &v2 );
			v0 -= 1;
			v1 -= 1;
			v2 -= 1;
			faces[numFaces*3+0] = v0;
			faces[numFaces*3+1] = v1;
			faces[numFaces*3+2] = v2;
			numFaces++;
			
			if( numFaces < 100 ) {
				printf( "%d, %d, %d\n", v0, v1, v2 );
				//fflush( stdout );
			}
			
		} else {
			while( c != '\n' && c != EOF ) {
				c = fgetc( f );
			}
		}
	}
	
	printf( "verts %d\ntris %d\n", numVerts, numFaces );
	fflush( stdout );
}

int triInBox( int tri, vec3 pos, vec3 size ) {
	vec3 v0 = verts[faces[tri*3+0]];
	vec3 v1 = verts[faces[tri*3+1]];
	vec3 v2 = verts[faces[tri*3+2]];
	
	vec3 tmn = vmin( vmin( v0, v1 ), v2 );
	vec3 tmx = vmax( vmax( v0, v1 ), v2 );
	
	vec3 bmn = pos - size;
	vec3 bmx = pos + size;
	
	if( ( tmn.x <= bmx.x && tmx.x >= bmn.x ) &&
		( tmn.y <= bmx.y && tmx.y >= bmn.y ) &&
		( tmn.z <= bmx.z && tmx.z >= bmn.z )
	) {
		// TODO
		return TRUE;
	}
	
	return FALSE;
}

#define NODE_BRANCH 0
#define NODE_LEAF 1

typedef struct {
	vec3 pos, size;
	u32 children[2];
	u8 nodeType;
	int* tris;
	int numTris;
} bvhNode_t;

bvhNode_t bvh[1024];
int bvhEntries = 0;

int bvhSplitThreshold = 256;
int bvhMaxDepth = 8;

void splitBVH( int n, int depth ) {
	int splitAxis = bvh[n].size.x > bvh[n].size.y ? 0 : 1;
	splitAxis = bvh[n].size[splitAxis] > bvh[n].size.z ? splitAxis : 2;
	
	int a = bvhEntries++;
	int b = bvhEntries++;
	
	bvh[n].nodeType = NODE_BRANCH;
	bvh[n].children[0] = a;
	bvh[n].children[1] = b;
	
	bvh[a] = bvh[n];
	bvh[b] = bvh[n];
	
	bvh[a].pos[splitAxis] -= bvh[n].size[splitAxis] / 2.0f;
	bvh[a].size[splitAxis] /= 2.0f;
	
	bvh[b].pos[splitAxis] += bvh[n].size[splitAxis] / 2.0f;
	bvh[b].size[splitAxis] /= 2.0f;
	
	int c[2] = { a, b };
	
	for( int i = 0; i < 2; i++ ) {
		bvh[c[i]].nodeType = NODE_LEAF;
		bvh[c[i]].children[0] = -1;
		bvh[c[i]].children[1] = -1;
		
		bvh[c[i]].numTris = 0;
		bvh[c[i]].tris = malloc( sizeof( int ) * bvhSplitThreshold );
		int mtris = bvhSplitThreshold;
		
		for( int j = 0; j < bvh[n].numTris; j++ ) {
			if( triInBox( bvh[n].tris[j], bvh[c[i]].pos, bvh[c[i]].size ) ) {
				if( bvh[c[i]].numTris == mtris ) {
					mtris *= 2;
					bvh[c[i]].tris = realloc( bvh[c[i]].tris, sizeof( int ) * mtris );
					
				}
				
				bvh[c[i]].tris[bvh[c[i]].numTris] = bvh[n].tris[j];
				bvh[c[i]].numTris++;
			}
		}
		
		printf( "bvh %d tris: %d\n", c[i], bvh[c[i]].numTris );
		fflush( stdout );
		
		if( bvh[c[i]].numTris > bvhSplitThreshold && depth < bvhMaxDepth ) {
			splitBVH( c[i], depth + 1 );
		}
	}
}

void initBVH( void ) {
	vec3 mn = (vec3){ FLT_MAX, FLT_MAX, FLT_MAX };
	vec3 mx = (vec3){ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	
	for( int i = 0; i < numVerts; i++ ) {
		mn = vmin( mn, verts[i] );
		mx = vmax( mx, verts[i] );
	}
	
	bvh[0].nodeType = NODE_BRANCH;
	bvh[0].pos = ( mn + mx ) / 2;
	bvh[0].size = ( mx - mn ) / 2;
	bvh[0].tris = NULL;
	bvh[0].numTris = 0;
	bvh[0].children[0] = 1;
	bvh[0].children[1] = 2;
	
	int splitAxis = bvh[0].size.x > bvh[0].size.y ? 0 : 1;
	splitAxis = bvh[0].size[splitAxis] > bvh[0].size.z ? splitAxis : 2;
	
	bvh[1] = bvh[0];
	bvh[2] = bvh[0];
	
	bvh[1].pos[splitAxis] -= bvh[0].size[splitAxis] / 2.0f;
	bvh[1].size[splitAxis] /= 2.0f;
	
	bvh[2].pos[splitAxis] += bvh[0].size[splitAxis] / 2.0f;
	bvh[2].size[splitAxis] /= 2.0f;
	
	bvhEntries = 3;
	
	for( int n = 1; n < 3; n++ ) {
		bvh[n].nodeType = NODE_LEAF;
		bvh[n].children[0] = -1;
		bvh[n].children[1] = -1;
		
		bvh[n].numTris = 0;
		bvh[n].tris = malloc( sizeof( int ) * bvhSplitThreshold );
		int mtris = bvhSplitThreshold;
		
		for( int i = 0; i < numFaces; i++ ) {
			if( triInBox( i, bvh[n].pos, bvh[n].size ) ) {
				if( bvh[n].numTris == mtris ) {
					mtris *= 2;
					bvh[n].tris = realloc( bvh[n].tris, sizeof( int ) * mtris );
					
				}
				
				bvh[n].tris[bvh[n].numTris] = i;
				bvh[n].numTris++;
			}
		}
		
		printf( "bvh %d tris: %d\n", n, bvh[n].numTris );
		fflush( stdout );
		
		if( bvh[n].numTris > bvhSplitThreshold ) {
			splitBVH( n, 1 );
		}
	}
	
	
}

int testCount;

static inline float traceRayBVH( int n, float preZ, vec3 origin, vec3 dir, vec3* col, vec3* normal ) {
	if( bvh[n].nodeType == NODE_LEAF ) {
		
		vec3 c = { 0, 0, 0 };
		vec3 nrm = { 0, 0, 0 };
		float z = preZ;
		
		for( int i = 0; i < bvh[n].numTris; i++ ) {
			vec3 v0 = verts[faces[bvh[n].tris[i]*3+0]];//-1];
			vec3 v1 = verts[faces[bvh[n].tris[i]*3+1]];//-1];
			vec3 v2 = verts[faces[bvh[n].tris[i]*3+2]];//-1];
			//printf( "ray test {%f,%f,%f}, {%f,%f,%f}, {%f,%f,%f}\n", v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z );
			vec3 t = intersectTri( origin, dir, v0, v1, v2 );
			if( t.x > 0.0f && t.x < z ) {
				//col = tigrRGB( t.y * 255, t.z * 255, ( 1.0f - ( t.y + t.z ) ) * 255 );
				c = (vec3){ t.y, t.z, 1.0f - ( t.y + t.z ) };
				nrm = normalize( cross( v0 - v1, v0 - v2 ) );
				z = t.x;
			}
		}
		
		testCount += bvh[n].numTris;
		
		*col = c;
		*normal = nrm;
		
		return z;
		
		/*
		vec3 bvhN;
		float bvhZ = intersectBox( origin, dir, bvh[n].pos, bvh[n].size, &bvhN );
		
		*normal = bvhN;
		return bvhZ;
		*/
		
	} else {
		int c1 = bvh[n].children[0];
		int c2 = bvh[n].children[1];
		
		vec3 bvhN;
		float bvhZ1 = intersectBox( origin, dir, bvh[c1].pos, bvh[c1].size, &bvhN ).y;
		float bvhZ2 = intersectBox( origin, dir, bvh[c2].pos, bvh[c2].size, &bvhN ).y;
		
		vec3 bvhC1, bvhN1;
		vec3 bvhC2, bvhN2;
		
		if( bvhZ1 >= 0.0f ) {
			bvhZ1 = traceRayBVH( c1, preZ, origin, dir, &bvhC1, &bvhN1 );
		}
		
		if( bvhZ2 >= 0.0f ) {
			bvhZ2 = traceRayBVH( c2, preZ, origin, dir, &bvhC2, &bvhN2 );
		}
		
		int c1Hit = ( bvhZ1 >= 0.0f );
		int c2Hit = ( bvhZ2 >= 0.0f );
		
		if( c1Hit && bvhZ1 < preZ && ( !c2Hit || bvhZ2 > bvhZ1 ) ) {
			*col = bvhC1;
			*normal = bvhN1;
			return bvhZ1;
		}
		
		if( c2Hit && bvhZ2 < preZ ) {
			*col = bvhC2;
			*normal = bvhN2;
			return bvhZ2;
		}
	}
	
	return FLT_MAX;
}

float traceRay( vec3 origin, vec3 dir, vec3* col, vec3* normal ) {
	testCount = 0;
	/*
	vec3 c = { 0, 0, 0 };
	vec3 n = { 0, 0, 0 };
	float z = FLT_MAX;
	
	for( int i = 0; i < numFaces; i++ ) {
		vec3 v0 = verts[faces[i*3+0]];//-1];
		vec3 v1 = verts[faces[i*3+1]];//-1];
		vec3 v2 = verts[faces[i*3+2]];//-1];
		//printf( "ray test {%f,%f,%f}, {%f,%f,%f}, {%f,%f,%f}\n", v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z );
		vec3 t = intersectTri( origin, dir, v0, v1, v2 );
		if( t.x > 0.0f && t.x < z ) {
			//col = tigrRGB( t.y * 255, t.z * 255, ( 1.0f - ( t.y + t.z ) ) * 255 );
			c = (vec3){ t.y, t.z, 1.0f - ( t.y + t.z ) };
			n = normalize( cross( v0 - v1, v0 - v2 ) );
			z = t.x;
		}
	}
	
	*col = c;
	*normal = n;
	
	return z;
	*/
	return traceRayBVH( 0, FLT_MAX, origin, dir, col, normal );
	/*
	vec3 c = { 0, 0, 0 };
	vec3 n = { 0, 0, 0 };
	float z = FLT_MAX;
	
	for( int i = 0; i < bvhEntries; i++ ) {
		if( bvh[i].nodeType == NODE_LEAF ) {
			vec3 n2;
			float z2 = intersectBox( origin, dir, bvh[i].pos, bvh[i].size, &n2 );
			if( z2 > 0.0f && z2 < z && bvh[i].numTris > 0 ) {
				n = n2;
				z = z2;
				//printf( "box hit %d %f\n", i, z );
				return z;
			}
		}
	}
	
	*col = c;
	*normal = n;
	return z;
	*/
}

void renderRegion( Tigr* win, u32* rndState, int x, int y, int w, int h, int tw, int th, int numSamples, float ar, float scale, vec3 forward, vec3 up, vec3 right, vec3 camPos ) {
	for( int ry = y; ry < y + h; ry++ ) {
		for( int rx = x; rx < x + w; rx++ ) {
			vec3 c2 = { 0, 0, 0 };
			
			for( int i = 0; i < numSamples; i++ ) {
				float x2 = (float)rx + frand( rndState ) / 2.0f + 0.5f;
				float y2 = (float)ry + frand( rndState ) / 2.0f + 0.5f;
				float u = ( x2 / tw ) * 2.0f - 1.0f;
				float v = -( ( y2 / th ) * 2.0f - 1.0f );
				u = u * ar * scale;
				v = v * scale;
				vec3 viewRay = normalize( forward + right * u + up * v );
				
				vec3 c = { 0, 0, 0 };
				vec3 n = { 0, 0, 0 };
				float z = traceRay( camPos, viewRay, &c, &n );
				
				vec3 hitPos = camPos + viewRay * ( z - 0.0001f );
				vec3 hitDir = viewRay;
				
				int tc = 0;
				
				//vec3 c3 = (vec3){ 1.0f, 1.0f, 1.0f };
				
				int bounces = 1;
				while( z != FLT_MAX && bounces < 5 ) {
					//vec3 r = normalize( reflect( hitDir, n ) + (vec3){ frand( rndState ), frand( rndState ), frand( rndState ) } * 0.5f );
					vec3 r = normalize( n + (vec3){ frand( rndState ), frand( rndState ), frand( rndState ) } * 0.8f );
					z = traceRay( hitPos, r, &c, &n );
					///c3 *= c;
					//printf( "intersection tests: %d\n", testCount );
					tc += testCount;
					hitPos = hitPos + hitDir * ( z - 0.0001f );
					hitDir = r;
					bounces++;
				}
				
				//z *= 0.5f;
				//float l = z / ( z + 1 );
				//float l = z / 1000.0f;
				//float l = (float)tc / ( tc + 1 );
				float l = 1.0f / bounces;
				c = (vec3){ l, l, l };
				//c = c3;
				
				c2 += c;
			}
			
			c2 /= numSamples;
			
			c2 = vclamp( c2, 0.0f, 1.0f );
			TPixel col = tigrRGB( powf( c2.x, 1.0f / 2.2f ) * 255, powf( c2.y, 1.0f / 2.2f ) * 255, powf( c2.z, 1.0f / 2.2f ) * 255 );
			tigrPlot( win, rx, ry, col );
		}
	}
}

int main( int argc, char* argv[] ) {
	setvbuf( stdout, NULL, _IOLBF, 0 );
	
	u32 rndState[5];
	
	srand( time( NULL ) );
	for( int i = 0; i < 5; i++ ) {
		rndState[i] = rand();
	}
	
	//loadOBJ( "sponza-edit/sponza.obj" );
	loadOBJ( "crytek-sponza/sponza-tri.obj" );
	//loadOBJ( "test2.obj" );
	
	initBVH();
	
	int w = 1280;
	int h = 720;
	
	int numSamples = 64;
	
	Tigr* win = tigrWindow( w, h, "ray4", TIGR_FIXED );
	tigrClear( win, tigrRGB( 0, 0, 0 ) );
	
	vec3 camPos = (vec3){ 0.0f, 0.5f, 0.0f };
	//vec3 camPos = (vec3){ 0.0f, 0.4f, 2.0f };
	vec3 camDir = (vec3){ 1.0f, 0.0f, 0.0f };
	//vec3 camDir = (vec3){ 0.0f, -0.1f, -1.0f };
	vec3 worldUp = (vec3){ 0.0f, 1.0f, 0.0f };
	
	float fovY = 56.0f;
	
	vec3 forward = normalize( camDir );
	vec3 right = normalize( cross( forward, worldUp ) );
	vec3 up = normalize( cross( right, forward ) );
	
	float ar = (float)w / h;
	float scale = tanf( DEG_TO_RAD( fovY ) * 0.5f );
	
	int tileW = 16;
	int tileH = 16;
	
	for( int i = 0; i < w / tileW; i++ ) {
		for( int j = 0; j < h / tileH; j++ ) {
			renderRegion( win, rndState, i * tileW, j * tileH, tileW, tileH, w, h, numSamples, ar, scale, forward, up, right, camPos );
			tigrUpdate( win );
		}
	}
	
	while( !tigrClosed( win ) ) {
		tigrUpdate( win );
	}
	
	return 0;
}

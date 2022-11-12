#ifndef TERRAIN_VEC2_H
#define TERRAIN_VEC2_H
#include "defs.h"

typedef struct 
{
	fp_t x;
	fp_t y;
} vec2;


vec2 add_vec(const vec2 a, const vec2 b);
vec2 sub_vec(const vec2 a, const vec2 b);
fp_t length_vec(const vec2 v);
vec2 normalize_vec(const vec2 v);


#endif //TERRAIN_VEC2_H

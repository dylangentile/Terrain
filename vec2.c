#include "vec2.h"
#include <math.h>

vec2 
add_vec(const vec2 a, const vec2 b)
{
	const vec2 v = {a.x + b.x, a.y + b.y};
	return v;
}

vec2
sub_vec(const vec2 a, const vec2 b)
{
	const vec2 v = {a.x - b.x, a.y - b.y};
	return v;
}

fp_t 
length_vec(const vec2 v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}

vec2 
normalize_vec(const vec2 v)
{
	const fp_t len = length_vec(v);
	if(len == 0.0)
		return v;

	const vec2 n = {v.x/len, v.y/len};
	return n;
}


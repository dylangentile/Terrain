#ifndef TERRAIN_DEFS_H
#define TERRAIN_DEFS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define rassert(x, msg) if(!(x)) {fprintf(stderr, __FILE__":%d -- %s\n", __LINE__, msg); exit(1);} 

typedef struct
{
	int32_t x;
	int32_t y;
} Coord;


typedef struct
{
	union
	{
		struct
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};
		uint32_t value;
	};
} Color;







#endif

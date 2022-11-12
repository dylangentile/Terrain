#ifndef TERRAIN_EROSION_H
#define TERRAIN_EROSION_H
#include "defs.h"


void erode(fp_t* height_map, const Coord height_map_extent, const int64_t iterations);


#endif //TERRAIN_EROSION_H

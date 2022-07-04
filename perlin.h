#ifndef TERRAIN_PERLIN_H
#define TERRAIN_PERLIN_H

#include <stdint.h>

typedef struct
{
    int32_t x;
    int32_t y;
} Coord;

double* generate_noise(const Coord size, const double freq, const double amp, const double granularity);
double* generate_octave_noise(const Coord size, const int32_t octave_count, const double freq_base, const double amp_base, const double granularity);
#endif
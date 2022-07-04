#ifndef TERRAIN_PERLIN_H
#define TERRAIN_PERLIN_H
#include "defs.h"


double* generate_noise(const Coord size, const double freq, const double amp, const double granularity);
double* generate_octave_noise(const Coord size, const int32_t octave_count, const double freq_base, const double amp_base, const double granularity);


double* generate_octave_noise_smp(const int32_t thread_count, const Coord size, const int32_t octave_count, const double freq_base, const double amp_base, const double granularity);
#endif

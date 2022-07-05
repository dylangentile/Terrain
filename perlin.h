#ifndef TERRAIN_PERLIN_H
#define TERRAIN_PERLIN_H
#include "defs.h"
#include "smp.h"


double* generate_noise(const Coord size, const double freq, const double amp, const double granularity);
double* generate_octave_noise(const Coord size, const int32_t octave_count, const double freq_base, const double amp_base, const double granularity);


double* generate_octave_noise_smp(ThreadPool* const tpool, const Coord size, const int32_t octave_count, const double freq_base, const double amp_base, const double granularity);
#endif

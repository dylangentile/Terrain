#ifndef TERRAIN_PERLIN_H
#define TERRAIN_PERLIN_H
#include "defs.h"
#include "smp.h"


fp_t* generate_noise(const Coord size, const fp_t freq, const fp_t amp, const fp_t granularity);
fp_t* generate_octave_noise(const Coord size, const int32_t octave_count, const fp_t freq_base, const fp_t amp_base, const fp_t granularity);


fp_t* generate_octave_noise_smp(ThreadPool* const tpool, const Coord size, const int32_t octave_count, const fp_t freq_base, const fp_t amp_base, const fp_t granularity);
#endif

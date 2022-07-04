#include "perlin.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <assert.h>

double fade(const double t) 
{ 
	return t * t * t * (t * (t * 6 - 15) + 10); 
}

double lerp(const double t, const double a, const double b) 
{ 
	return a + t * (b - a); 
}

double grad(const int hash, const double x, const double y, const double z) {
	const int h = hash & 15;
	const double u = h<8 ? x : y;
	const double v = h<4 ? y : h==12||h==14 ? x : z;
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}


double noise(const double x_, const double y_, const int32_t* p)
{

	const int32_t X = (int32_t)floor(x_) & 255;
	const int32_t Y = (int32_t)floor(y_) & 255;
	const int32_t Z = 12 & 255;	

	const double x = x_ - floor(x_);
	const double y = y_ - floor(y_);
	const double z = 0.6;

	const double u = fade(x);
	const double v = fade(y);
	const double w = fade(z);

	int32_t	A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,
			B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;


	return 	lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),
									grad(p[BA  ], x-1, y  , z   )), 
							lerp(u, grad(p[AB  ], x  , y-1, z   ),  
									grad(p[BB  ], x-1, y-1, z   ))),
					lerp(v, lerp(u,	grad(p[AA+1], x  , y  , z-1 ),  
									grad(p[BA+1], x-1, y  , z-1 )),
							lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
									grad(p[BB+1], x-1, y-1, z-1 ))));
}



double*
generate_noise(const Coord size, const double freq, const double amp, const double granularity)
{
	const int32_t noise_image_size = size.x*size.y;
	double* const noise_image = calloc(noise_image_size, sizeof(*noise_image));

	const int32_t permutation[256] = { 
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,
		20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166, 77,146,158,231,83,111,229,122,60,211,133,
		230,220,105,92,41,55,46,245,40,244, 102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,
		18,169,200,196, 135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,
		38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152, 
		2,44,154,163, 70,221,153,101,155,167, 43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185, 
		112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,
		67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	int32_t p[512];

	memcpy(p, permutation, 256*sizeof(*p));
	memcpy(p+256, permutation, 256*sizeof(*p));

	Coord current_coord;
	double* it = noise_image;

	for(; it != noise_image + noise_image_size; it++)
	{
		*it = amp * noise(((double)current_coord.x)*(freq/granularity), ((double)current_coord.y)*(freq/granularity), p);

		current_coord.x++;
		if(current_coord.x >= size.x)
		{
			current_coord.x = 0;
			current_coord.y++;
		}
	}



	return noise_image;
}



double* 
generate_octave_noise(const Coord size, const int32_t octave_count, const double freq_base, const double amp_base, const double granularity)
{
	const int32_t noise_image_size = size.x*size.y;
	double* const noise_image = calloc(noise_image_size, sizeof(*noise_image));

	double l = 1.0;
	double p = 1.0;

	for(int32_t i = 0; i < octave_count; i++)
	{
		double* oct = generate_noise(size, l, p, granularity);

		for(int32_t j = 0; j < noise_image_size; j++)
			noise_image[j] += oct[j];
		
		free(oct);

		l *= freq_base;
		p *= amp_base;
	}

	return noise_image;
}

typedef struct
{
	Coord size;
	Coord current_coord;
	double* start;
	
	double freq;
	double amp;
	double granularity;

	double* scratch;
	double* final;
	int32_t count;

	int32_t octave_count;

	thrd_t handle;
}ThreadData;


void
generate_noise_work(const ThreadData* const tdata)
{
	const int32_t permutation[256] = { 
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,
		20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166, 77,146,158,231,83,111,229,122,60,211,133,
		230,220,105,92,41,55,46,245,40,244, 102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,
		18,169,200,196, 135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,
		38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152, 
		2,44,154,163, 70,221,153,101,155,167, 43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185, 
		112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,
		67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	int32_t p[512];

	memcpy(p, permutation, 256*sizeof(*p));
	memcpy(p+256, permutation, 256*sizeof(*p));

	Coord current_coord = tdata->current_coord;
	double* it = tdata->start;
	const double* const stop = tdata->start + tdata->count;

	for(; it != stop; it++)
	{
		*it = tdata->amp * noise(	((double)current_coord.x)*(tdata->freq/tdata->granularity), 
									((double)current_coord.y)*(tdata->freq/tdata->granularity), 
									p);

		current_coord.x++;
		if(current_coord.x >= tdata->size.x)
		{
			current_coord.x = 0;
			current_coord.y++;
		}
	}
}


int
thread_work(void* tdata_arg)
{
	ThreadData* const tdata = tdata_arg;

	const double freq_base = tdata->freq;
	const double amp_base = tdata->amp;

	tdata->freq = 1.0;
	tdata->amp = 1.0;

	for(int32_t i = 0; i < tdata->octave_count; i++)
	{
		tdata->start = tdata->scratch;
		generate_noise_work(tdata);

		for(int32_t j = 0; j < tdata->count; j++)
			tdata->final[j] += tdata->scratch[j];

		tdata->freq *= freq_base;
		tdata->amp *= amp_base;
	}

	return 0;
}


double* 
generate_octave_noise_smp(const int32_t thread_count, const Coord size, const int32_t octave_count, const double freq_base, const double amp_base, const double granularity)
{
	const int32_t image_size = size.x*size.y;
	double* const layered_image = calloc(image_size, sizeof(*layered_image));
	double* const scratch_image = calloc(image_size, sizeof(*scratch_image));
	ThreadData* const thread_array = calloc(thread_count, sizeof(*thread_array)); 


	{
		int32_t value_offset = 0;
		const int32_t value_count = image_size/thread_count;
		for(int32_t i = 0; i < thread_count; i++)
		{
			thread_array[i].count = 	value_count;
			thread_array[i].scratch = 	scratch_image + value_offset;
			thread_array[i].final = 	layered_image + value_offset;

			thread_array[i].freq = freq_base;
			thread_array[i].amp = amp_base;
			thread_array[i].granularity = granularity;

			thread_array[i].size = size;

			thread_array[i].current_coord.y = value_offset/size.x;
			thread_array[i].current_coord.x = value_offset%size.x;

			thread_array[i].octave_count = octave_count;

			value_offset += value_count;
		}
		thread_array[thread_count - 1].count = value_count + (image_size%thread_count);
	}


	for(int32_t i = 0; i < thread_count; i++)
		assert(thrd_create(&thread_array[i].handle, &thread_work, thread_array + i) == thrd_success && "failed to create thread!");


	for(int32_t i = 0; i < thread_count; i++)
		assert(thrd_join(thread_array[i].handle, NULL) == thrd_success && "failed to join thread");


	free(scratch_image);
	free(thread_array);

	return layered_image;

	
}


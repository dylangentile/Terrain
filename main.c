#include "perlin.h"
#include "world.h"
#include "smp.h"
#include "erosion.h"

#include "stb_image_write.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>



double 
clamp(const double x, const double min, const double max)
{
	const double z = x < min ? min : x;
	return z > max ? max : z;
}


int
write_monochrome_float_image(const char* path, const Coord image_dim, double* image_buf)
{
	const size_t s = image_dim.x * image_dim.y;
	uint8_t* r_buf = calloc(s, sizeof(*r_buf));

	for(size_t i = 0; i < s; i++)
		r_buf[i] = (uint8_t)clamp(image_buf[i]*255.0, 0.0, 255.0);


	const int ret_val = stbi_write_png(path, image_dim.x, image_dim.y, 1, r_buf, image_dim.x);
	free(r_buf); //if only we had defer!!!
	
	return ret_val;
}



int 
main(const int argc, const char * const argv[])
{
	if(argc < 2)
	{
		fprintf(stderr, "%s: requires output file argument!\n", argv[0]);
		return 1;
	}

	ThreadPool* const tpool = smp_create_thread_pool(16);


	int32_t scale_factor = 4;
	double zoom_out = 1.0;

	const Coord image_dim = {4*1024, 4*1024};
	double* const noise_image = generate_octave_noise_smp(tpool, image_dim, 8, 2.0, 0.5, ((double)scale_factor)*128.0/zoom_out);

	rassert(write_monochrome_float_image("noise.png", image_dim, noise_image),
		"bad noise image write!");

	erode(noise_image, image_dim);

	




	free(noise_image);

	/*
	Color* const world_image = generate_world(image_dim, noise_monochrome_image);

	rassert(stbi_write_png(argv[1], image_dim.x, image_dim.y, 4, world_image, image_dim.x*sizeof(Color)),
		"bad image write!");

	rassert(stbi_write_png("noise.png", image_dim.x, image_dim.y, 1, noise_monochrome_image, image_dim.x),
		"bad image write!");

	free(world_image);
	*/



	smp_destroy_thread_pool(tpool);
	return 0;
}

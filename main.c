#include "perlin.h"
#include "world.h"
#include "smp.h"
#include "erosion.h"

#include "stb_image_write.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>



fp_t 
clamp(const fp_t x, const fp_t min, const fp_t max)
{
	const fp_t z = x < min ? min : x;
	return z > max ? max : z;
}


int
write_monochrome_float_image(const char* path, const Coord image_dim, fp_t* image_buf)
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
	fp_t zoom_out = 4.0;

	const Coord image_dim = {512, 512};
	fp_t* const noise_image = generate_octave_noise_smp(tpool, image_dim, 8, 2.0, 0.5, ((fp_t)scale_factor)*128.0/zoom_out);

	rassert(write_monochrome_float_image("before.png", image_dim, noise_image),
		"bad noise image write!");
	printf("Made Noise image!!!\n");

	erode(noise_image, image_dim, 100000LL);

	
	rassert(write_monochrome_float_image("after.png", image_dim, noise_image),
		"bad noise image write!");



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

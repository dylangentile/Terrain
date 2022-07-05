#include "perlin.h"
#include "world.h"
#include "smp.h"

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

	const Coord image_dim = {scale_factor*1024, scale_factor*1024};
	const int32_t image_size = image_dim.x * image_dim.y;	
	double* const noise_image = generate_octave_noise_smp(tpool, image_dim, 8, 2.0, 0.5, ((double)scale_factor)*128.0/zoom_out);
	//double* const ocean_image = generate_octave_noise_smp(16, image_dim, 8, 1.7, 0.5, ((double)scale_factor)*512.0/zoom_out);
	uint8_t* const noise_monochrome_image = calloc(image_size, sizeof(*noise_monochrome_image));
	//uint8_t* const ocean_monochrome_image = calloc(image_size, sizeof(*ocean_monochrome_image));

	for(int32_t i = 0; i < image_size; i++)
	{
		/*if(ocean_image[i] < 0.001)
			noise_monochrome_image[i] = (uint8_t)clamp((ocean_image[i]*0.5 + 0.5)*114.6, 0.0, 80.0);
		else if(ocean_image[i] < 0.01)
			noise_monochrome_image[i] = 79;
		else*/
			noise_monochrome_image[i] = (uint8_t)clamp((noise_image[i]*0.5 + 0.5)*255.0, 0.0, 255.0);
			

		//ocean_monochrome_image[i] = (uint8_t)clamp((ocean_image[i]*0.5 + 0.5)*255.0, 0.0, 255.0);

	}
	
	free(noise_image);

	Color* const world_image = generate_world(image_dim, noise_monochrome_image);

	rassert(stbi_write_png(argv[1], image_dim.x, image_dim.y, 4, world_image, image_dim.x*sizeof(Color)),
		"bad image write!");

	rassert(stbi_write_png("noise.png", image_dim.x, image_dim.y, 1, noise_monochrome_image, image_dim.x),
		"bad image write!");

	free(noise_monochrome_image);
	free(world_image);


	smp_destroy_thread_pool(tpool);
	return 0;
}







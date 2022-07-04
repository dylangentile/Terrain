#include "perlin.h"
#include "world.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
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

	const Coord image_dim = {4*1024, 4*1024};
	const int32_t image_size = image_dim.x * image_dim.y;	
	double* const noise_image = generate_octave_noise_smp(16, image_dim, 8, 2.0, 0.5, 4.0*128.0);
	uint8_t* const noise_monochrome_image = calloc(image_size, sizeof(*noise_monochrome_image));
	
	for(int32_t i = 0; i < image_size; i++)
		noise_monochrome_image[i] = (uint8_t)clamp((noise_image[i]*0.5 + 0.5)*255.0, 0.0, 255.0);
	
	free(noise_image);

	Color* const world_image = generate_world(image_dim, noise_monochrome_image);

	free(noise_monochrome_image);

	assert(stbi_write_png(argv[1], image_dim.x, image_dim.y, 4, world_image, image_dim.x*sizeof(Color)) &&
		"bad image write!");

	free(world_image);
	return 0;
}







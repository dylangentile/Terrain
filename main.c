#include "perlin.h"

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
	const Coord image_dim = {4096*4, 4096*4};
	const int32_t image_size = image_dim.x * image_dim.y;	
	double* const noise_image = generate_octave_noise(image_dim, 8, 2, 0.5, 1024.0);
	uint8_t* const output_image = calloc(image_size, sizeof(*output_image));
	
	for(int32_t i = 0; i < image_size; i++)
		output_image[i] = (uint8_t)clamp((noise_image[i]*0.5 + 0.5)*255.0, 0.0, 255.0);
	
	assert(stbi_write_png("test.png", image_dim.x, image_dim.y, 1, output_image, image_dim.x) &&
		"bad image write!");


	free(noise_image);

	return 0;
}







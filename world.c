#include "world.h"
#include <stdlib.h>
#include <string.h>


Color* 
generate_world(const Coord size, const uint8_t* noise)
{
	const int32_t image_size = size.x*size.y;
	Color* const world_image = calloc(image_size, sizeof(Color));


	
	for(int32_t i = 0; i < image_size; i++)
	{
		const uint8_t n = noise[i];
		if(n < 57)
		{
			const Color c = {.r=0x00, .g=0x15, .b=0x97, .a=0xFF}; 
			world_image[i] = c;
		}
		else if(n < 60)
		{
			const Color c = {.r=0x47, .g=0xBC, .b=0xFF, .a=0xFF}; 
			world_image[i] = c;
		}
		else if(n < 80)
		{
			const Color c = {.r=0xFC, .g=0xD5, .b=0x90, .a=0xFF}; 
			world_image[i] = c;
		}
		else if(n < 120)
		{
			const Color c = {.r=0x6F, .g=0xFC, .b=0x56, .a=0xFF}; 
			world_image[i] = c;
		}
		else if(n < 170)
		{
			const Color c = {.r=0x30, .g=0x87, .b=0x20, .a=0xFF}; 
			world_image[i] = c;
		}
		else if(n < 200)
		{
			const Color c = {.r=0x61, .g=0x45, .b=0x25, .a=0xFF}; 
			world_image[i] = c;
		}
		else if(n < 220)
		{
			const Color c = {.r=0x52, .g=0x52, .b=0x52, .a=0xFF}; 
			world_image[i] = c;
		}
		else if(n < 235)
		{
			const Color c = {.r=0x38, .g=0x38, .b=0x38, .a=0xFF}; 
			world_image[i] = c;
		}
		else
		{
			const Color c = {.r=0xFF, .g=0xFF, .b=0xFF, .a=0xFF}; 
			world_image[i] = c;
		}
	}


	return world_image;
}

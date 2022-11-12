#include "erosion.h"
#include "vec2.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//based upon the paper by Hans Theobold Beyer


#define access(u, v) height_map[(u)*height_map_extent.x + (v)]


typedef struct
{
	vec2 pos;
	vec2 dir;
	fp_t spd;
	fp_t wat; //water
	fp_t sed; //sediment
}Particle;

typedef struct 
{
	fp_t weight;
	fp_t* height_ptr; //"sediment"
}WeightSediment;

typedef struct 
{
	size_t ws_array_size;
	WeightSediment* ws_array;
}WSA_Data;


fp_t 
rng()
{
	return (fp_t)rand()/(fp_t)RAND_MAX;
}

fp_t 
max(const fp_t x, const fp_t y)
{
	return x > y ? x : y;
}

fp_t
min(const fp_t x, const fp_t y)
{
	return x < y ? x : y;
}




void 
get_height_and_gradient(fp_t* h, vec2* g, const fp_t* height_map, const Coord height_map_extent, const Particle* p)
{
	const Coord coord = {p->pos.x, p->pos.y};
	const vec2 offsets = {p->pos.x - (fp_t)coord.x, p->pos.y - (fp_t)coord.y};


	/*****************
	 * 
	 *  a  b
	 *  c  d
	 * 
	 ****************/

	fp_t a = access(coord.x,     coord.y    );
	fp_t b = access(coord.x + 1, coord.y    );
	fp_t c = access(coord.x,     coord.y + 1);
	fp_t d = access(coord.x + 1, coord.y + 1);


	const vec2 gradient = 
	{
		((b - a)*(1.0-offsets.y) + (d - c)*offsets.y),
		((c - a)*(1.0-offsets.x) + (d - b)*offsets.x)
	};

	*g = gradient;


	*h =  (a * (1 - offsets.x) * (1 - offsets.y)) 
		+ (b * offsets.x * (1 - offsets.y)) 
		+ (c * (1 - offsets.x) * offsets.y) 
		+ (d * offsets.x * offsets.y);

}

void 
erode(fp_t* height_map, const Coord height_map_extent, const int64_t iterations)
{
	srand(69);

	const fp_t p_inertia = 0.03;
	const fp_t p_capacity = 4.0;
	const fp_t p_min_slope = 0.01;
	const fp_t p_deposition = 0.3; //fraction that gets deposited
	const fp_t p_erosion = 0.5; //fraction that gets eroded
	const fp_t p_gravity = 1.0;
	const fp_t p_evaporation = 0.1;
	const fp_t p_erosion_radius = 3.0;
	const int64_t p_lifetime = 30;

	

	const size_t wsa_array_size = height_map_extent.x * height_map_extent.y;
	WSA_Data* const wsa_array = calloc(wsa_array_size, sizeof(*wsa_array));

	for(fp_t y_it = 0.5; y_it < height_map_extent.y - 1; y_it++)
	{
		for(fp_t x_it = 0.5; x_it < height_map_extent.x - 1; x_it++)
		{
			const vec2 pos = {x_it, y_it};
			const vec2 top_left = 
			{
				max(pos.x - p_erosion_radius, 0.0), 
				max(pos.y - p_erosion_radius, 0.0)
			};
			const vec2 bottom_right = 
			{
				min(pos.x + p_erosion_radius, (fp_t)(height_map_extent.x - 1)),
				min(pos.y + p_erosion_radius, (fp_t)(height_map_extent.y - 1))
			};


			const Coord current = {pos.x, pos.y};
			const Coord tl_c = {top_left.x, top_left.y};
			const Coord br_c = {bottom_right.x, bottom_right.y};
			const Coord ws_array_extent = {br_c.x - tl_c.x, br_c.y - tl_c.y};

			//printf("%3d, %3d\n", current.x, current.y);

			WSA_Data* wsa = wsa_array + (current.y*height_map_extent.x + current.x);
			{
				wsa->ws_array_size = ws_array_extent.x*ws_array_extent.y; 
				wsa->ws_array = calloc(wsa->ws_array_size, sizeof(*wsa->ws_array)); //set to zero
			}
			
			fp_t running_sum = 0.0;
			for(int32_t j = tl_c.y; j < br_c.y; j++)
			{
				for(int32_t i = tl_c.x; i < br_c.x; i++)
				{
					const vec2 loc = {i, j};
					if(length_vec(sub_vec(loc, pos)) > p_erosion_radius)
						continue;
					
					WeightSediment* ws = wsa->ws_array + ((j - tl_c.y)*ws_array_extent.x + (i - tl_c.x));
					ws->weight = max(0.0, p_erosion_radius - length_vec(sub_vec(loc, pos)));
					ws->height_ptr = &access(i, j);

					running_sum += ws->weight;
				}
			}

			for(int32_t j = tl_c.y; j < br_c.y; j++)
			{
				for(int32_t i = tl_c.x; i < br_c.x; i++)
				{
					WeightSediment* ws = wsa->ws_array + ((j - tl_c.y)*ws_array_extent.x + (i - tl_c.x));
					ws->weight = ws->weight/running_sum;
				}
			}
		}
	}

	for(int64_t iteration = 0; iteration < iterations; iteration++)
	{		
		Particle drop;
		{
			const vec2 pos = {rng()*((fp_t)(height_map_extent.x - 1)), rng()*((fp_t)(height_map_extent.y - 1))};
			const vec2 dir = {0.0, 0.0};
			drop.pos = pos;
			drop.dir = dir;
			drop.spd = 1.0;
			drop.wat = 1.0;
			drop.sed = 0.0;
		}


		for(int64_t life = 0; life < p_lifetime; life++)
		{
			fp_t height_o;
			vec2 gradient_o;
			get_height_and_gradient(&height_o, &gradient_o, height_map, height_map_extent, &drop);
		

			{
				const vec2 dir = 
				{
					(drop.dir.x * p_inertia) - (gradient_o.x *(1.0 - p_inertia)),
					(drop.dir.y * p_inertia) - (gradient_o.y *(1.0 - p_inertia))
				};

				drop.dir = dir;
			}

			drop.dir = normalize_vec(drop.dir);
			drop.pos = add_vec(drop.pos, drop.dir);

			if(length_vec(drop.dir) == 0.0 	|| drop.pos.x < 0.0 || drop.pos.x >= height_map_extent.x - 1 
											|| drop.pos.y < 0.0 || drop.pos.y >= height_map_extent.y - 1) //this makes it so we can ignore boundary conditions for accesses 
				break;

			fp_t height_new;
			{
				vec2 gradient_new;
				get_height_and_gradient(&height_new, &gradient_new, height_map, height_map_extent, &drop);
			}

			const fp_t delta_h = height_new - height_o;
			const fp_t c = max(-delta_h, p_min_slope) * drop.spd * drop.wat * p_capacity;
			
			const Coord the_coord = {drop.pos.x, drop.pos.y};
			const vec2 offsets = {drop.pos.x - (fp_t)the_coord.x, drop.pos.y - (fp_t)the_coord.y};


			if(drop.sed <= c && delta_h < 0.0) //erode
			{	
				const fp_t erosion_amt = min((c-drop.sed)*p_erosion, -delta_h);
				WSA_Data* wsa = wsa_array + (the_coord.y*height_map_extent.x + the_coord.x);

				for(size_t i = 0; i < wsa->ws_array_size; i++)
				{
					WeightSediment* const ws = wsa->ws_array + i;
					const fp_t amt_to_sub = ws->weight * erosion_amt;
					
					if(!ws->height_ptr)
						continue;

					if(erosion_amt > *ws->height_ptr)
					{
						drop.sed += *ws->height_ptr;
						*ws->height_ptr = 0.0; 
					}
					else
					{
						drop.sed += amt_to_sub;
						*ws->height_ptr -= amt_to_sub;
					}
				}
			}
			else //deposit
			{
				const fp_t deposit_amt = delta_h > 0 ? min(drop.sed, delta_h) : (drop.sed-c)*p_deposition;

				access(the_coord.x,     the_coord.y    ) += deposit_amt * (1.0 - offsets.x) * (1.0 - offsets.y);
				access(the_coord.x + 1, the_coord.y    ) += deposit_amt * offsets.x 		* (1.0 - offsets.y);
				access(the_coord.x,     the_coord.y + 1) += deposit_amt * (1.0 - offsets.x) * offsets.y;
				access(the_coord.x + 1, the_coord.y + 1) += deposit_amt * offsets.x			* offsets.y;
			}

			drop.spd = sqrt(fabs((drop.spd * drop.spd) + (delta_h*p_gravity)));
			drop.wat = drop.wat * (1.0 - p_evaporation);
		}
	}


	for(size_t i = 0; i < wsa_array_size; i++)
		free(wsa_array[i].ws_array);
	free(wsa_array);
}

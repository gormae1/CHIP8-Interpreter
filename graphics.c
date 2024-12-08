#include "graphics.h"
#include "cpu.h"

#include <SDL2/SDL.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 NOTE: there are 2 main ways of rendering all the tiles to the screen: 1. store an SDL matrix and blit the whole thing
																	   2. just run a loop that creates them as needed (not taken here)
*/

const char * color_codes[] = {
    "\0",      // for COLOR_NONE
    "\033[0m", // all off
  
   // start foregrounds
    "\033[5m",
    "\033[30m",
    "\033[31m",
    "\033[33m",
    "\033[32m",
    "\033[35m",
  
    // start backgrounds
    "\033[41m",
    "\033[43m",
    "\033[42m",
    "\033[45m",
    "\033[40m",
  
    // start special
    "\033[1m",
    "\033[4m",
};

int init_map(struct tile_map ** tiles)
{
	(*tiles) = malloc(sizeof(struct tile_map));

	if (*tiles == NULL)
	{
		COLOR_ON_S(COLOR_F_RED);
		fprintf(stderr, "FATAL: memory allocation for tile structure failed");
		return 1;
	}
	

	// malloc / calloc, doesn't really matter in this context
	//(*tiles)->s_pixels = malloc(4 * SCR_WIDTH * SCR_HEIGHT);
	for (int i = 0; i < 4 * SCR_HEIGHT * SCR_WIDTH; i++)
	{
		(*tiles)->s_pixels[i] = 0; 
	}

	if ((*tiles)->s_pixels == NULL)
	{
		COLOR_ON_S(COLOR_F_RED);
		fprintf(stderr, "FATAL: memory allocation for pixels failed");
		return 1;
	}

	(*tiles)->pitch     = SCR_WIDTH * sizeof(Uint32);
	(*tiles)->bpp       = (int) sizeof(Uint32);    // actually size_t
	(*tiles)->pix_count = SCR_WIDTH * SCR_HEIGHT; // - 1;  May be needed depending on the approach. Not currently
	
	SDL_Surface * tmp_surface = SDL_CreateRGBSurfaceWithFormat(0, SCR_WIDTH, SCR_HEIGHT, 32, SDL_PIXELFORMAT_RGB888);
	(*tiles)->color_off          = SDL_MapRGB(tmp_surface->format, 255, 0, 0);
	(*tiles)->color_on           = SDL_MapRGB(tmp_surface->format, 255, 255, 0);
	Uint32 tmp_color = SDL_MapRGB(tmp_surface->format, 255, 0, 0);

	clear_all(*tiles);
	SDL_FreeSurface(tmp_surface);

	return 0;
}

_Bool design_rect(
	struct tile_map * pix_map,
	struct rough_rgb arrgeebee, 
	int width, int height, 
	int x_coord, int y_coord)
{
	_Bool xored = false;
	
	int y_count = 0;
	int x_count = 0;
	
	for (int idx = 0; idx < width*height; idx ++)
	{
		
		int y_off = y_coord*SCR_WIDTH*4 + (y_count * pix_map->pitch);
		int x_off = x_coord*4 + (x_count * 4);
	
		pix_map->s_pixels[x_off + y_off + 0] = arrgeebee.r;
		pix_map->s_pixels[x_off + y_off + 1] = arrgeebee.g;
		pix_map->s_pixels[x_off + y_off + 2] = arrgeebee.b;
		pix_map->s_pixels[x_off + y_off + 3] = SDL_ALPHA_OPAQUE;
					
		if (x_count >= width - 1)
		{
			x_count = 0;
			y_count++;
			
		} else
		{
			x_count++;
		}
	}
	return xored;
}

_Bool xor_bytes(
	struct tile_map * pix_map, 
	int * memory_slice, 
	int N, 
	int x_coord, int y_coord)
{	
	_Bool overlapped = false;
	_Bool is_on = false;
	_Bool is_on_2 = false;

	int bit_encoded_row;
	//regurgitate_gfx_memslice(memory_slice, N);

	//fprintf(stdout, "N: %d\tXCOORD: %d\tYCOORD: %d\n", N, x_coord, y_coord);

	int x_coord_adjusted = x_coord * 10;
	int y_coord_adjusted = y_coord * 10;
	
	for (int i = 0; i < N; i++)	// height
	{
		bit_encoded_row = memory_slice[i];
		for (int col = 0; col < 8; col++)
		{
			is_on = (bit_encoded_row & (0x01 << 7-col)) > 0; // example bit extraction
			//is_on_2 = (bit_encoded_row >> col) & 0x1;
			//printf("%d", is_on_2 == is_on);
			if (is_on)
			{
				_Bool overlap = fill_rect(pix_map, 10, 10, x_coord_adjusted, y_coord_adjusted);
				if (overlap != false)
					overlapped = true;
				
				//printf("OVERLAPPED? :%d\n", overlap);
			}
			
			x_coord_adjusted += 10;
		}
		x_coord_adjusted = (x_coord % 640)* 10; // reset
		y_coord_adjusted += 10;	
	}
	return overlapped;
}

void regurgitate_gfx_memslice(int * slice_of_the_pie, int num_sprinkles)
{
	int i;
	int p;
	uint8_t bit;

	for (i = 0; i < num_sprinkles; i++)
	{
        for (p = 0; p < 8; p++)
		{
			bit = (slice_of_the_pie[i] >> p) & 0x1;
			printf("%d", bit);
		}
	}
	putchar('\n');
}
	
_Bool fill_rect(struct tile_map * pix_map, int width, int height, int x_coord, int y_coord)
{
	int idx_h;
	int idx_w;
	int pos_adj;
	_Bool xored = false;
	int y_wrap = y_coord;
	
	for (idx_h = 0; idx_h < height; idx_h++)
	{
		for (idx_w = 0; idx_w < width; idx_w++)
		{
			if (idx_h + y_coord + 25 > 320+25)
			{	y_wrap = 25;  }
			else
			{  y_wrap = y_coord + 25 + idx_h;  }
			
			pos_adj = (pix_map->pitch * y_wrap) + (idx_w * 4) + 
				((x_coord+25) * 4);
				
			if (PIX_IS_ON(pix_map->s_pixels, pos_adj))
			{
				SET_PIX(pix_map->s_pixels, pos_adj, 0);
				xored = true;	
			} else
			{
				SET_PIX(pix_map->s_pixels, pos_adj, 255);
			}
		}
	}
	return xored;
}

void clear_all(struct tile_map * tiles)
{
	int x              = 0;
	int y              = 0;
	int offset         = 0;
	
	tiles->s_pixels[offset + 0] = 0; // b
	tiles->s_pixels[offset + 1] = 0; // g
	tiles->s_pixels[offset + 2] = 0; // r
	tiles->s_pixels[offset + 3] = SDL_ALPHA_OPAQUE;
	// TODO consider the extra SCR_WIDTH here
	for (int i = 0; i < SCR_HEIGHT * SCR_WIDTH + SCR_HEIGHT; i++)
	{	
		if (x >= SCR_WIDTH)
		{
			x = 0;
			y++;
		} else 
		{ 
			x++; 
		}
		
		offset         = (SCR_WIDTH * 4 * y) + x * 4;
		
		tiles->s_pixels[offset + 0] = 0; // b
		tiles->s_pixels[offset + 1] = 0; // g
		tiles->s_pixels[offset + 2] = 0; // r
		tiles->s_pixels[offset + 3] = SDL_ALPHA_OPAQUE;
	}
}

void print_map(struct tile_map * tile)
{
	for (int i = 0; i < SCR_HEIGHT; i++)
	{
		for (int u = 0; u < SCR_WIDTH; u++)
		{
			printf("%d\n",*((Uint32*)(tile)->s_pixels + i + u));

		}
	}
	printf("end print");
	putchar('\n');
}

void refresh_all(struct tile_map * tiles, SDL_Surface * inner, SDL_Surface * outer, SDL_Surface * main)
{

	SDL_Rect border_layout = (SDL_Rect){.x = 25, .y = 25, .w = 640, .h = 320};
	//update_tiles(tiles, inner);

	SDL_BlitSurface(inner, NULL, outer, &border_layout);
	SDL_BlitSurface(outer, NULL, main, NULL);
}

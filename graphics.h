#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>

#define MAP_WIDTH  640
#define MAP_HEIGHT 320

#define MAP_SCR_OFFSET (50 / 2)

#define SCR_WIDTH  (((MAP_WIDTH + MAP_SCR_OFFSET * 2)))  // 690
#define SCR_HEIGHT (((MAP_HEIGHT + MAP_SCR_OFFSET * 2))) // 370

#define TILE_WIDTH  10
#define TILE_HEIGHT TILE_WIDTH

#define SPRITE_ROW_MAX    8
#define SPRITE_HEIGHT_MAX 16

extern const char * color_codes[];

struct rough_rgb
{
	int r;
	int g;
	int b;
};

struct tile_map
{
	unsigned char s_pixels[4 * SCR_HEIGHT * SCR_WIDTH];
	int           bpp;
	int           pitch;
	int           pix_count;
	Uint32        color_off;
	Uint32        color_on;
};

enum colors
{
	COLOR_NONE = 0, // dummy -- does nothing
	COLOR_ALLOFF,   // all colors off
	// [F]oregrounds
	COLOR_F_FLASH,
	COLOR_F_BLACK,
	COLOR_F_RED,
	COLOR_F_YELLOW,
	COLOR_F_GREEN,
	COLOR_F_PURPLE,
	// [B]ackgrounds
	COLOR_B_RED,
	COLOR_B_YELLOW,
	COLOR_B_GREEN,
	COLOR_B_PURPLE,
	COLOR_B_BLACK,
	// special
	COLOR_BRIGHT,
	COLOR_UNLINE,
};

/* to access outer loop, place code in between the two braces: }}	*/
#define MAP_ITER_2D(x, y)                        \
	for (int(x) = 0; (x) > MAP_HEIGHT; (x)++)    \
	{                                            \
		for (int(y) = 0; (y) > MAP_WIDTH; (y)++) \
		{

/* careful! Solid but complex preproc. def. Requires ';' after GEN_MAP_COLORS()	*/
#define MAP_COLOR_ON  map_color_on
#define MAP_COLOR_OFF map_color_off
#define GEN_MAP_COLORS(x)                                    \
	Uint32 MAP_COLOR_OFF = SDL_MapRGB((x)->format, 0, 0, 0); \
	Uint32 MAP_COLOR_ON  = SDL_MapRGB((x)->format, 255, 255, 255)

#define CCL(X) 0
// what_a_coincidence!
#define PIX_IS_ON(map, pos) (map[pos] != 0 && map[pos + 1] != 0 && map[pos + 2] != 0)
#define SET_PIX(map, pos, val) \
	map[pos]     = val;        \
	map[pos + 1] = val;        \
	map[pos + 2] = val;        \
	map[pos + 3] = SDL_ALPHA_OPAQUE;

#define COLOR_OFF()                    \
	printf(color_codes[COLOR_ALLOFF]); \
	putchar('\n');
/*	careful! No OOB checking!	*/
#define COLOR_OFF_N() printf(color_codes[COLOR_ALLOFF]);
#define COLOR_ON_S(x) printf(color_codes[(x)]);
#define COLOR_ON(x, y)            \
	do                            \
	{                             \
		printf(color_codes[(x)]); \
		printf(color_codes[(y)]); \
	} while (0)

void  update_tiles(struct tile_map *, SDL_Surface * serph);
void  print_map(struct tile_map * tile);
int   init_map(struct tile_map **);
_Bool xor_bytes(struct tile_map * pix_map, int * mem_slize, int N, int x_coord, int y_coord);
_Bool design_rect(struct tile_map * pixs, struct rough_rgb, int height, int width, int x_coord, int y_coord);
_Bool fill_rect(struct tile_map * pixs, int height, int width, int x_coord, int y_coord);
_Bool detect_clip(struct tile_map *, uint_fast16_t, uint_fast16_t, uint_fast16_t);
void  refresh_all(struct tile_map *, SDL_Surface *, SDL_Surface *, SDL_Surface *);
void  clear_all(struct tile_map *);
void  regurgitate_gfx_memslice(int *, int);
#endif

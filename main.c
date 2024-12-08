#include "cpu.h"
#include "graphics.h"
#include "keyboard.h"

#include <SDL2/SDL.h>
#include <inttypes.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h> // *NIX only

int main(void)
{
	SDL_Window *   sdl_window;
	SDL_Renderer * render_migrate;
	SDL_Texture *  main_texture;

	FILE * rom_file;
	char * in_line = NULL;

	struct rough_rgb border_rgb;
	struct tile_map *    tile_surface = NULL;
	struct cpu_members * cpu          = NULL;
	struct keypad *      keys_pad     = NULL;

	_Bool     quit_flag = false;
	_Bool     pass_flag = false;
	
	SDL_Event key_ev;

	size_t file_name_len;
	size_t trim;


	/* **** FIND ROM	**** */
	in_line = readline("rom path:");
	if (in_line == 0 || in_line == NULL)
	{
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}

	if ((rom_file = fopen(in_line, "rb")) == NULL)
	{
		fprintf(stderr, "Can't open %s, trying to load IBM ROM test instead.\n", in_line);
		rom_file = fopen("ROMS/IBM Logo.ch8", "rb"); // force load test
	}


	/* **** INITIALIZE SDL THINGAMABOBS **** */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL video: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	sdl_window = SDL_CreateWindow("Window for CHIP8",
	                              SDL_WINDOWPOS_UNDEFINED,
	                              SDL_WINDOWPOS_UNDEFINED,
	                              SCR_WIDTH, SCR_HEIGHT,
	                              SDL_WINDOW_SHOWN);
	                              
	render_migrate = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);

	if (sdl_window == NULL || render_migrate == NULL)
	{
		fprintf(stderr, "Error initializing SDL window/renderer: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear"); // make the scaled rendering look 'smoother' (AKA Fuzzy)
	SDL_RenderSetLogicalSize(render_migrate, SCR_WIDTH, SCR_HEIGHT);

	main_texture = SDL_CreateTexture(render_migrate,
	                                 SDL_PIXELFORMAT_ARGB8888,
	                                 SDL_TEXTUREACCESS_STREAMING,
	                                 SCR_WIDTH, SCR_HEIGHT);


	/* **** INITIALIZE	**** */
	if (init_map(&tile_surface) != 0 || init_keypad(&keys_pad) != 0 || init_cpu(&cpu) != 0)
	{
		fprintf(stderr, "Failed to initialize modules, memory may be full.\n");
		exit(EXIT_FAILURE);
	}

	/* **** LOAD THE CANNONS	**** */
	if (load_rom_into_mem(cpu, rom_file) != 0)
	{
		fclose(rom_file);
		exit(EXIT_FAILURE);
	}

	/* **** DRAW PRETTY BORDER	**** */
	border_rgb = (struct rough_rgb){.r = 255, .g = 255, .b = 0};
	design_rect(tile_surface, border_rgb, SCR_WIDTH - 1, SCR_HEIGHT - 1, 0, 0);
	border_rgb = (struct rough_rgb){.r = 0, .g = 0, .b = 0};
	design_rect(tile_surface, border_rgb, MAP_WIDTH, MAP_HEIGHT, MAP_SCR_OFFSET, MAP_SCR_OFFSET);

	Uint64       fore_loop = SDL_GetPerformanceCounter();
	unsigned int frames    = 0;

	/* **** MAIN LOOP **** */
	while (quit_flag == false)
	{
		Uint64 start = SDL_GetPerformanceCounter(); // most likely no need for such accuracy

		while (SDL_PollEvent(&key_ev))
		{
			switch (key_ev.type)
			{
				case SDL_QUIT:
					quit_flag = true;
 					break;
				case SDL_KEYDOWN:
					//fill_rect(tile_surface, 10, 100, 0, 300);
					//fill_rect(tile_surface, 1200, 20, 0, 0);
					//fill_rect(tile_surface, 200, 32, 0, 0);
					//fill_rect(tile_surface, 10, 10, 630, 300);
					fill_rect(tile_surface, 8, 300, 140, 150);
							// V[X]: 630, V[Y]: 200
							// W: 25, H:140
					//fprintf(stdout, "K");
					//clear_all(tile_surface);
					cpu->need_to_draw = true;
					scan_keys(keys_pad, key_ev.key.keysym.sym, false);
					break;
				case SDL_KEYUP:
					scan_keys(keys_pad, key_ev.key.keysym.sym, true);
					break;
				default:
					break;
			}
		}
		
		/*  **** EMULATE CYCLE  **** */
		if (!pass_flag)
			if (emulate_cycle(tile_surface, cpu, keys_pad) == false)
				pass_flag = true;


		/* 	**** UPDATE SCREEN	**** */
		if (cpu->need_to_draw)
		{
			SDL_RenderClear(render_migrate);
			SDL_UpdateTexture(main_texture, NULL, tile_surface->s_pixels, tile_surface->pitch);

			SDL_RenderCopy(render_migrate, main_texture, NULL, NULL); //, &border_layout);
			SDL_RenderPresent(render_migrate);
			cpu->need_to_draw = false;
		}

		/* **** FRAMES CALCULATION	**** */
		frames++;

		/* delay so CPU doesn't go to 100%	*/
		Uint64 end  = SDL_GetPerformanceCounter();
		Uint64 freq = SDL_GetPerformanceFrequency();

		float  elapsedMS = (end - start) / (float)freq * 1000.0f;
		double seconds   = (end - fore_loop) / (double)freq;
		
		SDL_Delay(floor(4.99)); // remove FLOPS to increase speed?

		if (seconds > 2.0)
		{
			printf("%d frames in %.2f seconds =  %.2f FPS (%f ms/frame)\n",
			       frames, seconds, frames / seconds, elapsedMS);
			fore_loop = end;
			frames    = 0;
		}
	}

	goto clean_up;

clean_up:
	fclose(rom_file);
	free(keys_pad);
	free(tile_surface);
	free(cpu);
	
	SDL_DestroyRenderer(render_migrate);
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();

	return EXIT_SUCCESS;
}

#include "keyboard.h"
#include <stdbool.h>
#include <SDL2/SDL.h>

enum KEYS {
	K_START_ENUM,
	K_1, K_2, K_3, K_C,
	K_4, K_5, K_6, K_D,
	K_7, K_8, K_9, K_E,
	K_A, K_0, K_B, K_F,
	K_END_ENUM,
};

const Uint32 sdl_keys[KEYS_ON_KEYPAD] = {
	SDLK_1, SDLK_2, SDLK_3, SDLK_4, 
	SDLK_q, SDLK_w, SDLK_e, SDLK_r,
	SDLK_a, SDLK_s, SDLK_d, SDLK_f,
	SDLK_z, SDLK_x, SDLK_c, SDLK_v, 
};

int init_keypad(struct keypad ** keypad_s)
{
	*keypad_s = malloc(sizeof (struct keypad));
	if (*keypad_s == NULL)
	{
		fprintf(stderr, "Failed to allocate memory.\n");
		return 1;
	}
	
	for (int i = 0; i < KEYS_ON_KEYPAD; i++)
	{
		(*keypad_s)->key_list[i].key_type = sdl_keys[i];
		(*keypad_s)->key_list[i].pressed = false; 
	}
	
	return 0;
}

// TODO possible optimization when scanning for up if it is known
// that down key was not in the list of interested keys
_Bool scan_keys(struct keypad * keypad_s, Uint32 key_type, _Bool key_up)
{
	for (int i = 0; i < KEYS_ON_KEYPAD; i++)
	{
		if (key_type == keypad_s->key_list[i].key_type)
		{
			keypad_s->key_list[i].pressed = !key_up; 
			return true;
		}
	}
	return false;
}

_Bool check_key_state(struct keypad * keypad_s, int key)
{
	if (key < 0 || key >= K_END_ENUM)
	{
		return false;
	}
	
	return keypad_s->key_list[key].pressed;
}

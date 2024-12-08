#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>

#define KEYS_ON_KEYPAD	16

extern const Uint8 default_keys[KEYS_ON_KEYPAD];

struct keypad_key {
	_Bool pressed;
	Uint32 key_type;	// mostly for debug
};

struct keypad {	// may seem redundant for now, but eh
	struct keypad_key key_list[KEYS_ON_KEYPAD];
};

int init_keypad(struct keypad **);
_Bool scan_keys(struct keypad *, Uint32, _Bool);
_Bool check_key_state(struct keypad *, int);	// int = idx of key

#endif

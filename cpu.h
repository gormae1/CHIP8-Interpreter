#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>

#include "keyboard.h"
#include "graphics.h"

#define CLOCK_HZ 70
#define CLOCK_RATE_MS ((int) ((1.0 / CLOCK_HZ) * 1000 + 0.5))

#define ROM_CODE_START	0x200

#define DATA_PER_SPRITE	5
#define FONT_SET_SIZE	(16 * 5)

#define MEM_SIZE	((0xFFF+1))	// 4096 / 2 == 2048 (2 bytes)
#define STACK_SIZE	16

struct cpu_members {
	int current_opcode;

	// special registers
	int reg_progcount;
	int reg_index;
	int reg_stackpoint; // only really need integer type capable of holding 0-16

	// registers V0 - VF
	int regs[16];
		
	int stack[STACK_SIZE];
	// TODO: Seperate memory module or no?
	//uint16_t memory_4k[(0xFFF+1)/2];	<--- or could have? 
	int memory_4k[MEM_SIZE];

	int mem_occupied;
	
	_Bool BLOCK_PROGCOUNT;
	_Bool need_to_draw;

	struct timeval current_timer;
	struct timeval trailing_timer;
	
	int delay_timer;
	int sound_timer;
	
	// ignore 
	unsigned int rand_seedv;
	unsigned int * rand_seedp;
};

enum register_names
{
	V0,
	V1,
	V2,
	V3,
	V4,
	V5,
	V6,
	V7,
	V8,
	V9,
	VA,
	VB,
	VC,
	VD,
	VE,
	VF,
};

_Bool emulate_cycle(struct tile_map *, struct cpu_members *, struct keypad *);
void decode_opcode(int, struct tile_map *, struct cpu_members *, struct keypad *);
int load_rom_into_mem(struct cpu_members *, FILE *);
int init_cpu(struct cpu_members **);
int timediff_ms(struct timeval *, struct timeval *);

#endif

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#include "graphics.h"
#include "cpu.h"
#include "keyboard.h"

// uncomment for CPU debug messages
// #define CPU_DEBUG


/// TODO: Analyze ROM at last draw
// TODO: change type?
const int font_set[FONT_SET_SIZE] = 
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

int init_cpu(struct cpu_members ** cpu_s)
{	
	*cpu_s = malloc(sizeof (struct cpu_members));
	if (*cpu_s == NULL)
	{
		fprintf(stderr, "Computer says no (CPU).\n");
		return 1;
	}
	
	for (int idx = 0; idx < FONT_SET_SIZE; idx++)
	{ (*cpu_s)->memory_4k[idx] = font_set[idx]; }

	srand(time(NULL));
	
	(*cpu_s)->BLOCK_PROGCOUNT = false;
	(*cpu_s)->need_to_draw = false;
	(*cpu_s)->delay_timer = 0;
	(*cpu_s)->sound_timer = 0;
	gettimeofday(&((*cpu_s)->trailing_timer), NULL);

	return 0;
}

int load_rom_into_mem(struct cpu_members * cpu_s, FILE * rom)
{
	long rom_file_size; // IN SINGLE OCTET BYTES
	
	if (fseek(rom, (long)0, SEEK_END) != 0)
	{
		fprintf(stderr, "Weird. Cannot seek through ROM file. Invalid ROM?\n");
		return 1;
	}
	
	rom_file_size = ftell(rom);
	if (rom_file_size > MEM_SIZE-FONT_SET_SIZE)
	{
		fprintf(stderr, "ROM too large to fit into memory. Invalid ROM?\n");
		return 1;
	}

	if (fseek(rom, (long)0, SEEK_SET) != 0)
	{
		fprintf(stderr, "Weird. Cannot seek through ROM file anymore. Invalid ROM?\n");
		return 1;
	}
	printf("size of file: %ld\n", rom_file_size);
	
	// before 0x200 is the font
	cpu_s->reg_progcount = 0x200;	// 512
	cpu_s->current_opcode = 0;
	cpu_s->reg_index = 0;
	cpu_s->reg_stackpoint = 0;

	for (long idx = 0; idx < rom_file_size; idx++)
	{
		int err = fread(&cpu_s->memory_4k[idx + ROM_CODE_START], 1, 1, rom);
		if (err == 0 || err != 1)
		{
			fprintf(stderr, "fread() returns strange value: %d\n", err);
			continue; // ignore strange value for now
		}
		printf("0x%x ", cpu_s->memory_4k[idx + ROM_CODE_START]);
		if (idx % 8 == 0)
		{ putchar('\n'); } 
	}
	
	cpu_s->mem_occupied = rom_file_size + ROM_CODE_START;
	COLOR_ON_S(COLOR_F_GREEN);
	fprintf(stdout, "\nROM read successfully:\n\toccupies %d memory\n", cpu_s->mem_occupied);
	COLOR_OFF_N();
	
	//uint8_t back_track = 0;
	// concat bytes
	// TODO ensure compatibility with different Endianness
	//for (long idx = 0; idx < (rom_file_size); idx+=2)
	//{
		//int byte1 = buf[idx];
		//int byte2 = buf[idx+1];
		//back_track = (uint8_t)(idx/2);
		//cpu_s->memory_4k[back_track + FONT_SET_SIZE] =  (byte1 << 8 | byte2);
	//}
	
	printf("MEMSIZE (IN [OCTET] BYTES) %ld\n", (ROM_CODE_START + rom_file_size));
	
	return 0;
}

_Bool emulate_cycle(struct tile_map * tiles, struct cpu_members * cpu, struct keypad *keypad_s)
{
	if (cpu->reg_progcount > cpu->mem_occupied)
	{
		fprintf(stdout, "Ran out of instructions. Program counter %d\n", cpu->reg_progcount);
		return false;
	}
	
	gettimeofday(&cpu->current_timer, NULL);

	//if (cpu->delay_timer > 0) {
		//cpu->delay_timer--;
	//}
	int op = cpu->memory_4k[cpu->reg_progcount] << 8 | cpu->memory_4k[cpu->reg_progcount + 1];	// necessary because we must set the opcode

	if (cpu->BLOCK_PROGCOUNT == false)
		cpu->reg_progcount += 2;
		
	decode_opcode(op, tiles, cpu, keypad_s);

#ifdef CPU_DEBUG
	COLOR_ON(COLOR_F_BLACK, COLOR_B_YELLOW);
	printf("CLOCK RATE: %d", timediff_ms(&cpu->current_timer, &cpu->trailing_timer));
	COLOR_OFF();
#endif
	//if (timediff_ms(&cpu->current_timer, &cpu->trailing_timer) >= CLOCK_RATE_MS)
	//{
		//printf("DELAY TIMER IS ON");
		//cpu->trailing_timer = cpu->current_timer;
		
		if (cpu->delay_timer > 0) {
			cpu->delay_timer--;
		}
		if (cpu->sound_timer > 0) {
			cpu->sound_timer--;
		}
	//}
	return true;
}

void decode_opcode(
	int opcode,
	struct tile_map * tiles, 
	struct cpu_members * cpu, 
	struct keypad * keypad_s) // for now, keeps emulate_cycle() cleaner
{
	// should all be uint8_t ? No smaller (i.e. 4t?)
	int full_opcode  = 	opcode;
	int first_nib    = 	(full_opcode & 0xF000) >> 12;
	int second_nib   = 	(full_opcode & 0x0F00) >> 8;
	int third_nib    = 	(full_opcode & 0x00F0) >> 4;
	int fourth_nib   = 	(full_opcode & 0x000F);
	
	int nnn			 =	full_opcode & 0x0FFF;
	int nn 	         = 	full_opcode & 0x00FF;
	int n	         =	full_opcode & 0x000F;

	int memory_slice[fourth_nib+1];

	//printf("Opcode: %s%x\n", full_opcode == 0xE0 ? "00" : "", full_opcode ); 
	//printf("\tIDX: %d\n\tPC: %d\n\t1st:%d, 2nd:%d, 3rd:%d, 4th:%d\n", cpu->reg_index, cpu->reg_progcount, first_nib, second_nib, third_nib, fourth_nib);
	switch (first_nib)
	{
		case 0x0:
			switch (full_opcode)
			{
				case 0x00E0:
					clear_all(tiles);
					break;
				case 0x00EE:
					cpu->reg_progcount = cpu->stack[cpu->reg_stackpoint];
					cpu->stack[cpu->reg_stackpoint] = 0;	// pop?
					cpu->reg_stackpoint--;
					break;
			}
			break;
		case 0x1:
			cpu->reg_progcount = nnn;
			//COLOR_ON_S(COLOR_F_FLASH);
			//fprintf(stdout, "Set the program counter to 0x%x\n", nnn);
			//COLOR_OFF_N();
			break;
		case 0x2:
			cpu->reg_stackpoint++;
			cpu->stack[cpu->reg_stackpoint] = cpu->reg_progcount;
			cpu->reg_progcount = nnn;
			break;
		case 0x3:
			if (cpu->regs[second_nib] == nn)
				cpu->reg_progcount += 2;
			break;
		case 0x4:
			if (cpu->regs[second_nib] != nn)
				cpu->reg_progcount += 2;
			break;
		case 0x5:
			if (cpu->regs[second_nib] == cpu->regs[third_nib])
				cpu->reg_progcount += 2;
			break;
		case 0x6:
			cpu->regs[second_nib] = nn;
			COLOR_ON_S(COLOR_F_RED);
			//printf("Setting V%d to %d\n", second_nib, nn);
			COLOR_OFF_N();
			break;
		case 0x7:
			cpu->regs[second_nib] += nn;
			break;
		case 0x8:
			switch (fourth_nib)
			{
				case 0x0:
					cpu->regs[second_nib] = cpu->regs[third_nib];
					break;
				case 0x1:
					cpu->regs[second_nib] |= cpu->regs[third_nib];
					break;
				case 0x2:
					cpu->regs[second_nib] &= cpu->regs[third_nib];
					break;
				case 0x3:
					cpu->regs[second_nib] ^= cpu->regs[third_nib];
					break;
				case 0x4:
					if (cpu->regs[second_nib] + cpu->regs[third_nib] > 0xFF)
						cpu->regs[VF] = 1;
					else
						cpu->regs[VF] = 0;
						
					cpu->regs[second_nib] += cpu->regs[third_nib];
					break;
				case 0x5:
					if (cpu->regs[second_nib] > cpu->regs[third_nib])
					{
						cpu->regs[VF] = 1;
					} else
					{
						cpu->regs[VF] = 0;
					}
					cpu->regs[second_nib] -= cpu->regs[third_nib];
					break;
				case 0x6:
					cpu->regs[VF] = cpu->regs[second_nib] & 0x01; // least sig bit
					cpu->regs[second_nib] /= 2;
					break;
				case 0x7:
					if (cpu->regs[third_nib] > cpu->regs[second_nib])
					{
						cpu->regs[VF] = 1;
					} else
					{
						cpu->regs[VF] = 0;
					}
					cpu->regs[second_nib] = cpu->regs[third_nib] - cpu->regs[second_nib];
					break;
				case 0xE:
					cpu->regs[VF] = cpu->regs[second_nib] & 0x80; // most sig bit
					cpu->regs[second_nib] *= 2;
					break;
				default:
					fprintf(stderr, "Invalid (0x8...) opcode: 0x%x\n", cpu->current_opcode);
					break;
			}
			break;
		case 0x9:
			if (cpu->regs[second_nib] != cpu->regs[third_nib])
				cpu->reg_progcount += 2;
			break;
		case 0xA:
			cpu->reg_index = nnn; 
			break;
		case 0xB:
			cpu->reg_progcount = cpu->regs[V0] + nnn; 
			break;
		case 0xC:
			;int rand_byte = rand();
			cpu->regs[second_nib] = rand_byte & nn;
			break;
		case 0xD:
			cpu->need_to_draw = true;
			for (int i = 0; i < fourth_nib; i++)
			{
				memory_slice[i] = cpu->memory_4k[cpu->reg_index + i];
			}
			_Bool overlapped = xor_bytes(tiles, memory_slice, fourth_nib, cpu->regs[second_nib], cpu->regs[third_nib]); 
			if (overlapped)
			{
				COLOR_ON_S(COLOR_F_PURPLE);
				cpu->regs[VF] = overlapped;
				//printf("OVERLAPPED");
				COLOR_OFF_N();
			}
			break;
		case 0xE: 
			switch(third_nib)
			{
					case 0x9:
						if (check_key_state(keypad_s, cpu->regs[second_nib]))
						{
							cpu->reg_progcount += 2;	// skip
						}
						break;
					
					case 0xA:
						//COLOR_ON(COLOR_B_YELLOW, COLOR_F_BLACK);
						//printf("CHECKING FOR KEY: %d. SURVEY SAYS: %d", cpu->regs[second_nib], check_key_state(keypad_s, cpu->regs[second_nib]));
						//COLOR_OFF();
						if (check_key_state(keypad_s, cpu->regs[second_nib]) == 0)
						{
							cpu->reg_progcount += 2;	// skip
						}
						break;
			}
			break;
		case 0xF:
			switch(nn)
			{
				case 0x0A:
					if (cpu->BLOCK_PROGCOUNT == false)
					{
						COLOR_ON_S(COLOR_F_YELLOW);
						cpu->reg_progcount -= 2;	// stay on the same opcode in order to keep looping
						printf("Now BLOCKING\n");
						COLOR_OFF_N();
					}
					
					cpu->BLOCK_PROGCOUNT = true;
					
					if (check_key_state(keypad_s, second_nib))
					{
						COLOR_ON(COLOR_F_YELLOW, COLOR_B_BLACK);
						printf("UNBLOCKING PG");
						cpu->BLOCK_PROGCOUNT = false;
						COLOR_OFF();
						cpu->reg_progcount += 2;
					}
					
					break;
				case 0x07:
					cpu->regs[second_nib] = cpu->delay_timer;
					break;
				case 0x15:
					cpu->delay_timer = cpu->regs[second_nib]; 
					break;
				case 0x18:
					cpu->sound_timer = cpu->regs[second_nib];
					break;
				case 0x1E:
					cpu->reg_index += cpu->regs[second_nib];
					break;
				case 0x29:
					if (cpu->regs[second_nib] > (FONT_SET_SIZE / DATA_PER_SPRITE))
						printf("Out of bounds sprite character in V%d: %d\n", second_nib, cpu->regs[second_nib]);
					cpu->reg_index = cpu->regs[second_nib] * DATA_PER_SPRITE; // shift into position of sprite lane
					break;
				case 0x33:
				;
					// using this method instead of modulo for slight performance (not that it matters on regular computers)
					int hundreds = cpu->regs[second_nib] / 100;
					int tens = (cpu->regs[second_nib] - ((int) (hundreds * 100))) / 10;
					int ones = (cpu->regs[second_nib] - ((int) (hundreds * 100))) - tens * 10;
					cpu->memory_4k[cpu->reg_index] = hundreds;
					cpu->memory_4k[cpu->reg_index + 1] = tens;
					cpu->memory_4k[cpu->reg_index + 2] = ones;
					break;	
				case 0x55:
					;
					for (int i = 0; i <= second_nib; i++)
					{
						cpu->memory_4k[cpu->reg_index + i] = cpu->regs[i];
					}
					break;
				case 0x65:
					;
					for (int i = 0; i <= second_nib; i++)
					{
						cpu->regs[i] = cpu->memory_4k[cpu->reg_index + i];
					}
					break;
				default:
					fprintf(stderr, "Invalid (0xF...) opcode: 0x%x\n", full_opcode);
					break;
			}
			break;
		default:
			printf("Couldn't recognize instruction: 0x%x\n", full_opcode);
			break;
	}
}

int timediff_ms(struct timeval *end, struct timeval *start)
{
    int diff =  (end->tv_sec - start->tv_sec) * 1000 +
                (end->tv_usec - start->tv_usec) / 1000;
    return diff;
}

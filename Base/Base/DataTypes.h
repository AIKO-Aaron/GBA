#pragma once

#include <stdint.h>
#include <stdio.h>

typedef uint32_t word;
typedef uint16_t hword;
typedef uint8_t byte;

// All modes the cpu can be in (Almost equal to the register banks)
constexpr word MODE_USER 		= 0b10000;
constexpr word MODE_FIQ 		= 0b10001;
constexpr word MODE_IRQ 		= 0b10010;
constexpr word MODE_SUPERVISOR 	= 0b10011;
constexpr word MODE_ABORT 		= 0b10111;
constexpr word MODE_UNDEFINED 	= 0b11011;
constexpr word MODE_SYSTEM 		= 0b11111;

// All register bank constants (Internal)
constexpr word BANK_USER_SYSTEM 		= 0;
constexpr word BANK_FIQ 				= 1;
constexpr word BANK_SUPERVISOR 			= 2;
constexpr word BANK_ABORT 				= 3;
constexpr word BANK_IRQ 				= 4;
constexpr word BANK_UNDEFINED 			= 5;


// The flags which can be set through operations
constexpr word FLAG_N = (1 << 31);
constexpr word FLAG_Z = (1 << 30);
constexpr word FLAG_C = (1 << 29);
constexpr word FLAG_V = (1 << 28);

// Flags that change how the cpu runs
constexpr word FLAG_I = (1 << 7);
constexpr word FLAG_F = (1 << 6);
constexpr word FLAG_T = (1 << 5);

// Names for all the registers
constexpr word R0   = 0;
constexpr word R1   = 1;
constexpr word R2   = 2;
constexpr word R3   = 3;
constexpr word R4   = 4;
constexpr word R5   = 5;
constexpr word R6   = 6;
constexpr word R7   = 7;
constexpr word R8   = 8;
constexpr word R9   = 9;
constexpr word R10  = 10;
constexpr word R11  = 11;
constexpr word R12  = 12;
constexpr word R13  = 13;
constexpr word R14  = 14;
constexpr word R15  = 15;
constexpr word CPSR = 16;
constexpr word SPSR = 17;

// Alternative names
constexpr word SP = 13;
constexpr word LR = 14;
constexpr word PC = 15;


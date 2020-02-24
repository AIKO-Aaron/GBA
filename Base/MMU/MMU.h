#pragma once

#include "../Base/DataTypes.h"
#include "../Base/FileReader.h"
#include "../Audio/Audio.h"
#include <stdlib.h>
#include <cstring>
#include <stdio.h>
#include <string>
#include <iostream>

namespace Base {
	
    class Timers;
    
	class MMU {
	private:
		byte *bios = nullptr;
		byte *wram_1 = nullptr;
		byte *wram_2 = nullptr;
		byte *io = nullptr;
		byte *palette = nullptr;
		byte *vram = nullptr;
		byte *oam = nullptr;
		byte *null_page = nullptr;
		byte *flash_memory = nullptr;
		
		byte gpio = 0;

		byte *cart = nullptr;
		
		word *bit_masks = new word[16];


		friend class Timers;
		void dma_transfer(hword cntrl, word start, word end, word num);
		void timer_dma(bool is_timer_1);
		void start_dma(byte dma);
		
		// Flash stuff
		bool flash_check(word address, word value);
		bool id_mode = false;
		bool change_bank = false;
		int flash_cmd_stage = 0;

		//Audio
		Base::AudioController* controller;
        
	public:
		MMU();
		~MMU();
		byte **memory = nullptr;
        Timers *timers;

		bool button_a = false, button_b = false, button_select = false, button_start = false, button_up = false, button_down = false, button_left = false, button_right = false;
		bool breakpoint_hit = false;

		void on_hblank();
		void on_vblank();

		MMU(MMU &mmu);
		int i = 0;
		word addr, game_state_obj_addr = 0xFFFFFFFF;

		inline byte r8(word address) { 
			if (address == 0x0890000C4) return gpio; 
			if (id_mode && address == 0x0E000000) return 0xC2;
			if (id_mode && address == 0x0E000001) return 0x09;
			return *(memory[(address >> 24) & 0xF] + (address % bit_masks[(address >> 24) & 0xF]));
		}
		inline hword r16(word address) { 
			if (address == 0x04000130) return 0x3F00 | (button_a ? 0 : 1) | (button_b ? 0 : 2) | (button_select ? 0 : 4) | (button_start ? 0 : 8) | (button_right ? 0 : 16) | (button_left ? 0 : 32) | (button_up ? 0 : 64) | (button_down ? 0 : 128);
			if (address == 0x0BFFFFE0 + 2 * i) return 0xFFF0 + i++; 
			return *(hword*)(memory[(address >> 24) & 0xF] + (address % bit_masks[(address >> 24) & 0xF])); 
		}
		inline word r32(word address) { if (address >= 0x10000000) return 0;  return *(word*)(memory[(address >> 24) & 0xF] + (address % bit_masks[(address >> 24) & 0xF])); }

		bool halted = false;
        
        void check_stuff(word addr, word val);
		void pre_check(word address, word val);

        inline void w8(word address, byte val) {
			pre_check(address, val);
			if (((address >> 24) & 0xF) == 0xE) if (flash_check(address, val)) return;
			if(address == 0x04000410 || address < 0x02000000 || ((address >> 24) == 0x8)) return;
            *(memory[(address >> 24) & 0xF] + (address % bit_masks[(address >> 24) & 0xF])) = val;
			check_stuff(address, val);
        }
        
		inline void w16(word address, hword val) {
			if(address < 0x02000000 || ((address >> 24) == 0x8)) return;
			*(hword*)(memory[(address >> 24) & 0xF] + (address % bit_masks[(address >> 24) & 0xF])) = val;
            
			check_stuff(address, val);
			check_stuff(address + 1, val >> 8);
		}

        inline void w32(word address, word val) {
			if(address < 0x02000000 || ((address >> 24) == 0x8)) return;
			*(word*)(memory[(address >> 24) & 0xF] + (address % bit_masks[(address >> 24) & 0xF])) = val;
            check_stuff(address, val);
            check_stuff(address + 1, val >> 8);
            check_stuff(address + 2, val >> 16);
            check_stuff(address + 3, val >> 24);
		}
    };
	
}

#include "MMU.h"
#include <vector>
#include "../Timers/Timers.h"

Base::MMU::MMU() {
	bios = Base::readFile("bios.gba").data; //(byte*) malloc(0x4000);
	filedata cartridge_full = Base::readFile("emerald.gba");
    cart = cartridge_full.data;
    
	wram_1 = (byte*) malloc(0x40000);
	wram_2 = (byte*) malloc(0x8000);
	io = (byte*) malloc(0x3FF);
	palette = (byte*) malloc(0x400);
	vram = (byte*) malloc(0x18000);
	oam = (byte*) malloc(0x400);

	null_page = (byte*) malloc(1);
	null_page[0] = 0;
	
    memset(wram_1, 0, 0x40000);
    memset(wram_2, 0, 0x8000);
    memset(io, 0, 0x3FF);
    memset(palette, 0, 0x400);
    memset(vram, 0, 0x18000);
    memset(oam, 0, 0x400);
    
	memory = new byte*[16];
	memory[0x0] = bios;
	memory[0x1] = null_page;
	memory[0x2] = wram_1;
	memory[0x3] = wram_2;
	memory[0x4] = io;
	memory[0x5] = palette;
	memory[0x6] = vram;
	memory[0x7] = oam;
	

	for(byte i = 0; i < 6; i++) {
		memory[0x8 + i] = cartridge_full.size >= 0x1000000u * i ? cart + 0x1000000u * i : null_page;
		word size = cartridge_full.size - 0x1000000u * i;
		bit_masks[0x8 + i] = cartridge_full.size >= 0x1000000u * i ? (size >= 0x1000000u ? 0x1000000u : size) : 1u; 
	}

	memory[0xE] = (byte*) malloc(0x10000); // Actually cart_ram
	bit_masks[0xE] = 0x00010000;

	bit_masks[0] = 0x00FFFFFF + 1;
	bit_masks[1] = 0x00000000 + 1;
	bit_masks[2] = 0x0003FFFF + 1;
	bit_masks[3] = 0x00007FFF + 1;
	bit_masks[4] = 0x00FFFFFF + 1;
	bit_masks[5] = 0x000003FF + 1;
	bit_masks[6] = 0x00017FFF + 1;
	bit_masks[7] = 0x000003FF + 1;

	// Default values for IO:
	w32(0x04000088, 0x200);

	w8(0x0E000000, 0xC2);
	w8(0x0E000001, 0x09);
	w16(0x04000130, 0x3FF);
}

Base::MMU::~MMU() {
	free(bios);
    free(cart);
	free(wram_1);
	free(wram_2);
	free(io);
	free(palette);
	free(vram);
	free(oam);
}

Base::MMU::MMU(MMU &mmu) {
	bios = (byte*) malloc(0x4000);
    cart = (byte*) malloc(0x6000000);
	wram_1 = (byte*) malloc(0x40000);
	wram_2 = (byte*) malloc(0x8000);
	io = (byte*) malloc(0x3FF);
	palette = (byte*) malloc(0x400);
	vram = (byte*) malloc(0x18000);
	oam = (byte*) malloc(0x400);
	
	memcpy(bios, mmu.bios, 0x4000);
    memcpy(cart, mmu.cart, 0x6000000);
	memcpy(wram_1, mmu.wram_1, 0x40000);
	memcpy(wram_2, mmu.wram_2, 0x8000);
	memcpy(io, mmu.io, 0x3FF);
	memcpy(palette, mmu.palette, 0x400);
	memcpy(vram, mmu.vram, 0x18000);
	memcpy(oam, mmu.oam, 0x400);
}

byte directions = 0x0;
bool clock = false;
byte currentValue = 0, currentBit = 0;
std::vector<byte> toWrite;
int receiving_data = 0;

inline void Base::MMU::pre_check(word address, word val) {
	if (address == 0x080000C8 || address == 0x080000C9) {
		printf("##################################################################################### Written to IO Port Control...\n");
	}
	else if (address == 0x080000C6 || address == 0x080000C7) {
		printf("##################################################################################### Written to IO Port Direction: %.02X\n", val);
		directions = val & 0xF;
	}
	else if (address == 0x080000C4 || address == 0x080000C5) {
		if ((val & 0x1) && !clock && (val & 0x4)) {
			if (toWrite.size() > 0) {
				printf("Reading data from the chip\n");
				bool bit = toWrite[0] >> (7 - currentBit++);
				if (currentBit == 8) {
					toWrite.erase(toWrite.begin());
					currentBit = 0;
				}
				gpio = (val & 0xFD) | (bit ? 2 : 0);
			}
			else {
				bool bit = (val >> 1) & 1;
				currentValue |= (bit ? 1 : 0) << currentBit++;
				if (currentBit >= 8) {
					printf("##################################################################################### Received byte: %.02X\n", currentValue);
					if (!receiving_data && (currentValue & 0x6) == 0x6) {

						byte cmd = currentValue & 0x70;
						if (currentValue & 0x80) {
							// Reading....
							if (cmd == 0x20) {
								// date and time
								for (int i = 0; i < 7; i++) toWrite.push_back(1);
							}
							else if (cmd == 0x60) {
								for (int i = 0; i < 3; i++) toWrite.push_back(1);
							}
							else if (cmd == 0x40) {
								toWrite.push_back(0x41);
							}
						}
						else if(receiving_data) {
							--receiving_data;

							printf("##################################################################################### Received param byte: %.02X\n", currentValue);
						}

					}
					currentBit = 0;
					currentValue = 0;
				}
			}
		}
		clock = val & 1;
	}
}

void Base::MMU::check_stuff(word address, word value) {
    if(address == 0x040000BB && (value & 0x80)) {
        printf("DMA control 0 set!\n");
    }
    if(address == 0x040000C7 && (value & 0x80)) {
        //printf("DMA control 1 set! : %.04X\n", r16(0x040000C6));
        //printf("\tCopying from: %.08X\n\tCopying to    %.08X\n\tCopy amount:  %.04X\n", r32(0x040000BC), r32(0x040000C0), r16(0x040000C4));
    }
    if(address == 0x040000D3 && (value & 0x80)) {
        //printf("DMA control 2 set!\n");
    }
    if(address == 0x040000DF && (value & 0x80)) {
        //printf("DMA control 3 set!\n");
        //printf("\tCopying from: %.08X\n\tCopying to    %.08X\n\tCopy amount:  %.04X\n", r32(0x040000D4), r32(0x040000D8), r16(0x040000DC));
        
        bool bit_32 = value & 0x04;
        
        word start_addr = r32(0x040000D4);
        word cpy_addr = r32(0x040000D8);

		if (((cpy_addr) & 0x0F000000) == 0x08000000) {
			printf("What the hell is going on?... Writing to %.08X\n", cpy_addr);
		}

        for(int addr_off = 0; addr_off < r16(0x040000DC) * (bit_32 ? 4 : 2); addr_off += bit_32 ? 4 : 2) {
            if(bit_32) w32(cpy_addr + addr_off, r32(start_addr + addr_off));
            else w16(cpy_addr + addr_off, r16(start_addr + addr_off));
        }
    }
    
    // if(address == 0x04000200 || address == 0x04000201) printf("Wrote to IE register: %.04X\n", r16(0x04000200));
    
    if(address == 0x04000202) {
        memory[4][0x202] &= ~value;
    }
    
    if(address == 0x04000301) {
        halted = true;
    }
    
    if(address == 0x0400010C) {
        timers->t3_reload = value;
    }
    
    if(address >= 0x10000000) printf("Illegal address written to at: %.08X\n", i);
}

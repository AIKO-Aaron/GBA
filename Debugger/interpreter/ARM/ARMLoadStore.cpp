#include "ARMInstructions.h"

void Debugger::arm_load(word instruction, Base::CPU *cpu) {
    word base_val = cpu->reg((instruction >> 16) & 0xF).data.reg32;
    bool pre = (instruction & (1 << 24));
    if(((instruction >> 16) & 0xF) == 0xF) base_val += 4;
    word off;
    if(instruction & (1 << 25)) off = Base::shift((instruction >> 4) & 0xFF, cpu->reg(instruction & 0xF).data.reg32, cpu, false);
    else off = instruction & 0xFFF;
    
    word total_address = base_val;
    if(pre) total_address += ((instruction & (1 << 23)) ? off : (~off + 1));
    
    // if(total_address % 4 != 0) rotate right?
        
    if(instruction & (1 << 22)) { // Load single byte
        cpu->reg((instruction >> 12) & 0xF).data.reg32 = (word) cpu->r8(total_address);
    } else { // Load word
        cpu->reg((instruction >> 12) & 0xF).data.reg32 = cpu->r32(total_address);
    }
    
    if(!pre) total_address += ((instruction & (1 << 23)) ? off : (~off + 1));

    if(instruction & (1 << 21) || !pre) cpu->reg((instruction >> 16) & 0xF).data.reg32 = total_address; // Writeback
}

void Debugger::arm_store(word instruction, Base::CPU *cpu) {
    word base_val = cpu->reg((instruction >> 16) & 0xF).data.reg32;
    bool pre = (instruction & (1 << 24));
    word off;
    if(instruction & (1 << 25)) off = Base::shift((instruction >> 4) & 0xFF, cpu->reg(instruction & 0xF).data.reg32, cpu, false);
    else off = instruction & 0xFFF;

    word total_address = base_val;
    if(pre) total_address += ((instruction & (1 << 23)) ? off : (~off + 1));
    
    // if(total_address % 4 != 0) rotate right?
    
    if(instruction & (1 << 22)) { // Store single byte
        cpu->w8(total_address, cpu->reg((instruction >> 12) & 0xF).data.reg32 & 0xFF);
    } else { // Store word
        cpu->w32(total_address, cpu->reg((instruction >> 12) & 0xF).data.reg32);
    }
    
    if(!pre) total_address += ((instruction & (1 << 23)) ? off : (~off + 1));

    if(instruction & (1 << 21) || !pre) cpu->reg((instruction >> 16) & 0xF).data.reg32 = total_address; // Writeback
}

void Debugger::arm_storeh(word instruction, Base::CPU *cpu) {
    bool pre = instruction & 0x01000000;
    bool up = instruction & 0x00800000;
    bool writeback = (instruction & 0x00200000) || !pre;
    bool sig = instruction & 0x40;
    bool half = instruction & 0x20;

    word offset = 0;

    if (instruction & 0x00400000) offset = (instruction & 0xF) | (((instruction >> 8) & 0xF) << 4);
    else offset = cpu->reg(instruction & 0xF).data.reg32;
    if(!up) offset *= -1;

    word address = cpu->reg((instruction >> 16) & 0xF).data.reg32;

    if(pre) address += offset;

    if(sig) {
        printf("Why is the sign extend bit set in storing operation? Makes no sense...\n");
    }

    if(half) {
        cpu->w16(address, cpu->reg((instruction >> 12) & 0xF).data.reg32 & 0xFFFF);
    } else {
        cpu->w8(address, cpu->reg((instruction >> 12) & 0xF).data.reg32 & 0xFF);
    }

    if(!pre || writeback) cpu->reg((instruction >> 16) & 0xF).data.reg32 += offset;
}

void Debugger::arm_loadh(word instruction, Base::CPU *cpu) {
    bool pre = instruction & 0x01000000;
    bool up = instruction & 0x00800000;
    bool writeback = (instruction & 0x00200000) || !pre;
    bool sig = instruction & 0x40;
    bool half = instruction & 0x20;

    word offset = 0;

    if (instruction & 0x00400000) offset = (instruction & 0xF) | (((instruction >> 8) & 0xF) << 4);
    else offset = cpu->reg(instruction & 0xF).data.reg32;


    if(!up) offset = ~offset + 1;

    word address = cpu->reg((instruction >> 16) & 0xF).data.reg32;
	if (address == 0x06000022) {
		printf("trying to read the thingy...\n");
	}

    if(pre) address += offset;

    if(half) {
        word r = (word) cpu->r16(address);
		if (address == 0x06000020) printf("Read the 1111, but got %.04X\n", (hword) r);
        if(sig && (r & 0x8000)) r |= 0xFFFF0000;
        cpu->reg((instruction >> 12) & 0xF).data.reg32 = r;
		if (address == 0x06000020) {
			printf("Read the 1111, but got %.04X in the register\n", (hword)cpu->reg((instruction >> 12) & 0xF).data.reg32);
		}
	} else {
        word r = (word) cpu->r8(address);
        if(sig && (r & 0x80)) r |= 0xFFFFFF00;
        cpu->reg((instruction >> 12) & 0xF).data.reg32 = r;
    }

    if(writeback) cpu->reg((instruction >> 16) & 0xF).data.reg32 += offset;
}
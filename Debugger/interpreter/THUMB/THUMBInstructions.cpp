#include "THUMBInstructions.h"

void Debugger::execute_thumb(hword instruction, Base::CPU *cpu) {
    cpu->update_cycles(1);
    if(instruction & 0x8000) {
        if(instruction & 0x4000) {
            if(instruction & 0x2000) {
                if(instruction & 0x1000) {
                    word off = ((instruction & 0x7FF) << 12);
                    if(instruction & 0x0400) off |= 0xFF800000;
                    cpu->reg(LR).data.reg32 = cpu->reg(PC).data.reg32 + 2 + off;
                    
                    hword p2 = cpu->r16(cpu->pc().data.reg32);
                    cpu->pc().data.reg32 += 2;
                    cpu->reg(LR).data.reg32 += (p2 & 0x7FF) << 1;

                    word addr = cpu->reg(LR).data.reg32;
                    cpu->reg(LR).data.reg32 = cpu->pc().data.reg32 | 1;
                    JUMP_TAKEN(cpu->pc().data.reg32, addr);
                    cpu->pc().data.reg32 = addr;
                } else {
                    word offset = (instruction & 0x7FF) << 1;
                    if(offset & 0x0800) offset |= 0xFFFFF000;
                    JUMP_TAKEN(cpu->pc().data.reg32, cpu->pc().data.reg32 + 2 + offset);
                    cpu->pc().data.reg32 += 2 + offset;
                }
            } else {
                if(instruction & 0x1000) {
                    if((instruction & 0x0F00) == 0x0F00) {
                        word old_cpsr = cpu->reg(CPSR).data.reg32;
                        cpu->set->setRegisterBank(MODE_SUPERVISOR);
                        cpu->reg(LR).data.reg32 = cpu->pc().data.reg32; // Return address				
                        cpu->pc().data.reg32 = 0x8;
                        cpu->reg(SPSR).data.reg32 = old_cpsr; // Clear T bit
                        cpu->reg(CPSR).data.reg32 &= 0xFFFFFF40 | MODE_SUPERVISOR;
                        cpu->reg(CPSR).data.reg32 |= FLAG_I;
                    } else {
						if(Base::isConditionMet((instruction >> 8) & 0xF, cpu->reg(CPSR).data.reg32)) {
							word offset = (instruction & 0xFF) << 1;
							if(offset & 0x100) offset |= 0xFFFFFE00;
                            JUMP_TAKEN(cpu->pc().data.reg32, cpu->pc().data.reg32 + 2 + offset);
							cpu->pc().data.reg32 += offset + 2;
						}
                    }
                } else {
                    word rb = (instruction >> 8) & 0x7;
                    int num = 0; for(int i = 0; i < 8; i++) if(instruction & (1 << i)) ++num;
                    word addr = cpu->reg(rb).data.reg32;
                    if(instruction & 0x0800) {
                        for(int i = 0; i < 8; i++) {
                            if(instruction & (1 << i)) {
                                cpu->reg(i).data.reg32 = cpu->r32(addr);
                                addr += 4;
                            }
                        }
                        cpu->reg(rb) += 4 * num;
                    } else {
                        for(int i = 0; i < 8; i++) {
                            if(instruction & (1 << i)) {
                                cpu->w32(addr, cpu->reg(i).data.reg32);
                                addr += 4;
                            }
                        }
                        cpu->reg(rb) += 4 * num;
                    }
                }
            }
        } else {
            if(instruction & 0x2000) {
                if(instruction & 0x1000) {
                    if(instruction & 0x0400) { // POP
                        int num = 0;
                        for(int i = 0; i < 9; i++) if(instruction & (1 << i)) ++num;
                        if(instruction & 0x0800) {
                            word start_address = cpu->reg(SP).data.reg32;
                            for(int i = 0; i < 8; i++) {
                                if(instruction & (1 << i)) {
                                    cpu->reg(i).data.reg32 = cpu->r32(start_address);
                                    start_address += 4;
                                }
                            }
                            if(instruction & 0x0100) cpu->reg(PC).data.reg32 = cpu->r32(start_address) & 0xFFFFFFFE;
                            cpu->reg(SP).data.reg32 += 4 * num;
                        } else { // PUSH
                            word start_address = cpu->reg(SP).data.reg32 - 4 * num;
                            for(int i = 0; i < 8; i++) {
                                if(instruction & (1 << i)) {
                                    cpu->w32(start_address, cpu->reg(i).data.reg32);
                                    start_address += 4;
                                }
                            }
                            if(instruction & 0x0100) cpu->w32(start_address, cpu->reg(LR).data.reg32);
                            cpu->reg(SP).data.reg32 -= 4 * num;
                        }
                    } else {
                        word offset = (instruction & 0x7F) << 2;
                        if(instruction & 0x0080) cpu->reg(SP).data.reg32 -= offset;
                        else cpu->reg(SP).data.reg32 += offset;
                    }
                } else {
                    word addr = instruction & 0x0800 ? (cpu->reg(SP).data.reg32) : ((cpu->pc().data.reg32 + 2) & 0xFFFFFFFC);
                    addr += (instruction & 0xFF) << 2;
                    cpu->reg((instruction >> 8) & 0x7).data.reg32 = addr;
                }
            } else {
                if(instruction & 0x1000) {
                    word addr = cpu->reg(SP).data.reg32 + ((instruction & 0xFF) << 2);
                    if(instruction & 0x0800) cpu->reg((instruction >> 8) & 0x7).data.reg32 = cpu->r32(addr);
                    else cpu->w32(addr, cpu->reg((instruction >> 8) & 0x7).data.reg32);
                } else {
                    word addr = cpu->reg((instruction >> 3) & 0x7).data.reg32;
                    addr += ((instruction >> 6) & 0x1F) << 1; // Offset
                    if(instruction & 0x0800) cpu->reg(instruction & 0x7).data.reg32 = cpu->r16(addr); // LDRH
                    else cpu->w16(addr, cpu->reg(instruction & 0x7).data.reg32 & 0xFFFF); // STRH
                }
            }
        }
    } else {
        if(instruction & 0x4000) {
            if((instruction & 0xFC00) == 0x4000) { // 467B = 010001100 1111 011
                thumb_alu(instruction, cpu);
            } else if((instruction & 0xFC00) == 0x4400) {
                word a1 = cpu->reg((instruction & 0x7) | ((instruction >> 4) & 0x8)).data.reg32;
                word a2 = cpu->reg((instruction >> 3) & 0xF).data.reg32;

                if(((instruction & 0x7) | ((instruction >> 4) & 0x8)) == 0xF) a1 += 2;
                if(((instruction >> 3) & 0xF) == 0xF) a2 += 2;

				switch((instruction >> 8) & 3) {
					case 0:
                        cpu->reg((instruction & 0x7) | ((instruction >> 4) & 0x8)).data.reg32 += a2;
						break;
					case 1:
                    {
                        word out = a1 - a2;
                        
                        if(out == 0) cpu->reg(CPSR).data.reg32 |= FLAG_Z;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_Z;

                        if(out & 0x80000000) cpu->reg(CPSR).data.reg32 |= FLAG_N;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_N;

                        if(a2 <= a1) cpu->reg(CPSR).data.reg32 |= FLAG_C;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
                        
                        if((a1 & 0x80000000) != (a2 & 0x80000000) && (a1 & 0x80000000) == (a2 & 0x80000000)) cpu->reg(CPSR).data.reg32 |= FLAG_V;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_V;

                    }
						break;
					case 2:
                        if(((instruction & 0x7) | ((instruction >> 4) & 0x8)) == 0xF) {
                            cpu->pc().data.reg32 = a2 & 0xFFFFFFFE;
                        } else {
                            cpu->reg((instruction & 0x7) | ((instruction >> 4) & 0x8)).data.reg32 = a2;
                        }
						break;
					case 3: // BX
					{
						word addr = a2;
                        JUMP_TAKEN(cpu->pc().data.reg32, addr);
						cpu->pc().data.reg32 = addr & 0xFFFFFFFE;
						if(!(addr & 1)) cpu->reg(CPSR) &= ~FLAG_T;
					}
						break;
				}
            } else if((instruction & 0xF800) == 0x4800) {
                word addr = (cpu->pc().data.reg32 + 2) & 0xFFFFFFFC;
                addr += (instruction & 0xFF) << 2;
                cpu->reg((instruction >> 8) & 0x7) = cpu->r32(addr); // PC relative load
            } else if(instruction & 0x2000) { // Load/Store with immidiate offset
                word addr = cpu->reg((instruction >> 3) & 0x7).data.reg32;
                word off = ((instruction >> 6) & 0x1F);
                if(instruction & 0x1000) {
                    if(instruction & 0x0800) cpu->reg(instruction & 0x7).data.reg32 = (word) cpu->r8(addr + off);
                    else cpu->w8(addr + off, cpu->reg(instruction & 0x7).data.reg32 & 0xFF);
                } else {
                    if(instruction & 0x0800) cpu->reg(instruction & 0x7).data.reg32 = cpu->r32(addr + (off << 2));
                    else cpu->w32(addr + (off << 2), cpu->reg(instruction & 0x7).data.reg32);
                }
            } else if(instruction & 0x0200) { // Load/Store with sign extended halfword
                word addr = cpu->reg((instruction >> 3) & 0x7).data.reg32 + cpu->reg((instruction >> 6) & 0x7).data.reg32;
                if(instruction & 0x0400) { // Sign extend
                    if(instruction & 0x0800) {
                        word res = cpu->r16(addr);
                        res |= ((res & 0x8000) ? 0xFFFF0000 : 0x00000000);
                        cpu->reg(instruction & 0x7).data.reg32 = res;
                        //printf("Loaded halfword from %.08X (=%.04X)\n", addr, res);
                    } else {
                        word res = cpu->r8(addr);
                        res |= ((res & 0x80) ? 0xFFFFFF00 : 0x00000000);
                        cpu->reg(instruction & 0x7).data.reg32 = res;
                    }
                } else { // Do not sign extend
                    if(instruction & 0x0800) cpu->reg(instruction & 0x7).data.reg32 = (word) cpu->r16(addr);
                    else cpu->w16(addr, cpu->reg(instruction & 0x7).data.reg32 & 0xFFFF);
                }
            } else { // Load store with register offset
                word addr = (cpu->reg((instruction >> 3) & 0x7).data.reg32 + cpu->reg((instruction >> 6) & 0x7).data.reg32);
                if(instruction & 0x0800) {
                    if(instruction & 0x0400) cpu->reg(instruction & 0x7).data.reg32 = (word) cpu->r8(addr);
                    else cpu->reg(instruction & 0x7).data.reg32 = cpu->r32(addr);
                } else {
					if(instruction & 0x0400) cpu->w8(addr, cpu->reg(instruction & 0x7).data.reg32 & 0xFF);
					else cpu->w32(addr, cpu->reg(instruction & 0x7).data.reg32);
                }
            }
        } else {
            if(instruction & 0x2000) thumb_data_processing(instruction, cpu);
			else {
                word res = cpu->reg((instruction >> 3) & 0x7).data.reg32;
                word offset = (instruction >> 6) & 0x1F;
				switch((instruction >> 11) & 3) {
					case 0:
                    {
                        word out = res << offset;
                        if((res >> (32 - offset)) & 1) cpu->reg(CPSR).data.reg32 |= FLAG_C;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
                        if(out == 0) cpu->reg(CPSR).data.reg32 |= FLAG_Z;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_Z;
                        if(out & 0x80000000) cpu->reg(CPSR).data.reg32 |= FLAG_N;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_N;
                        cpu->reg(instruction & 0x7).data.reg32 = out;
                    }
						break;
					case 1:
                    {
                        word out = res >> offset;
                        if((res >> (offset - 1)) & 1) cpu->reg(CPSR).data.reg32 |= FLAG_C;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
                        if(out == 0) cpu->reg(CPSR).data.reg32 |= FLAG_Z;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_Z;
                        if(out & 0x80000000) cpu->reg(CPSR).data.reg32 |= FLAG_N;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_N;
                        cpu->reg(instruction & 0x7).data.reg32 = out;
                    }
						break;
					case 2:
                    {
                        word out = (res >> offset);
                        if(res & 0x80000000) out |= (0xFFFFFFFF << (32 - offset));
                        if((res >> (offset - 1)) & 1) cpu->reg(CPSR).data.reg32 |= FLAG_C;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
                        if(out == 0) cpu->reg(CPSR).data.reg32 |= FLAG_Z;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_Z;
                        if(out & 0x80000000) cpu->reg(CPSR).data.reg32 |= FLAG_N;
                        else cpu->reg(CPSR).data.reg32 &= ~FLAG_N;
                        cpu->reg(instruction & 0x7).data.reg32 = out;
                    }
						break;
					case 3:
					{
						word a1 = cpu->reg((instruction >> 3) & 0x7).data.reg32;
						word a2 = instruction & 0x0400 ? ((instruction >> 6) & 0x7) : cpu->reg(((instruction >> 6) & 0x7)).data.reg32;
						if(instruction & 0x0200) { // SUB
							cpu->reg(instruction & 0x7).data.reg32 = a1 - a2;
							
							if(a2 <= a1) cpu->reg(CPSR).data.reg32 |= FLAG_C;
							else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
							
							if((a2 & 0x80000000) != (a1 & 0x80000000) && (a1 & 0x80000000) == ((a1-a2) & 0x80000000)) cpu->reg(CPSR).data.reg32 |= FLAG_V;
							else cpu->reg(CPSR).data.reg32 &= ~FLAG_V;
						} else { // ADD
							cpu->reg(instruction & 0x7).data.reg32 = a1 + a2;
							
							if((uint64_t) a2 + (uint64_t) a1 >= 0x100000000) cpu->reg(CPSR).data.reg32 |= FLAG_C;
							else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
							
							if((a2 & 0x80000000) == (a1 & 0x80000000) && (a1 & 0x80000000) != ((a1+a2) & 0x80000000)) cpu->reg(CPSR).data.reg32 |= FLAG_V;
							else cpu->reg(CPSR).data.reg32 &= ~FLAG_V;
						}
						
						if(!cpu->reg(instruction & 0x7).data.reg32) cpu->reg(CPSR).data.reg32 |= FLAG_Z;
						else cpu->reg(CPSR).data.reg32 &= ~FLAG_Z;
						
						if(cpu->reg(instruction & 0x7).data.reg32 & 0x80000000) cpu->reg(CPSR).data.reg32 |= FLAG_N;
						else cpu->reg(CPSR).data.reg32 &= ~FLAG_N;
					}
						break;
				}
            }
        }
    }
}

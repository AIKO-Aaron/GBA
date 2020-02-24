#include "ARMInstructions.h"

void Debugger::execute_arm(word instruction, Base::CPU *cpu) {
    if(!Base::isConditionMet((instruction >> 28) & 0xF, cpu->reg(CPSR).data.reg32)) {
        //printf("Condition not met...\n");
        return;
    }
    
    cpu->update_cycles(1);

    if (instruction & 0x08000000) {
        if (instruction & 0x04000000) {
            if ((instruction & 0x0F000000) == 0x0F000000) {
                word old_cpsr = cpu->reg(CPSR).data.reg32;
				cpu->set->setRegisterBank(MODE_SUPERVISOR);
				cpu->reg(LR).data.reg32 = cpu->pc().data.reg32; // Return address				
				cpu->pc().data.reg32 = 0x8;
				cpu->reg(SPSR).data.reg32 = old_cpsr; // Clear T bit
				cpu->reg(CPSR).data.reg32 &= 0xFFFFFF40 | MODE_SUPERVISOR;
                cpu->reg(CPSR).data.reg32 |= FLAG_I;
                //printf("Software interrupt (ARM) %.08X\n", instruction);
            }
            else {
                if (instruction & 0x02000000) {
                    if (instruction & 0x00000010) {
                        printf("CoProcessor Register transfer: %.08X\n", instruction);

                        exit(0);
                    }
                    else {
                        printf("CoProcessor Data operation: %.08X\n", instruction);
                        exit(0);
                    }
                }
                else {
                    printf("CoProcessor Data transfer: %.08X\n", instruction);
                    exit(0);
                }
            }
        }
        else {
            if (instruction & 0x02000000) arm_branch(instruction, cpu);
            else {
                word a = (instruction & 0x01800000);
                word start_address = cpu->reg((instruction >> 16) & 0xF).data.reg32;
                byte amount_regs = 0;
                for(int i = 0; i < 16; i++) if(instruction & (1 << i)) ++amount_regs;
                
                if(a == 0x01800000) start_address += 4;
                else if(a == 0x00800000) start_address += 0;
                else if(a == 0x01000000) start_address -= amount_regs * 4;
                else if(a == 0x00000000) start_address += amount_regs * 4 - 4;

                if(instruction & 0x00200000) {
                    if(instruction & 0x00800000) cpu->reg((instruction >> 16) & 0xF).data.reg32 += 4 * amount_regs;
                    else cpu->reg((instruction >> 16) & 0xF).data.reg32 -= 4 * amount_regs;
                }
                
                if(instruction & 0x00100000) {
                    for(int i = 0; i < 16; i++) if(instruction & (1 << i)) {
                        cpu->reg(i).data.reg32 = cpu->r32(start_address);
                        start_address += 4;
                    }
                } else {
                    for(int i = 0; i < 16; i++) if(instruction & (1 << i)) {
                        cpu->w32(start_address, cpu->reg(i).data.reg32 + ((i == 15) ? 8 : 0));
                        start_address += 4;
                    }
                }
            }
        }
    }
    else {
        if (instruction & 0x04000000) {
            if ((instruction & 0x02000010) == 0x02000010) {
                printf("Undefined\n");
                exit(0);
            }
            else {
                if(instruction & (1 << 20)) arm_load(instruction, cpu);
                else arm_store(instruction, cpu);
            }
        }
        else {
            if ((instruction & 0x020000F0) == 0x00000090) {
                if (instruction & 0x01000000) {
                    printf("Single Data Swap\n");
                    exit(0);
                }
                else if (instruction & 0x00800000) { // MUL
                    bool sign = instruction & (1 << 22);
                    bool acc = instruction & (1 << 21);
                    bool s = instruction & (1 << 20);

                    if(sign) {
                        int64_t old_values = ((uint64_t) cpu->reg((instruction >> 16) & 0xF).data.reg32 << 32) | (uint64_t) cpu->reg((instruction >> 12) & 0xF).data.reg32;

                        word op1 = cpu->reg((instruction >> 8) & 0xF).data.reg32;
                        word op2 = cpu->reg((instruction) & 0xF).data.reg32;
                        int64_t result = (int64_t) op1 * (int64_t) op2;

                        if(acc) result += old_values;

                        cpu->reg((instruction >> 12) & 0xF).data.reg32 = result & 0xFFFFFFFF;
                        cpu->reg((instruction >> 16) & 0xF).data.reg32 = (result >> 32);

                        if(s) {
                            if(result == 0) cpu->reg(CPSR) |= FLAG_Z;
                            else cpu->reg(CPSR) &= ~FLAG_Z;
                            
                            if(result & 0x80000000) cpu->reg(CPSR) |= FLAG_N;
                            else cpu->reg(CPSR) &= ~FLAG_N;
                        }
                    } else {
                        uint64_t old_values = ((uint64_t) cpu->reg((instruction >> 16) & 0xF).data.reg32 << 32) | (uint64_t) cpu->reg((instruction >> 12) & 0xF).data.reg32;

                        word op1 = cpu->reg((instruction >> 8) & 0xF).data.reg32;
                        word op2 = cpu->reg((instruction) & 0xF).data.reg32;
                        uint64_t result = (uint64_t) op1 * (uint64_t) op2;

                        if(acc) result += old_values;

                        cpu->reg((instruction >> 12) & 0xF).data.reg32 = result & 0xFFFFFFFF;
                        cpu->reg((instruction >> 16) & 0xF).data.reg32 = (result >> 32);

                        if(s) {
                            if(result == 0) cpu->reg(CPSR) |= FLAG_Z;
                            else cpu->reg(CPSR) &= ~FLAG_Z;
                            
                            if(result & 0x80000000) cpu->reg(CPSR) |= FLAG_N;
                            else cpu->reg(CPSR) &= ~FLAG_N;
                        }
                    }
                }
                else {
                    bool accumulate = instruction & (1 << 21);
                    word op1 = cpu->reg(instruction & 0xF).data.reg32;
                    word op2 = cpu->reg((instruction >> 8) & 0xF).data.reg32;
                    word acc = cpu->reg((instruction >> 12) & 0xF).data.reg32;
                    bool s = instruction & (1 << 20);
                    
                    word result = op1 * op2;
                    if(accumulate) result += acc;
                    
                    if(s) {
                        if(result == 0) cpu->reg(CPSR) |= FLAG_Z;
                        else cpu->reg(CPSR) &= ~FLAG_Z;
                        
                        if(result & 0x80000000) cpu->reg(CPSR) |= FLAG_N;
                        else cpu->reg(CPSR) &= ~FLAG_N;
                    }
                    
                    cpu->reg((instruction >> 16) & 0xF).data.reg32 = result;
                }
            }
            else if ((instruction & 0x020000B0) == 0x000000B0) {
                bool load = instruction & 0x00100000;
                if(load) arm_loadh(instruction, cpu);
                else arm_storeh(instruction, cpu);
            }
            else if ((instruction & 0x0FFFFFF0) == 0x012FFF10) {
                word target_addr = cpu->reg(instruction & 0xF).data.reg32;
                JUMP_TAKEN(cpu->pc().data.reg32, target_addr);
                cpu->pc().data.reg32 = target_addr & 0xFFFFFFFE;
                if(target_addr & 1) {
                    cpu->reg(CPSR) |= FLAG_T;
                }
            } else if((instruction & 0x0F800000) == 0x01000000 && (instruction & 0x003F0000) == 0x000F0000 && (instruction & 0x00000FFF) == 0x00000000) {
                cpu->reg((instruction >> 12) & 0xF).data.reg32 = cpu->reg(instruction & (1 << 22) ? SPSR : CPSR).data.reg32;
            } else if((instruction & 0x0F800000) == 0x01000000 && (instruction & 0x003FFFF0) == 0x0029F000) {
                if(instruction & (1 << 22)) cpu->reg(SPSR).data.reg32 = cpu->reg(instruction & 0xF).data.reg32;
                else {
                    cpu->reg(CPSR).data.reg32 = cpu->reg(instruction & 0xF).data.reg32;
                    cpu->set->setRegisterBank(cpu->reg(CPSR).data.reg32 & 0x1F);
                }
            } else if((instruction & 0x0D800000) == 0x01000000 && (instruction & 0x003FF000) == 0x0028F000) {
                printf("MSR PSR only\n");
                exit(0);
            }
            else arm_data_processing(instruction, cpu);
        }
    }
}

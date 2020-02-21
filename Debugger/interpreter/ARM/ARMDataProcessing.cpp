#include "ARMInstructions.h"

enum CHANGE_FLAG { NO_CHANGE, SET, RESET };

void Debugger::arm_data_processing(word instruction, Base::CPU *cpu) {
    bool imm = instruction & 0x02000000;
    bool s = instruction & 0x00100000;
    byte opcode = (instruction >> 21) & 0xF;

    bool was_n_set = cpu->reg(CPSR) & FLAG_N;
    bool was_z_set = cpu->reg(CPSR) & FLAG_Z;
    bool was_c_set = cpu->reg(CPSR) & FLAG_C;
    bool was_v_set = cpu->reg(CPSR) & FLAG_V;
    
    Base::Register &dest = cpu->reg((instruction >> 12) & 0xF);
    word arg1 = cpu->reg((instruction >> 16) & 0xF).data.reg32;
    if(((instruction >> 16) & 0xF) == 0xF) arg1 += 4;
    word arg2 = instruction & 0xFFF;
    
    if(imm) arg2 = Base::rotate(instruction & 0xFF, (instruction >> 8) & 0xF, cpu, s);
    else {
        word reg = cpu->reg(instruction & 0xF).data.reg32;
        arg2 = Base::shift((instruction >> 4) & 0xFF, reg, cpu, s);
    }
    
    CHANGE_FLAG set_n = NO_CHANGE;
    CHANGE_FLAG set_z = NO_CHANGE;
    CHANGE_FLAG set_c = NO_CHANGE;
    CHANGE_FLAG set_v = NO_CHANGE;
    
    switch (opcode) {
        case 0x0: // AND
            {
                dest.data.reg32 = arg1 & arg2;
                set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
                set_z = (dest.data.reg32 == 0) ? SET : RESET;
            }
            break;
        case 0x1: // EOR
            {
                dest.data.reg32 = arg1 ^ arg2;
                set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
                set_z = (dest.data.reg32 == 0) ? SET : RESET;
            }
            break;
        case 0x2: // SUB
            {
                word result = arg1 - arg2;

                if(arg1 >= arg2) set_c = SET; //cpu->reg(CPSR) |= FLAG_C;
                else set_c = RESET; //cpu->reg(CPSR) &= ~FLAG_C;
                
                if((result & 0x80000000) == (arg2 & 0x80000000) && (result & 0x80000000) != (arg1 & 0x80000000)) set_v = SET; //cpu->reg(CPSR) |= FLAG_V;
                else set_v = RESET; //cpu->reg(CPSR) &= ~FLAG_V;     


                if(((instruction >> 12) & 0xF) == 0xF && s) {
                    // printf("Returning from address %.08X (CPSR: %.08X, SPSR: %.08X)\n", cpu->pc().data.reg32, cpu->reg(CPSR).data.reg32, cpu->reg(SPSR).data.reg32);
                    dest.data.reg32 = result;
                    cpu->reg(CPSR) = cpu->reg(SPSR);
                    cpu->set->setRegisterBank(cpu->reg(CPSR).data.reg32 & 0x1F);
                    return;
                } else {
                    dest.data.reg32 = result;
                    set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
                    set_z = (dest.data.reg32 == 0) ? SET : RESET;           
                }
            }
            break;
        case 0x3: // RSB
            {
                word result = arg2 - arg1;

                if(arg2 >= arg1) set_c = SET;
                else set_c = RESET;
                
                if((result & 0x80000000) == (arg1 & 0x80000000) && (result & 0x80000000) != (arg2 & 0x80000000)) set_v = SET;
                else set_v = RESET;     

                dest.data.reg32 = result;
                set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
                set_z = (dest.data.reg32 == 0) ? SET : RESET;           
            }
            break;
        case 0x4: // ADD
        {
            word out = arg1 + arg2;
            
            set_n = out & 0x80000000 ? SET : RESET;
            set_z = out == 0 ? SET : RESET;
            set_c = ((uint64_t) arg1 + (uint64_t) arg2) > (uint64_t) 0xFFFFFFFF ? SET : RESET;

            set_v = (arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg1 & 0x80000000) != (out & 0x80000000) ? SET : RESET;
            
            dest.data.reg32 = out;
        }
            break;
        case 0x5: // ADC
        {
            word to_add = 0;
            if(was_c_set) ++to_add;
            word out = arg1 + arg2 + to_add;
            set_c = ((uint64_t) arg1 + (uint64_t) arg2 + (uint64_t) to_add) > (uint64_t) 0xFFFFFFFF ? SET : RESET;
            set_v = (arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg1 & 0x80000000) != (out & 0x80000000) ? SET : RESET;
            
            dest.data.reg32 = out;
            set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
            set_z = (dest.data.reg32 == 0) ? SET : RESET;
        }
            break;
        case 0x6: // SBC
        {
            word to_sub = 1;
            if(was_c_set) --to_sub;
            word out = arg1 - arg2 - to_sub;
            
            if(arg1 >= arg2 + to_sub) set_c = SET; //cpu->reg(CPSR) |= FLAG_C;
            else set_c = RESET; //cpu->reg(CPSR) &= ~FLAG_C;
                
            if((out & 0x80000000) == (arg2 & 0x80000000) && (out & 0x80000000) != (arg1 & 0x80000000)) set_v = SET; //cpu->reg(CPSR) |= FLAG_V;
            else set_v = RESET; //cpu->reg(CPSR) &= ~FLAG_V;

            dest.data.reg32 = out;
            set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
            set_z = (dest.data.reg32 == 0) ? SET : RESET;
        }
            break;
        case 0x7: // RSC
        {
            word to_sub = 1;
            if(was_c_set) --to_sub;
            word out = arg2 - arg1 - to_sub;
            
            if(arg2 >= arg1 + to_sub) set_c = SET; //cpu->reg(CPSR) |= FLAG_C;
            else set_c = RESET; //cpu->reg(CPSR) &= ~FLAG_C;
                
            if((out & 0x80000000) == (arg2 & 0x80000000) && (out & 0x80000000) != (arg1 & 0x80000000)) set_v = SET; //cpu->reg(CPSR) |= FLAG_V;
            else set_v = RESET; //cpu->reg(CPSR) &= ~FLAG_V;

            dest.data.reg32 = out;
            set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
            set_z = (dest.data.reg32 == 0) ? SET : RESET;
        }
            break;
        case 0x8: // TST
        {
            word out = arg1 & arg2;
            set_n = (out & 0x80000000) ? SET : RESET;
            set_z = (out == 0) ? SET : RESET;
        }
            break;
        case 0x9: // EOR --> test
        {
            word out = arg1 ^ arg2;
            set_n = (out & 0x80000000) ? SET : RESET;
            set_z = (out == 0) ? SET : RESET;
        }
            break;
        case 0xA: // CMP
            {
                word out = arg1 - arg2;
                set_n = (out & 0x80000000) ? SET : RESET;
                set_z = (out == 0) ? SET : RESET;
                set_c = (arg2 <= arg1) ? SET : RESET;
                set_v = ((out & 0x80000000) == (arg2 & 0x80000000) && (out & 0x80000000) != (arg1 & 0x80000000)) ? SET : RESET;
            }
            break;
        case 0xB: // Compare with add CMN
        {
            word out = arg1 + arg2;
            
            set_n = out & 0x80000000 ? SET : RESET;
            set_z = out == 0 ? SET : RESET;
            set_c = ((uint64_t) arg1 + (uint64_t) arg2) > 0xFFFFFFFF ? SET : RESET;
            set_v = (arg1 & 0x80000000) == (arg2 & 0x80000000) && (arg1 & 0x80000000) != (out & 0x80000000) ? SET : RESET;            
        }
            break;
        case 0xC:
            dest.data.reg32 = arg1 | arg2;
            set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
            set_z = (dest.data.reg32 == 0) ? SET : RESET;
            break;
        case 0xD: // MOV
            dest.data.reg32 = arg2;
            set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
            set_z = (dest.data.reg32 == 0) ? SET : RESET;
            
            if(((instruction >> 12) & 0xF) == 0xF && (instruction & 0xFFF) == 0xE) {
                // Also restore the CPSR, as we probably returned from SWI...
                dest.data.reg32 &= 0xFFFFFFFE;
                cpu->reg(CPSR) = cpu->reg(SPSR);
                cpu->set->setRegisterBank(cpu->reg(CPSR).data.reg32 & 0x1F);
            }
            break;
        case 0xE:
            dest.data.reg32 = arg1 & ~arg2;
            set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
            set_z = (dest.data.reg32 == 0) ? SET : RESET;
            break;
        case 0xF: // MVN
            dest.data.reg32 = ~arg2;
            set_n = (dest.data.reg32 & 0x80000000) ? SET : RESET;
            set_z = (dest.data.reg32 == 0) ? SET : RESET;
            break;
    }

    if(s) {        
        if(set_n == SET) cpu->reg(CPSR) |= FLAG_N;
        else if(set_n == RESET) cpu->reg(CPSR) &= ~FLAG_N;
        
        if(set_z == SET) cpu->reg(CPSR) |= FLAG_Z;
        else if(set_z == RESET) cpu->reg(CPSR) &= ~FLAG_Z;
        
        if(set_c == SET) cpu->reg(CPSR) |= FLAG_C;
        else if(set_c == RESET) cpu->reg(CPSR) &= ~FLAG_C;
        
        if(set_v == SET) cpu->reg(CPSR) |= FLAG_V;
        else if(set_v == RESET) cpu->reg(CPSR) &= ~FLAG_V;
    }
}

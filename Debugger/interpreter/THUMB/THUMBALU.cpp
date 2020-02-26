#include "THUMBInstructions.h"

void Debugger::thumb_alu(hword instruction, Base::CPU *cpu) {
    word res = 0;
    
    /**
    word op1 = cpu->reg((instruction >> 3) & 0x7).data.reg32;
    word op2 = cpu->reg(instruction & 0x7).data.reg32;
    */

    word op1 = cpu->reg(instruction & 0x7).data.reg32;
    word op2 = cpu->reg((instruction >> 3) & 0x7).data.reg32;
    
    bool write = true;
    
    switch ((instruction >> 6) & 0xF) {
        case 0x0: // AND
            res = op1 & op2;
            break;
        case 0x1: // EOR
            res = op1 ^ op2;
            break;
        case 0x2: // LSL
            op2 &= 0xFF;
            res = op1 << op2;
            if((op1 >> (32 - op2)) & 1) cpu->reg(CPSR).data.reg32 |= FLAG_C;
            else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
            break;
        case 0x3: // LSR
            op2 &= 0xFF;
            res = op1 >> op2;
            if((op1 >> (op2 - 1)) & 1) cpu->reg(CPSR).data.reg32 |= FLAG_C;
            else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
            break;
        case 0x4: // ASR
            op2 &= 0xFF;
            res = (op1 >> op2) | ((op1 & 0x80000000) ? (0xFFFFFFFF << (32 - op2)) : 0);
            if((op1 >> (op2 - 1)) & 1) cpu->reg(CPSR).data.reg32 |= FLAG_C;
            else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
            break;
        case 0x5: // ADC
            printf("ADC ");
            exit(0);
            break;
        case 0x6: // SBC
            printf("SBC ");
            exit(0);
            break;
        case 0x7: // ROR
            op2 &= 0xFF;
            res = (op1 >> op2) | (op1 << (32 - op2));
            if((op1 >> (op2 - 1)) & 1) cpu->reg(CPSR).data.reg32 |= FLAG_C;
            else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
            break;
        case 0x8: // TST
            {
                res = op1 & op2;
                write = false;
            }
            break;
        case 0x9: // NEG
            res = -op2;
            if(op2 == 0) {
                cpu->reg(CPSR).data.reg32 |= FLAG_C;
                cpu->reg(CPSR).data.reg32 |= FLAG_V;
            } else {
                cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
                cpu->reg(CPSR).data.reg32 &= ~FLAG_V;
            }
            break;
        case 0xA: // CMP
            {
                res = op1 - op2;

                if(op2 <= op1) cpu->reg(CPSR) |= FLAG_C;
                else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
                
                if((res & 0x80000000) == (op2 & 0x80000000) && (res & 0x80000000) != (op1 & 0x80000000)) cpu->reg(CPSR) |= FLAG_V;
                else cpu->reg(CPSR).data.reg32 &= ~FLAG_V;

                write = false;                
            }
            break;
        case 0xB: // CMN
            {
                res = op1 + op2;
                
                if(((uint64_t) op1 + (uint64_t) op2) > 0xFFFFFFFF) cpu->reg(CPSR) |= FLAG_C;
                else cpu->reg(CPSR) &= ~FLAG_C;
                
                if((op1 & 0x80000000) == (op2 & 0x80000000) && (op1 & 0x80000000) != (res & 0x80000000)) cpu->reg(CPSR).data.reg32 |= FLAG_V;
                else cpu->reg(CPSR).data.reg32 &= ~FLAG_V;
                
                write = false;
            }
            break;
        case 0xC: // ORR
            {
                res = op1 | op2;
            }
            break;
        case 0xD: // MUL
            res = op1 * op2;
            break;
        case 0xE: // BIC
            res = op1 & ~op2;
            break;
        case 0xF: // MVN
            res = ~op2;
            break;
        default:
            break;
    }
    
    if(res == 0) cpu->reg(CPSR).data.reg32 |= FLAG_Z;
    else cpu->reg(CPSR).data.reg32 &= ~FLAG_Z;
    
    if(res & 0x80000000) cpu->reg(CPSR).data.reg32 |= FLAG_N;
    else cpu->reg(CPSR).data.reg32 &= ~FLAG_N;
    
    if(write) cpu->reg(instruction & 0x7).data.reg32 = res;
}

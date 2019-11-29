#include "ARMInstructions.h"

void Debugger::arm_branch(word instruction, Base::CPU *cpu) {
    word addr = (instruction & 0x00FFFFFF) << 2;
    JUMP_TAKEN(cpu->pc().data.reg32, addr);
    if(addr & 0x02000000) addr |= 0xFC000000;
    addr += cpu->pc().data.reg32 + 4;

    if(instruction & 0x01000000) cpu->reg(LR) = cpu->pc();
    cpu->pc() = addr;

}

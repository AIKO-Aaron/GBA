#include "Debugger.h"

Debugger::Debugger::Debugger() {
    cpu = new Base::CPU();
}

Debugger::Debugger::Debugger(Base::CPU* _cpu) : cpu(_cpu) {

}

Debugger::Debugger::~Debugger() {
    delete cpu;
}

word Debugger::Debugger::fetchNextInstruction() {
    word ni = cpu->r32(cpu->pc());
    cpu->pc() += cpu->reg(CPSR) & FLAG_T ? 2 : 4;
    return ni;
}

void Debugger::Debugger::printRegisters() {
    for(int i = 0; i < 6; i++) {
        printf("[DEBUGGER] ");
        for(int j = 0; j < 3; j++) printf("%4s = %.08X, ", Decompiler::reg_names[i * 3 + j], cpu->reg(i * 3 + j).data.reg32);
        printf("\n");
    }
}

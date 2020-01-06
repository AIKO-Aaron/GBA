#include "Interpreter.h"

#define BACKTRACE_SIZE 30

Debugger::Interpreter::Interpreter() : Debugger::Debugger() {
	for(int i = 0; i < BACKTRACE_SIZE; i++) instruction_trace.push_back({});
}

Debugger::Interpreter::Interpreter(Base::CPU *_cpu) : Debugger::Debugger(_cpu) {
    for (int i = 0; i < BACKTRACE_SIZE; i++) instruction_trace.push_back({});
}

Debugger::Interpreter::~Interpreter() {
    
}

void Debugger::Interpreter::executeNextInstruction(bool disass) {
    cpu->update_cycles(1); // All instructions increase the cycle by at least one

    // printf("Cpu cycle count: %.08X\n", cpu->cycle_count);
    

    if(!cpu->mmu->halted) {
        word ni = fetchNextInstruction();
        //for (int i = BACKTRACE_SIZE - 1; i > 0; i--) instruction_trace[i] = instruction_trace[i - 1];
        //instruction_trace[0] = { pc().data.reg32, ni & (cpu->reg(CPSR).data.reg32 & FLAG_T ? 0xFFFF : 0xFFFFFFFF), cpu->reg(CPSR).data.reg32 & FLAG_T ? Decompiler::decompileTHUMB(ni & 0xFFFF, cpu, false) : Decompiler::decompileARM(ni, cpu, false), cpu->reg(R1).data.reg32 };
        
        /*if (pc().data.reg32 == 0x08007804) {
            printf("Reached this point with %.08X in R1 and %.08X in R2\n", cpu->reg(R1).data.reg32, cpu->reg(R2).data.reg32);
        }*/

        if(cpu->reg(CPSR).data.reg32 & FLAG_T) {
            if(disass) std::string out = Decompiler::decompileTHUMB(ni & 0xFFFF, cpu);
            execute_thumb(ni & 0xFFFF, cpu);
        } else {
            if(disass) std::string out = Decompiler::decompileARM(ni, cpu);
            execute_arm(ni, cpu);
        }
    }
    
    cpu->update_gpu();
}

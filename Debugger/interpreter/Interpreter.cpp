#include "Interpreter.h"

#define BACKTRACE_SIZE 15

Debugger::Interpreter::Interpreter() : Debugger::Debugger() {
	for(int i = 0; i < BACKTRACE_SIZE; i++) instruction_trace.push_back({});
}

Debugger::Interpreter::Interpreter(Base::CPU *_cpu) : Debugger::Debugger(_cpu) {
    for (int i = 0; i < BACKTRACE_SIZE; i++) instruction_trace.push_back({});
}

Debugger::Interpreter::~Interpreter() {
    
}

void Debugger::Interpreter::executeNextInstruction(bool disass) {
    cpu->update_cycles(2); // All instructions increase the cycle by at least one

    // printf("Cpu cycle count: %.08X\n", cpu->cycle_count);
    

    if(!cpu->mmu->halted) {
        word ni = fetchNextInstruction();
        //for (int i = BACKTRACE_SIZE - 1; i > 0; i--) instruction_trace[i] = instruction_trace[i - 1];
        //instruction_trace[0] = { pc().data.reg32, ni & (cpu->reg(CPSR).data.reg32 & FLAG_T ? 0xFFFF : 0xFFFFFFFF), cpu->reg(CPSR).data.reg32 & FLAG_T ? Decompiler::decompileTHUMB(ni & 0xFFFF, cpu, false) : Decompiler::decompileARM(ni, cpu, false), cpu->reg(R0).data.reg32 };
        
        if(cpu->reg(R4).data.reg32 == 0x02021730 && cpu->r8(0x0202175B) == 0x01) {
            printf("That was not there before: %.08X\n", cpu->pc().data.reg32);            
            cpu->w8(0x0202175B, 0);
        }


        if(pc().data.reg32 == 0x0807643C) {
            printf("We're in this other function doing birch stuff... (CreatePokeballSpriteToReleaseMon)\n");
        }

        if(pc().data.reg32 == 0x080078C6) {
            word sprite_obj_addr = cpu->reg(R0).data.reg32;
            word anims = cpu->r32(sprite_obj_addr + 8);


            if(anims == 0x082EC69C) {
                disassemble = 100;
                printf("Reached the place...\n");
                printf("Sprite state object (r0 --> r0+0x30) @ %.08X:\n00\t\t", cpu->reg(R0).data.reg32);
                for(int i = 0; i < 16 * 3 + 15; i++) {
                    printf("%.02X ", cpu->r8(cpu->reg(R0).data.reg32 + i));
                    if(i % 16 == 15) printf("\n%.02X\t\t", i / 16 + 1);
                }
                printf("\n");
            }
        }

        /*if (pc().data.reg32 == 0x08007804) {
            printf("Reached this point with %.08X in R1 and %.08X in R2\n", cpu->reg(R1).data.reg32, cpu->reg(R2).data.reg32);
        }*/

        if(disassemble) {
            --disassemble;
            for(int i = 0; i < 6; i++) {
                printf("[DECOMP] ");
                for(int j = 0; j < 3; j++) printf("%4s = %.08X, ", Decompiler::reg_names[i * 3 + j], cpu->reg(i * 3 + j).data.reg32);
                printf("\n");
            }
        }

        cpu->mmu->addr=cpu->pc().data.reg32;

        if(cpu->reg(CPSR).data.reg32 & FLAG_T) {
            if(disass || disassemble) std::string out = Decompiler::decompileTHUMB(ni & 0xFFFF, cpu);
            execute_thumb(ni & 0xFFFF, cpu);
        } else {
            if(disass || disassemble) std::string out = Decompiler::decompileARM(ni, cpu);
            execute_arm(ni, cpu);
        }
    }
    
    cpu->update_gpu();
}

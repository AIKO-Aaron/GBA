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

bool done = false;
bool a = false;
int ctr = 0;
void Debugger::Interpreter::executeNextInstruction(bool disass) {
    cpu->update_cycles(1); // All instructions increase the cycle by at least one

    // printf("Cpu cycle count: %.08X\n", cpu->cycle_count);
    

    if(!cpu->mmu->halted) {
        word ni = fetchNextInstruction();
        //for (int i = BACKTRACE_SIZE - 1; i > 0; i--) instruction_trace[i] = instruction_trace[i - 1];
        //instruction_trace[0] = { pc().data.reg32, ni & (cpu->reg(CPSR).data.reg32 & FLAG_T ? 0xFFFF : 0xFFFFFFFF), cpu->reg(CPSR).data.reg32 & FLAG_T ? Decompiler::decompileTHUMB(ni & 0xFFFF, cpu, false) : Decompiler::decompileARM(ni, cpu, false), cpu->reg(R0).data.reg32 };

        if(pc().data.reg32 == 0x0808af16) {
            printf("Collision variable is: %.08X\n", cpu->reg(R4).data.reg32);
            cpu->reg(R4).data.reg32 = 0; // Walk through walls
        }

        if(pc().data.reg32 == 0x080888ac) {
            

            if(cpu->reg(R10).data.reg32) {
                if(ctr++ % 16) {
                    cpu->reg(R10).data.reg32 = 0;
                } else {
                    ctr %= 16;
                    cpu->reg(R9).data.reg32 = 0;
                } 
            }

            word saveptr = 0x03005d8c;
            word saveblock = cpu->r32(saveptr);
            printf("[1] Pos: %.04X, %.04X\n", cpu->r16(saveblock), cpu->r16(saveblock + 2));
            printf("[2] Adding %.08X %.08X to coordinates...\n", cpu->reg(R10).data.reg32, cpu->reg(R9).data.reg32);
            //a = true;
        }

        if(pc().data.reg32 == 0x0808a112) {
            word cameraobjptr = cpu->reg(R3).data.reg32;
            word spriteid = cpu->r32(cameraobjptr + 0x04);
            word sprite_ptr = 0x02020662;
            word our_sprite = sprite_ptr + 0x44 * spriteid;
            // printf("Sprite @ %.08X { data[2] = %.02X data[3] = %.02X}\n", our_sprite, cpu->r16(our_sprite), cpu->r16(our_sprite + 2));
            // cpu->w16(our_sprite + 0, 0);
            // cpu->w16(our_sprite + 2, 0);
            // cpu->mmu->game_state_obj_addr = our_sprite;
        }

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

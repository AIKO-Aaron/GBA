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
void Debugger::Interpreter::executeNextInstruction(bool disass) {
    cpu->update_cycles(1); // All instructions increase the cycle by at least one

    // printf("Cpu cycle count: %.08X\n", cpu->cycle_count);
    

    if(!cpu->mmu->halted) {
        word ni = fetchNextInstruction();
        //for (int i = BACKTRACE_SIZE - 1; i > 0; i--) instruction_trace[i] = instruction_trace[i - 1];
        //instruction_trace[0] = { pc().data.reg32, ni & (cpu->reg(CPSR).data.reg32 & FLAG_T ? 0xFFFF : 0xFFFFFFFF), cpu->reg(CPSR).data.reg32 & FLAG_T ? Decompiler::decompileTHUMB(ni & 0xFFFF, cpu, false) : Decompiler::decompileARM(ni, cpu, false), cpu->reg(R0).data.reg32 };

        /*if(pc().data.reg32 == 0x0808af16) {
            printf("Collision variable is: %.08X\n", cpu->reg(R4).data.reg32);
            // cpu->reg(R4).data.reg32 = 0; // Walk through walls
        }

        if(pc().data.reg32 == 0x08092bcc) {
            printf("Called GetCollisionAtCoords with coordinates: %.04X, %.04X\n", cpu->reg(R1).data.reg32, cpu->reg(R2).data.reg32);
        }

        if(pc().data.reg32 == 0x080887f6) {
            printf("Returning from 0x080887f6 R0=%.08X\n", cpu->reg(R0).data.reg32);
            cpu->reg(R0).data.reg32 = 1;
        }*/

        if(pc().data.reg32 == 0x080887b4) {
            word saveptr = 0x03005d8c;
            word saveblock = cpu->r32(saveptr);
            cpu->mmu->game_state_obj_addr = saveblock;
            printf("[1] Called CanCameraMoveInDirection: Saveblock addr: %.08X, pos: %.04X, %.04X\n", saveblock, cpu->r16(saveblock), cpu->r16(saveblock + 2));
        }

        /*if(pc().data.reg32 == 0x0808edE0) {
            word sprite_ptr = cpu->reg(R0).data.reg32;
            word our_sprite = sprite_ptr + 0x32;
            //printf("CameraObject_1 --> Sprite @ %.08X { data[2] = %.02X data[3] = %.02X}\n", our_sprite, cpu->r8(our_sprite), cpu->r8(our_sprite + 1));
            word pos1 = sprite_ptr + 0x20;
            cpu->mmu->game_state_obj_addr = pos1;

            //if(a) disassemble = 0x30;
            //a = false;
        }*/

        if(pc().data.reg32 == 0x080888b2) {
            cpu->reg(R10).data.reg32 = 0;
            cpu->reg(R9).data.reg32 = 0;
            printf("Adding %.08X %.08X to coordinates...\n", cpu->reg(R10).data.reg32, cpu->reg(R9).data.reg32);
            //a = true;
        }

        /*if(pc().data.reg32 == 0x0808a0f8) {
            word cameraobjptr = cpu->reg(R0).data.reg32;
            word callback = cpu->r32(cameraobjptr + 0x00);
            word spriteid = cpu->r32(cameraobjptr + 0x04);
            word v_x = cpu->r32(cameraobjptr + 0x08);
            word v_y = cpu->r32(cameraobjptr + 0x0C);
            word x = cpu->r32(cameraobjptr + 0x10);
            word y = cpu->r32(cameraobjptr + 0x14);

            printf("CameraUpdateCallback with obj @ %.08X {callback = %.08X, sid = %.08X, vx = %.08X, vy = %.08X, x = %.08X, y = %.08X}\n", cameraobjptr, callback, spriteid, v_x, v_y, x, y);

            word sprite_ptr = 0x02020662;
            word our_sprite = sprite_ptr + 0x44 * spriteid;
            printf("Sprite @ %.08X { data[2] = %.02X data[3] = %.02X}\n", our_sprite, cpu->r8(our_sprite), cpu->r8(our_sprite + 1));
            cpu->mmu->game_state_obj_addr = our_sprite;
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

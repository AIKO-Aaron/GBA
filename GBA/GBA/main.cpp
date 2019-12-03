#include <stdio.h>
#include "GBABase.h"
#include "GBADebugger.h"
#include "GBADecompiler.h"

int main(int argc, char **args) {
    printf("[INFO] Starting GBA Emulator in %s\n", args[0]);
    Debugger::Interpreter *interpreter = new Debugger::Interpreter();

	int num;
	const uint8_t* keys = SDL_GetKeyboardState(&num);

    int counter = 0; // 0x082E062D
    while(
		(interpreter->pc().data.reg32 != 0x11d9) && // address
		!interpreter->cpu->mmu->breakpoint_hit &&  // Write access / Read access
		interpreter->cpu->r32(interpreter->pc().data.reg32) != 0 &&  // No null bytes executed
		(interpreter->pc().data.reg32 & 1) == 0 && !keys[SDL_SCANCODE_0]) // Not offset
	{
        interpreter->executeNextInstruction(false);
		interpreter->cpu->mmu->button_a = keys[SDL_SCANCODE_W];
		interpreter->cpu->mmu->button_start = keys[SDL_SCANCODE_E];
		interpreter->cpu->mmu->button_up = keys[SDL_SCANCODE_UP];
		interpreter->cpu->mmu->button_down = keys[SDL_SCANCODE_DOWN];
		interpreter->cpu->mmu->button_left = keys[SDL_SCANCODE_LEFT];
		interpreter->cpu->mmu->button_right = keys[SDL_SCANCODE_RIGHT];
        if(++counter >= 280896) {
            counter = 0;
            interpreter->cpu->render();
        }
    }
    
    interpreter->printTrace();
        
    Debugger::GUI a(interpreter);
    a.start();

    //start_command_line(interpreter);
    
    
    delete interpreter;
	return 0;
}

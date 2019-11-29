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
    while((interpreter->pc().data.reg32 != 0xFFFFFFFF) && interpreter->cpu->r32(interpreter->pc().data.reg32) != 0 && (interpreter->pc().data.reg32 & 1) == 0 && !keys[SDL_SCANCODE_0]) {
        interpreter->executeNextInstruction(false);
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

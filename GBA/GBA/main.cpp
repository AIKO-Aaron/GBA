#include <stdio.h>

// #define RUN_TESTS


#ifdef RUN_TESTS
#  include "tests.h"
#else
#  include "GBABase.h"
#  include "GBADebugger.h"
#  include "GBADecompiler.h"
#endif

bool saving = false;

int start_game(int argc, char **args) {
    printf("[INFO] Starting GBA Emulator in %s\n", args[0]);
    Debugger::Interpreter *interpreter = new Debugger::Interpreter();

	int num;
	const uint8_t* keys = SDL_GetKeyboardState(&num);

    int counter = 0; // 0x082E062D
	int num_ = 0;
	while (
		(interpreter->pc().data.reg32 != 0x080008C8 || (interpreter->cpu->reg(PC).data.reg32 != 0x00000000)) && // address
		(interpreter->cpu->reg(PC).data.reg32 >> 24) != 6 &&
		!interpreter->cpu->mmu->breakpoint_hit &&  // Write access / Read access
		interpreter->cpu->r32(interpreter->pc().data.reg32) != 0 &&  // No null bytes executed
		(interpreter->pc().data.reg32 & 1) == 0 && !keys[SDL_SCANCODE_0]) // Not offset
	{
        interpreter->executeNextInstruction(false);

        if(++counter >= 280896) {
			interpreter->cpu->mmu->button_a = keys[SDL_SCANCODE_W];
			interpreter->cpu->mmu->button_b = keys[SDL_SCANCODE_KP_PERIOD];
			interpreter->cpu->mmu->button_start = keys[SDL_SCANCODE_E];
			interpreter->cpu->mmu->button_select = keys[SDL_SCANCODE_N];
			interpreter->cpu->mmu->button_up = keys[SDL_SCANCODE_UP];
			interpreter->cpu->mmu->button_down = keys[SDL_SCANCODE_DOWN];
			interpreter->cpu->mmu->button_left = keys[SDL_SCANCODE_LEFT];
			interpreter->cpu->mmu->button_right = keys[SDL_SCANCODE_RIGHT];

			if(keys[SDL_SCANCODE_KP_0] && !saving) { // Load/Store save Save
				saving = true;
				if(keys[SDL_SCANCODE_LSHIFT]) {
					size_t size = 0x40000 + 0x8000 + 0x400 + 0x400 + 0x18000 + 0x400 + 4 * 37;
					byte *data = (byte*) malloc(size);

					int index = 0;
					memcpy(data + index, interpreter->cpu->mmu->memory[2], 0x40000); index += 0x40000;
					memcpy(data + index, interpreter->cpu->mmu->memory[3], 0x8000); index += 0x8000;
					memcpy(data + index, interpreter->cpu->mmu->memory[4], 0x400); index += 0x400;
					memcpy(data + index, interpreter->cpu->mmu->memory[5], 0x400); index += 0x400;
					memcpy(data + index, interpreter->cpu->mmu->memory[6], 0x18000); index += 0x18000;
					memcpy(data + index, interpreter->cpu->mmu->memory[7], 0x400); index += 0x400;
					for(int i = 0; i < 37; i++) memcpy(data + index + i * 4, &interpreter->cpu->set->allRegisters[i]->data.reg32, 4);

					FILE *f = fopen("save.sav", "wb");
					fwrite(data, 1, size, f);
					fclose(f);
					free(data);
				} else {
					FILE *f = fopen("save.sav", "rb");
					printf("Loading data...\n");

					int index = 0, should_index = 0x40000;
					while(index < should_index) index += fread(interpreter->cpu->mmu->memory[2], 1, 0x40000, f); should_index += 0x8000;
					while(index < should_index) index += fread(interpreter->cpu->mmu->memory[3], 1, 0x8000, f); should_index += 0x400;
					while(index < should_index) index += fread(interpreter->cpu->mmu->memory[4], 1, 0x400, f); should_index += 0x400;
					while(index < should_index) index += fread(interpreter->cpu->mmu->memory[5], 1, 0x400, f); should_index += 0x18000;
					while(index < should_index) index += fread(interpreter->cpu->mmu->memory[6], 1, 0x18000, f); should_index += 0x400;
					while(index < should_index) index += fread(interpreter->cpu->mmu->memory[7], 1, 0x400, f);
					for(int i = 0; i < 37; i++) fread(&interpreter->cpu->set->allRegisters[i]->data.reg32, 4, 1, f);
					interpreter->cpu->set->allRegisters[PC]->data.reg32 -= (interpreter->cpu->set->allRegisters[CPSR]->data.reg32 & FLAG_T) ? 2 : 4;
					interpreter->disassemble = 10;
					fclose(f);
				}
			} else saving = false;

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

int main(int argc, char** args) {
#ifdef RUN_TESTS
	Test::all_tests();
	return 0;
#else
	return start_game(argc, args);
#endif
}

#include "GUI.h"

void Debugger::GUI::renderText(const char *text, int x, int y, int col) {
    if(!text) return;
	SDL_Surface *text_surface = TTF_RenderText_Shaded(font, text, {(uint8_t) ((col >> 16) & 0xFF), (uint8_t) ((col >> 8) & 0xFF), (uint8_t) ((col >> 0) & 0xFF), (uint8_t) ((col >> 24) & 0xFF) }, {0, 0, 0, 0});
	SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    if(!text_surface) return;
    
	SDL_Rect dst = {x, y, text_surface->w, text_surface->h};

	SDL_RenderCopy(renderer, text_texture, nullptr, &dst);
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
}

void Debugger::GUI::renderDecompiler() {
    renderText("> ", 0, 300);
    for(int i = -2; i <= 10; i++) {
        word addr = i * ((mDebugger->cpu->reg(CPSR).data.reg32 & FLAG_T) ? 2 : 4);
        if(addr >= 0xF000000) continue;
        std::string dis = Decompiler::decompileInstruction(mDebugger->cpu, addr, false);

        std::string to_render = Decompiler::int_to_hex(mDebugger->cpu->pc().data.reg32 + addr);
        to_render += std::string(" = (");
        if(mDebugger->cpu->reg(CPSR).data.reg32 & FLAG_T) to_render += Decompiler::int_to_hex(mDebugger->cpu->r16(mDebugger->cpu->pc().data.reg32 + addr));
        else to_render += Decompiler::int_to_hex(mDebugger->cpu->r32(mDebugger->cpu->pc().data.reg32 + addr));
        to_render += std::string(") ==>  ");
        to_render += dis;

        renderText(to_render.c_str(), 25, i * 25 + 300);
    } 
}

void Debugger::GUI::renderFlags() {
    renderText("ZERO    ", 450, 0, mDebugger->cpu->reg(CPSR) & FLAG_Z ? 0xFF00FF00 : 0xFFFF0000);
    renderText("CARRY   ", 450, 25, mDebugger->cpu->reg(CPSR) & FLAG_C ? 0xFF00FF00 : 0xFFFF0000);
    renderText("OVERFLOW", 450, 50, mDebugger->cpu->reg(CPSR) & FLAG_V ? 0xFF00FF00 : 0xFFFF0000);
    renderText("NEGATIVE", 450, 75, mDebugger->cpu->reg(CPSR) & FLAG_N ? 0xFF00FF00 : 0xFFFF0000);
    
    renderText("THUMB", 450, 150, mDebugger->cpu->reg(CPSR) & FLAG_T ? 0xFF00FF00 : 0xFFFF0000);
    renderText("MODE: ", 450, 175);
    
    byte mode = mDebugger->cpu->reg(CPSR).data.reg32 & 0x1F;
    std::string mode_str = "";
    
    switch(mode) {
        case MODE_USER:
            mode_str = "USER";
            break;
        case MODE_FIQ:
            mode_str = "FIQ";
            break;
        case MODE_IRQ:
            mode_str = "IRQ";
            break;
        case MODE_SUPERVISOR:
            mode_str = "SUPERVISOR";
            break;
        case MODE_ABORT:
            mode_str = "ABORT";
            break;
        case MODE_UNDEFINED:
            mode_str = "UNDEFINED";
            break;
        case MODE_SYSTEM:
            mode_str = "SYSTEM";
            break;
    }
    
    renderText(mode_str.c_str(), 525, 175);
}

Debugger::GUI::GUI(Debugger *debugger) : mDebugger(debugger) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    TTF_Init();
    
    font = TTF_OpenFont("Raleway-Regular.ttf", 18);
    
    debugger_window = SDL_CreateWindow("Debugger", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 750, 0);
    renderer = SDL_CreateRenderer(debugger_window, -1, SDL_RENDERER_ACCELERATED);

    vram_gui = new VRAM_GUI(debugger->cpu);
}

void Debugger::GUI::start() {
    SDL_ShowWindow(debugger_window);

    std::string text = "== ";
    bool disassembled = false;
    
    SDL_Event e;
    bool running = true;
    while(running) {
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_WINDOWEVENT) {
                if(e.window.event == SDL_WINDOWEVENT_CLOSE) running = false;
            } else if(e.type == SDL_KEYDOWN) {
                if(e.key.keysym.sym == SDLK_SPACE) mDebugger->executeNextInstruction(true);
                else if(e.key.keysym.sym == SDLK_ESCAPE) running = false;
                else if(e.key.keysym.sym == SDLK_0) text += '0';
                else if(e.key.keysym.sym == SDLK_1) text += '1';
                else if(e.key.keysym.sym == SDLK_2) text += '2';
                else if(e.key.keysym.sym == SDLK_3) text += '3';
                else if(e.key.keysym.sym == SDLK_4) text += '4';
                else if(e.key.keysym.sym == SDLK_5) text += '5';
                else if(e.key.keysym.sym == SDLK_6) text += '6';
                else if(e.key.keysym.sym == SDLK_7) text += '7';
                else if(e.key.keysym.sym == SDLK_8) text += '8';
                else if(e.key.keysym.sym == SDLK_9) text += '9';
                else if(e.key.keysym.sym == SDLK_a) text += 'A';
                else if(e.key.keysym.sym == SDLK_b) text += 'B';
                else if(e.key.keysym.sym == SDLK_c) text += 'C';
                else if(e.key.keysym.sym == SDLK_d) text += 'D';
                else if(e.key.keysym.sym == SDLK_e) text += 'E';
                else if(e.key.keysym.sym == SDLK_f) text += 'F';
                else if(e.key.keysym.sym == SDLK_r) {
                    uint32_t r = (uint32_t) strtol(text.substr(3, 1).c_str(), NULL, 16);
                    printf("Register to write to: %.02X\n", r);
                    uint32_t value = (uint32_t) strtol(text.substr(4).c_str(), NULL, 16);
                    mDebugger->cpu->reg(r).data.reg32 = value;
                    printf("Wrote value %.8X to register\n", value);
                }
                else if(e.key.keysym.sym == SDLK_m) {
                    uint32_t r = (uint32_t) strtol(text.substr(3, 8).c_str(), NULL, 16);
                    printf("Address to write to: %.08X\n", r);
                    uint8_t value = (uint8_t) strtol(text.substr(11, 2).c_str(), NULL, 16);
                    mDebugger->cpu->w8(r, value);
                    printf("Wrote value %.08X to register\n", value);
                }
                else if(e.key.keysym.sym == SDLK_i) {
                    uint32_t r = (uint32_t) strtol(text.substr(3, 8).c_str(), NULL, 16);
                    printf("Address to read from: %.08X\n", r);
                    uint32_t value = mDebugger->cpu->r32(r);
                    printf("Read value %.08X from memory\n", value);
                }
                else if(e.key.keysym.sym == SDLK_BACKSPACE && text.length() > 3) text = text.substr(0, text.length() - 1);
                else if(e.key.keysym.sym == SDLK_RETURN) {
                    mDebugger->executeNextInstruction(false);
                    running_until = (int) strtol(text.substr(3).c_str(), NULL, 16);
                    animated = true;
                    //text = "== ";
                } else if(e.key.keysym.sym == SDLK_t) {
                    mDebugger->printTrace();
                } else if(e.key.keysym.sym == SDLK_s) {
                    animated = false;
                } else if(e.key.keysym.sym == SDLK_l) {
                    disassembled = !disassembled;
                } else if(e.key.keysym.sym == SDLK_LEFT) ++vram_gui->loffset;
                else if(e.key.keysym.sym == SDLK_h) {
                    mDebugger->executeNextInstruction(false);
                    running_until = (int) strtol(text.substr(3).c_str(), NULL, 16);
                    int counter = 0;
                    while(mDebugger->cpu->pc().data.reg32 != running_until) 
                    {
                        mDebugger->executeNextInstruction(false);
                        if(++counter >= 280896) {
                            counter = 0;
                            mDebugger->cpu->render();
                        }
                    }
                }
				else if (e.key.keysym.sym == SDLK_KP_ENTER) {
					FILE* f;
					fopen_s(&f, "hexdump.bin", "wb");
					if (!f) {
						printf("Could not create file...\n");
						continue;
					}
					fwrite(mDebugger->cpu->mmu->memory[6], 1, 0x18000, f);
					fclose(f);
				}
				else if (e.key.keysym.sym == SDLK_x) {
					printf("triggering breakpoint...\n");
				}
            }
        }
        
        if(animated && mDebugger->cpu->pc().data.reg32 != running_until) {
            for(int i = 0; i < 1000 && animated; i++) {
                mDebugger->executeNextInstruction(disassembled);
                if(disassembled) mDebugger->printRegisters();
                if(mDebugger->cpu->pc().data.reg32 == running_until) animated = false;
            }
        } else if(animated) {
            running_until = 0;
            animated = false;
        }
        
        vram_gui->render();
        mDebugger->cpu->render();

#define _RENDER_DEBUGGER
#ifdef _RENDER_DEBUGGER
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
        SDL_RenderClear(renderer);

        SDL_Rect rect = {0, 0, 200, 25};
        for(int i = 0; i < 18; i++) {
            std::string to_render;
            to_render += Decompiler::reg_names_with_constant_length[i];
            to_render += "    " + Decompiler::int_to_hex(mDebugger->cpu->reg(i).data.reg32);
            renderText(to_render.c_str(), 200 * (i % 2), 25 * (int)(i / 2));
            rect.x = 200 * (i % 2);
            rect.y = 25 * (int)(i / 2);

            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderDrawRect(renderer, &rect);
        }

        renderText("Disassembling", 450, 200, disassembled ? 0xFF00FF00 : 0xFFFF0000);

        renderText(text.c_str(), 0, 600);
        renderDecompiler();
        renderFlags();
#endif

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(debugger_window);
    SDL_Quit();
}

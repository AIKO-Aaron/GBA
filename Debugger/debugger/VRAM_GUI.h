#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <CPU/CPU.h>

namespace Debugger {
    
    class VRAM_GUI {
    private:
        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;
        Base::CPU *cpu = nullptr;

    public:
        int loffset = 0;
        VRAM_GUI(Base::CPU *_cpu);
        void render();
        void render_background(byte bg);
		void render_tile(byte num);
        void render_memory(word start, int len);
    };
}
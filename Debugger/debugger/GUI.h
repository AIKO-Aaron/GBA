#pragma once

#include "Debugger.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "VRAM_GUI.h"

namespace Debugger {
    
    class GUI {
    private:
        SDL_Window *debugger_window = nullptr;
        SDL_Renderer *renderer = nullptr;
	    TTF_Font *font = nullptr;

        VRAM_GUI *vram_gui = nullptr;
        
        Debugger *mDebugger = nullptr;
        word running_until = 0;
        bool animated = false;
        
	    void renderText(const char *text, int x, int y, int col = 0xFFFFFFFF);
        void renderDecompiler();
        void renderFlags();

    public:
        GUI(Debugger *debugger);
        void start();
    };
    
}

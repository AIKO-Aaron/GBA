#pragma once

#include "../MMU/MMU.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace Base {
    class CPU;
    class GPU {
        private:
            SDL_Window *window = nullptr;
            SDL_Renderer *renderer = nullptr;

            Base::MMU *mmu;

            int cycles = 0;
            int x_dot = 0, y_dot = 0;
            hword default_colors[240 * 160];

        public:
            GPU(Base::MMU *mmu);

            void update();
            void render(Base::CPU *cpu);

            inline void update_cycles(int num) { cycles += num; }
    };

}

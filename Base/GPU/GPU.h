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

			byte bg_mode = 0;
            int cycles = 0;
            int x_dot = 0, y_dot = 0;
            hword default_colors[240 * 160];

        public:
            GPU(Base::MMU *mmu);

            void update();
            void render(Base::CPU *cpu);
			void draw_tile(hword palette, byte *data, hword *plt, int screen_x, int screen_y, bool color_depth, bool horizontal_flip, bool vertical_flip);
			void render_background(byte bg);

            inline void update_cycles(int num) { cycles += num; }
    };

}

#include "VRAM_GUI.h"

Debugger::VRAM_GUI::VRAM_GUI(Base::CPU *_cpu) : cpu(_cpu) {
    window = SDL_CreateWindow("VRAM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

int i = 0;
byte randValue = 0;

void Debugger::VRAM_GUI::render_background(byte bg) {
    word cntrl = cpu->mmu->memory[4][8 + 2 * bg];

    word off = ((cntrl >> 2) & 3) * 0x4000;
    bool color_depth = cntrl & (1 << 7);

    hword *palette_bg = (hword*) (cpu->mmu->memory[5]);
    byte *vram_data = cpu->mmu->memory[6];

    for(int i = 0; i < 16 * 2; i++) {
        for(int j = 0; j < 16 * 2; j++) {

            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0xFF);
            SDL_RenderDrawPoint(renderer, j * 8 - 1 + (bg % 2) * 256, i * 8 - 1 + (bg / 2) * 256);

            word off_tot = off + i * 32 + j;
            for(int y = 0; y < 8; y++) {
                for(int x = 0; x < (color_depth ? 8 : 4); x++) {
                    if(color_depth) {
                        byte a = vram_data[off_tot + y * (color_depth ? 8 : 4) + x];
                        hword color = palette_bg[a];
                        SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
                        SDL_RenderDrawPoint(renderer, (int) x + j * 8 + (bg % 2) * 256, (int) y + i * 8 + (bg / 2) * 256);
                    } else {
                        byte a = vram_data[off_tot + y * (color_depth ? 8 : 4) + x];
                        int b1 = ((randValue) & 0xF0) | (a & 0xF);
                        int b2 = ((randValue) & 0xF0) | (a >> 4);
                        hword color = palette_bg[b1];
                        SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
                        SDL_RenderDrawPoint(renderer, (int) x * 2 + 1 + j * 8 + (bg % 2) * 256, (int) y + i * 8 + (bg / 2) * 256);
                        color = palette_bg[b2];
                        SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
                        SDL_RenderDrawPoint(renderer, (int) x * 2 + 0 + j * 8 + (bg % 2) * 256, (int) y + i * 8 + (bg / 2) * 256);
                    }
                }
            }
        }
    }

    if(i++ % 100 == 0) randValue++;
}

void Debugger::VRAM_GUI::render() {
    // byte *ioctrl = cpu->mmu->memory[4];

    hword dispcnt = cpu->r16(0x04000000);
    hword bg0_cnt = cpu->r16(0x04000008);
    hword bg1_cnt = cpu->r16(0x0400000A);
    hword bg2_cnt = cpu->r16(0x0400000C);
    hword bg3_cnt = cpu->r16(0x0400000E);

    hword *palette_bg = (hword*) (cpu->mmu->memory[5] + 0x000);
    bool one_d_mapping = dispcnt & (1 << 6);

    hword color = *(hword*)(cpu->mmu->memory[5]);
    SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
    SDL_RenderClear(renderer);

    // printf("Color depth: %s\n", color_depth ? "8 bit" : "4 bit");

    render_background(0);
    // for(int i = 0; i < 4; i++) render_background(i);

    SDL_RenderPresent(renderer);
}

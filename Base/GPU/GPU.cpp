#include "GPU.h"
#include "../CPU/CPU.h"

#define SCALE 1


static const byte* width_table = new const byte[16] {
	1, 2, 4, 8,
	2, 4, 4, 8,
	1, 1, 2, 4,
	0xFF, 0xFF, 0xFF, 0xFF
};

static const byte* height_table = new const byte[16]{
	1, 2, 4, 8,
	1, 1, 2, 4,
	2, 4, 4, 8,
	0xFF, 0xFF, 0xFF, 0xFF
};

#define MULTI_THREAD

Base::GPU::GPU(Base::MMU *m) : mmu(m) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    window = SDL_CreateWindow("GBA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 240 * SCALE, 160 * SCALE, 0);
   // renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_ShowWindow(window);
}

int frame = 0;

int i = 0;
byte randValue = 0;
static int frame_num = 0;

inline void Base::GPU::draw_tile(hword palette, byte *data, hword *plt, int x_off, int y_off, bool color_depth, bool horizontal_flip, bool vertical_flip, bool use_palette) {
	if (x_off < -8 || y_off < -8 || x_off >= 240 || y_off >= 160) return;
	SDL_Rect r = { 0, 0, SCALE, SCALE };
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < (color_depth ? 8 : 4); ++x) {

			int point_x = vertical_flip ? ((color_depth ? 7 : 3) - x) : x;
			int point_y = horizontal_flip ? (7 - y) : y;

			if (color_depth) {
				byte a = data[y * 8 + x];
				hword color = use_palette ? plt[a] : a;
				if (a || !use_palette) {
					SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
					if(SCALE == 1) SDL_RenderDrawPoint(renderer, (int)point_x + x_off, (int)point_y + y_off);
					else {
						r.x = (point_x + x_off) * SCALE;
						r.y = (point_y + y_off) * SCALE;
						SDL_RenderFillRect(renderer, &r);
					}
				}
			}
			else {
				byte a = data[y * 4 + x];
				int b1 = ((palette << 4) & 0xF0) | (a & 0xF);
				int b2 = ((palette << 4) & 0xF0) | (a >> 4);
				hword color = use_palette ? plt[b1] : b1;
				if ((a & 0xF) || !use_palette) {
					SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
					if(SCALE == 1) SDL_RenderDrawPoint(renderer, (int)point_x * 2 + (vertical_flip ? 1 : 0) + (int)x_off, (int) point_y + y_off);
					else {
						r.x = (point_x * 2 + (vertical_flip ? 1 : 0) + x_off) * SCALE;
						r.y = (point_y + y_off) * SCALE;
						SDL_RenderFillRect(renderer, &r);
					}
				}
				color = use_palette ? plt[b2] : b2;
				if ((a >> 4) || !use_palette) {
					SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
					if(SCALE == 1) SDL_RenderDrawPoint(renderer, (int)point_x * 2 + (vertical_flip ? 0 : 1) + (int)x_off, (int) point_y + y_off);
					else {
						r.x = (point_x * 2 + (vertical_flip ? 0 : 1) + x_off) * SCALE;
						r.y = (point_y + y_off) * SCALE;
						SDL_RenderFillRect(renderer, &r);
					}
				}
			}
		}
	}
}

inline void Base::GPU::render_background(byte bg) {
	hword bg_mode = mmu->memory[4][0] & 7;
	// if (bg_mode) printf("bg mode is %.02X\n", bg_mode);

	byte mode = bg_mode < 2 || (bg_mode == 2 && bg < 2); // true if we're in text mode, false otherwise

	//if (mode) return;

	if (bg_mode == 1 && bg == 3) return; // No background 3 in bg mode 1
	if (bg_mode == 2 && bg < 2) return; // no bg 0 & 1 in bg mode 2
	if (bg_mode > 2 && bg != 2) return; // only bg 2 in bg modes above 2

	hword cntrl = *(hword*) (&mmu->memory[4][8 + 2 * (int) bg]);
	byte size = (cntrl >> 14) & 0x3;

	word off = ((cntrl >> 2) & 3) * 0x4000;
	bool color_depth = cntrl & (1 << 7);

	hword* palette_bg = (hword*)(mmu->memory[5]);
	byte* vram_data = mmu->memory[6];

	int x_off = mmu->memory[4][0x10 + 4 * bg] | ((mmu->memory[4][0x11 + 4 * bg] << 8) & 0x100);
	int y_off = mmu->memory[4][0x12 + 4 * bg] | ((mmu->memory[4][0x13 + 4 * bg] << 8) & 0x100);

	word base_off = (cntrl >> 8) & 0x1F;
	word start_addr = base_off * 0x800;

	for (int i = 0; i < 32 * 32 * (size > 0 ? 2 : 1) * (size == 3 ? 2 : 1); i++) {
		hword data;
		if (bg_mode == 0 || (bg_mode == 1 && bg <= 1)) {
			start_addr += 2;
			data = *(hword*)(vram_data + start_addr);
		}
		else {
			++start_addr;
			data = *(byte*)(vram_data + start_addr);
		}

		draw_tile(data >> 12, &vram_data[off + (color_depth ? 64 : 32) * ((int) (data & 0x3FF))], palette_bg, (int) (i % 32) * 8 - x_off, (int) (i / 32) * 8 - y_off, color_depth, data & (1 << 11), data & (1 << 10), bg_mode < 3);
	}
}

void Base::GPU::update() {
    hword flags = 0;


    while(cycles >= 4) {
        cycles -= 4;
        ++x_dot;
        if(x_dot >= 240) flags |= 0x2;
        if(x_dot >= 308) {
            x_dot = 0;
            ++y_dot;

            if(y_dot == 0xA0) flags |= 0x1;

            if(y_dot >= 228) {
                y_dot = 0;
            }
        }

        if(y_dot == mmu->r8(0x04000004) && x_dot == 0) flags |= 0x4;

        // if(y_dot == 0x9F && x_dot == 0) printf("Frame %d\n", frame++); 

        byte *io = mmu->memory[4];
        io[0x202] |= flags;
        io[4] |= flags;
        mmu->w8(0x04000006, y_dot);
    }

}

void Base::GPU::render(Base::CPU *cpu) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_WINDOWEVENT) {
            if(e.window.event == SDL_WINDOWEVENT_CLOSE) exit(0);
        } else if(e.type == SDL_KEYDOWN) {
            if(e.key.keysym.sym == SDLK_ESCAPE) exit(0);
            else if(e.key.keysym.sym == SDLK_r) cpu->pc().data.reg32 = 0;
            //else if(e.key.keysym.sym == SDLK_b) cpu->pc().data.reg32 += 1;
        }
    }

    hword dispcnt = cpu->r16(0x04000000);

	bg_mode = dispcnt & 0x7;

    hword bg0_cnt = cpu->r16(0x04000008);
    hword bg1_cnt = cpu->r16(0x0400000A);
    hword bg2_cnt = cpu->r16(0x0400000C);
    hword bg3_cnt = cpu->r16(0x0400000E);

    hword *palette_obj = (hword*) (cpu->mmu->memory[5] + 0x200);
    bool one_d_mapping = dispcnt & (1 << 6);

    hword color = *(hword*)(cpu->mmu->memory[5]);
    SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
    SDL_RenderClear(renderer);

    // printf("Dispcnt: %.04X\n", dispcnt);

    bool force_blank = (dispcnt & 0x80);
    force_blank = false;

	if (!force_blank) {
		for (int i = 3; i >= 0; i--) {
			if (dispcnt & 0x0100 && (bg0_cnt & 3) == i) render_background(0);
			if (dispcnt & 0x0200 && (bg1_cnt & 3) == i) render_background(1);
			if (dispcnt & 0x0400 && (bg2_cnt & 3) == i) render_background(2);
			if (dispcnt & 0x0800 && (bg3_cnt & 3) == i) render_background(3);
		}
	}


    for(int index = 0; index < 128 && !force_blank; index++) {
        hword attrib0 = cpu->r16(0x07000000 + index * 8);
        hword attrib1 = cpu->r16(0x07000002 + index * 8);
        hword attrib2 = cpu->r16(0x07000004 + index * 8);

        bool rotscal = attrib0 & 0x0100;
        bool double_size = attrib0 & 0x0200;
        byte mode = (attrib0 >> 10) & 0x3;

        if(rotscal) {
			//printf("I think you didn't implement it yet..\n");
			continue;
		}
		//else if(attrib0 & (1 << 9)) continue;


        byte shape = (attrib0 >> 14) & 3;
        byte size = (attrib1 >> 14) & 3;

        word x = (attrib1 & 0x1FF), y = (attrib0 & 0xFF), w, h;

        if(y & 0x80) y |= 0xFFFFFF00;
        if(x & 0x100) x |= 0xFFFFFE00;

		w = width_table[shape * 4 + size];
		h = height_table[shape * 4 + size];

		if (shape == 3) {
			printf("What?\n");
			continue;
		}

        bool depth = (attrib0 & (1 << 13));
        word name = attrib2 & 0x3FF;

		if (!one_d_mapping) name &= 0xFFFFFFFE;

		//printf("Rendering tile with name: %.08X\n", name);

        for(word yc = 0; yc < h; yc++) {
            for(word xc = 0; xc < w; xc++) {
				word xc2 = depth ? (xc*2) : xc;
				word tile_index = name + (one_d_mapping ? (xc2 + yc * w) : (xc2 + yc * 32));

				//if (xc || yc) continue;
				
				byte* data = &(mmu->memory[6][(bg_mode < 3 ? 0x00010000 : 0x00014000) + tile_index * (depth ? 32 : 32)]);

				draw_tile((attrib2 >> 12) & 0xF, data, palette_obj, x + xc * 8, y + yc * 8, depth, attrib1 & (1 << 13), attrib1 & (1 << 12), true);
            }
        }
    }

	//printf("Frame %d\n", frame_num++);
	SDL_RenderPresent(renderer);
}

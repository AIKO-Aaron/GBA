#include "VRAM_GUI.h"

TTF_Font* vram_font = nullptr;
const char* chars_[16] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "F" };
SDL_Texture** hex_chars = new SDL_Texture * [16];

Debugger::VRAM_GUI::VRAM_GUI(Base::CPU *_cpu) : cpu(_cpu) {
    window = SDL_CreateWindow("VRAM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	vram_font = TTF_OpenFont("Raleway-Regular.ttf", 18);
	SDL_Color col = { 0xFF, 0x00, 0xFF, 0xFF };
	for (int i = 0; i < 16; i++) {
		SDL_Surface* s = TTF_RenderText_Solid(vram_font, chars_[i], col);
		hex_chars[i] = SDL_CreateTextureFromSurface(renderer, s);
		SDL_FreeSurface(s);
	}
}

static int i = 0;
static byte randValue = 0;

void Debugger::VRAM_GUI::render_background(byte bg) {
	hword cntrl = *(hword*)(&cpu->mmu->memory[4][8 + 2 * (int)bg]);

    word off = ((cntrl >> 2) & 3) * 0x4000;
    bool color_depth = cntrl & (1 << 7);

    hword *palette_bg = (hword*) (cpu->mmu->memory[5]);
    byte *vram_data = cpu->mmu->memory[6];

    for(int i = 0; i < 16 * 2; i++) {
        for(int j = 0; j < 16 * 2; j++) {

            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0xFF);
            SDL_RenderDrawPoint(renderer, j * 8 - 1 + (bg % 2) * 256, i * 8 - 1 + (bg / 2) * 256);

            word off_tot = off + (i * 32 + j) * (color_depth ? 64 : 32);
			

			byte* data = vram_data + off_tot;

			for (int y = 0; y < 8; y++) {
				for (int x = 0; x < (color_depth ? 8 : 4); x++) {
					if (color_depth) {
						byte a = data[y * 8 + x];
						hword color = palette_bg[a];
						if (a) {
							SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
							SDL_RenderDrawPoint(renderer, (int)x + j * 8 + (bg % 2) * 256, (int)y + i * 8 + (bg / 2) * 256);
						}
					}
					else {
						byte a = data[y * 4 + x];
						int b1 = ((randValue) & 0xF0) | (a & 0xF);
						int b2 = ((randValue) & 0xF0) | (a >> 4);
						hword color = palette_bg[b1];
						if (b1) {
							SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
							SDL_RenderDrawPoint(renderer, (int)x * 2 + 0 + j * 8 + (bg % 2) * 256, (int)y + i * 8 + (bg / 2) * 256);
						}
						color = palette_bg[b2];
						if (b2) {
							SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
							SDL_RenderDrawPoint(renderer, (int)x * 2 + 1 + j * 8 + (bg % 2) * 256, (int)y + i * 8 + (bg / 2) * 256);
						}
					}
				}
			}
        }
    }

    //if(i++ % 100 == 0) randValue++;
}

static int rendered_tiles = 0;

void Debugger::VRAM_GUI::render_tile(byte tile) {


	hword* palette_bg = (hword*)(cpu->mmu->memory[5]);
	byte* vram_data = cpu->mmu->memory[6] + 0x0001E000;
	bool color_depth = false;

	byte bg_mode = (cpu->r16(0x04000000)) & 7;
	bool use_palette = bg_mode < 3;

	word off = tile * 32;
	byte* data = vram_data + off;

	SDL_Rect r = {0, 0, color_depth ? 4 : 2, 4};

	SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0xFF);
	SDL_RenderDrawPoint(renderer, (rendered_tiles % 16) * 32, (rendered_tiles / 16) * 32);

	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < (color_depth ? 8 : 4); x++) {
			if (color_depth) {
				byte a = data[y * 8 + x];
				hword color = use_palette ? palette_bg[a] : a;
				if (!use_palette || a) {
					SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);

					r.x = x * 4 + (rendered_tiles % 16) * 32;
					r.y = y * 4 + (rendered_tiles / 16) * 32;
					SDL_RenderFillRect(renderer, &r);
				}
			}
			else {
				byte a = data[y * 4 + x];
				int b1 = ((randValue) & 0xF0) | (a & 0xF);
				int b2 = ((randValue) & 0xF0) | (a >> 4);
				hword color = use_palette ? palette_bg[b1] : b1;
				if (!use_palette || (a & 0xF)) {
					SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
					r.x = (x * 2 + 0) * 4 + (rendered_tiles % 16) * 32;
					r.y = (y) * 4 + (rendered_tiles / 16) * 32;
					SDL_RenderFillRect(renderer, &r);
				}
				color = use_palette ? palette_bg[b2] : b2;
				if (!use_palette || (a >> 4)) {
					SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
					r.x = (x * 2 + 1) * 4 + (rendered_tiles % 16) * 32;
					r.y = (y) * 4 + (rendered_tiles / 16) * 32;
					SDL_RenderFillRect(renderer, &r);
				}
			}
		}
	}

	++rendered_tiles;
}

void Debugger::VRAM_GUI::render_memory(word start, int len) {
	SDL_Rect dst = {0, 20};

	int w, h, access;
	unsigned int frmt;

	for (int i = 0; i < len; i++) {
		byte d = cpu->mmu->r8(start + i);
		byte first_digit = d / 16;
		byte second_digit = d % 16;

		SDL_Texture* fd = hex_chars[first_digit];
		SDL_Texture* sd = hex_chars[second_digit];

		SDL_QueryTexture(fd, &frmt, &access, &w, &h);
		dst.w = w;
		dst.h = h;
		SDL_RenderCopy(renderer, fd, NULL, &dst);
		dst.x += w;

		SDL_QueryTexture(sd, &frmt, &access, &w, &h);
		dst.w = w;
		dst.h = h;
		SDL_RenderCopy(renderer, sd, NULL, &dst);
		dst.x += w + 10;

		if (i % 16 == 15) {
			dst.x = 0;
			dst.y += h;
		}
	}
}

void Debugger::VRAM_GUI::render() {
    // byte *ioctrl = cpu->mmu->memory[4];

    hword dispcnt = cpu->r16(0x04000000);
    hword bg0_cnt = cpu->r16(0x04000008);
    hword bg1_cnt = cpu->r16(0x0400000A);
    hword bg2_cnt = cpu->r16(0x0400000C);
    hword bg3_cnt = cpu->r16(0x0400000E);

    hword color = *(hword*)(cpu->mmu->memory[5]);
    SDL_SetRenderDrawColor(renderer, (color & 0x1F) << 3, ((color >> 5) & 0x1F) << 3, ((color >> 10) & 0x1F) << 3, 0xFF);
    SDL_RenderClear(renderer);

    //for(int i = 0; i < 4; i++) render_background(i);

	rendered_tiles = 0;
	//for(int i = 0; i < 16 * 16; i++)	render_tile(i);

	render_memory(0x02021730, 0x40);

	SDL_RenderPresent(renderer);
}

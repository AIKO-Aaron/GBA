#pragma once

#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
#include "../Base/DataTypes.h"

namespace Base {

	void AudioCallback(void* userdata, Uint8* stream, int len);

	class AudioController {
	private:
		SDL_AudioDeviceID device;

		std::vector<byte> buffer_a, buffer_b;
		friend void AudioCallback(void* userdata, Uint8* stream, int len);

		byte* mem_a, * mem_b, *io_mem;
		byte note_a = 0, note_b = 0;

	public:
		AudioController(byte *io_mem);
		~AudioController();

		inline void push_dma_a(byte* data) { mem_a = data;  for (int i = 0; i < 16; i++) buffer_a.push_back(data[i]); }
		inline void push_dma_b(byte* data) { mem_b = data;  for (int i = 0; i < 16; i++) buffer_b.push_back(data[i]); }

		inline void repeat_dma_a() { push_dma_a(mem_a); }
		inline void repeat_dma_b() { push_dma_b(mem_b); }

		inline bool play_next_a() { if (!buffer_a.size()) return true; note_a = buffer_a[0]; buffer_a.erase(buffer_a.begin()); return buffer_a.size() <= 16; }
		inline bool play_next_b() { if (!buffer_b.size()) return true; note_b = buffer_b[0]; buffer_b.erase(buffer_b.begin()); return buffer_b.size() <= 16; }

	};

}
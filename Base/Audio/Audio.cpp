#include "Audio.h"

#define SAMPLES 1

void Base::AudioCallback(void* userdata, Uint8* stream, int len) {
	Base::AudioController* ctrl = (Base::AudioController*) userdata;

	hword soundbias = *(hword*) &ctrl->io_mem[0x88];
	if(!soundbias) *(hword*)& ctrl->io_mem[0x88] = 0x0200;

	
	hword sndcnt_h = *(hword*)&ctrl->io_mem[0x82];
	if (sndcnt_h & 0x8000) {
		ctrl->buffer_b.clear();
		*(hword*)&ctrl->io_mem[0x82] = sndcnt_h & 0x7FFF;
	}
	if (sndcnt_h & 0x0800) {
		ctrl->buffer_a.clear();
		*(hword*)&ctrl->io_mem[0x82] = sndcnt_h & 0xF7FF;
	}

	//printf("Soundbias2 = %.04X; Buflen = %.02X; SOUNDCNT_H = %.04X\n", soundbias, len, sndcnt_h);

	stream[0] = ctrl->note_b;
	stream[1] = ctrl->note_a;
	for (int i = 0; i < SAMPLES - 1; i++) {
		stream[2 * i + 2] = ctrl->buffer_b.size() ? ctrl->buffer_b[i] : 0;
		stream[2 * i + 3] = ctrl->buffer_a.size() ? ctrl->buffer_a[i] : 0;
	}
}


Base::AudioController::AudioController(byte *io) : io_mem(io) {
	SDL_AudioSpec audioSpec, actualSpec;

	SDL_memset(&audioSpec, 0, sizeof(SDL_AudioSpec));
	audioSpec.freq = 32768 / 8;
	audioSpec.format = AUDIO_S8;
	audioSpec.channels = 2;
	audioSpec.samples = SAMPLES;
	audioSpec.callback = Base::AudioCallback;
	audioSpec.userdata = this;


	device = SDL_OpenAudioDevice(NULL, 0, &audioSpec, &actualSpec, 0);
	//printf("Samples of actual spec: %.08X, format: %.08X\n", actualSpec.samples, actualSpec.format);
	if (device) {
		// SDL_PauseAudioDevice(device, 0);
	} else {
		printf("[ERROR] Could not open Audio device for playback... (%s)\n", SDL_GetError());
		exit(0);
	}
}

Base::AudioController::~AudioController() {
	SDL_CloseAudioDevice(device);
}
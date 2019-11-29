#pragma once

#include <stdint.h>
#include <stdio.h>
#include <fstream>

namespace Base {
	typedef struct {
		uint8_t *data;
		uint32_t size;
	} filedata;
	
	static filedata readFile(const char *path) {
		FILE *f = nullptr;
		
#ifdef __APPLE__
		f = fopen(path, "rb");
#else
		fopen_s(&f, path, "rb");
		printf("[INFO] Loading file: %s\n", path);
#endif
		if(!f) {
			printf("[ERROR] File not found: %s\n", path);
			return {};
		}
		
		filedata data;
		
		fseek(f, 0, SEEK_END);
		data.size = (uint32_t) ftell(f);
		fseek(f, 0, SEEK_SET);
		data.data = (byte*) malloc(data.size + 1);
		fread(data.data, 1, data.size, f);
		fclose(f);
		data.data[data.size] = 0;
		
		return data;
	}
}

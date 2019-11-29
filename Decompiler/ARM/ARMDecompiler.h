#pragma once

#include "GBABase.h"

namespace Decompiler {
	
    extern const char** reg_names, **reg_names_with_constant_length;
    extern const char** conditions;

    std::string int_to_hex(word num, int str_len = 8);
	std::string decompileARM(word instruction, Base::CPU *cpu, bool print = true);
	
}

#pragma once

#include "../ARM/ARMDecompiler.h" // Some definitions for the register names

namespace Decompiler {
    std::string decompileTHUMB(hword instruction, Base::CPU *cpu, bool = true);
}

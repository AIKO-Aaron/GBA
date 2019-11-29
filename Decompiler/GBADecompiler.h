#pragma once

#include "ARM/ARMDecompiler.h"
#include "THUMB/THUMBDecompiler.h"

namespace Decompiler {

    inline std::string decompileInstruction(Base::CPU *cpu, word offset, bool print = true) {
        std::string ret;
        cpu->pc().data.reg32 += offset;
        if(cpu->reg(CPSR).data.reg32 & FLAG_T) {
            hword i = cpu->r16(cpu->pc().data.reg32);
            cpu->pc().data.reg32 += 2;
            ret = decompileTHUMB(i, cpu, print);
            cpu->pc().data.reg32 -= 2;
        }
        else {
            word i = cpu->r32(cpu->pc().data.reg32);
            cpu->pc().data.reg32 += 4;
            ret = decompileARM(i, cpu, print);
            cpu->pc().data.reg32 -= 4;
        }
        
        cpu->pc().data.reg32 -= offset;
        return ret;
    }

}
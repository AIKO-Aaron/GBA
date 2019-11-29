#pragma once

#include "GBABase.h"

namespace Debugger {
    
    void execute_thumb(hword instruction, Base::CPU *cpu);
    
    void thumb_data_processing(hword instruction, Base::CPU *cpu);
    void thumb_alu(hword instruction, Base::CPU *cpu);
    
}

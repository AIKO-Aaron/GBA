#pragma once

#include "GBABase.h"

namespace Debugger {
    
    void execute_arm(word instruction, Base::CPU *cpu);
    
    void arm_branch(word instruction, Base::CPU *cpu);
    void arm_data_processing(word instruction, Base::CPU *cpu);
    void arm_load(word instruction, Base::CPU *cpu);
    void arm_store(word instruction, Base::CPU *cpu);
    void arm_loadh(word instruction, Base::CPU *cpu);
    void arm_storeh(word instruction, Base::CPU *cpu);
    
}

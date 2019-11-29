#pragma once

#include "DataTypes.h"
#include "../CPU/CPU.h"

namespace Base {
	
	bool isConditionMet(byte condition, word cpsrContents);
    word shift(byte data, word reg, Base::CPU *cpu, bool set_cond = true);
	word rotate(byte value, byte rotate, Base::CPU *cpu, bool set_cond = true);
	
}

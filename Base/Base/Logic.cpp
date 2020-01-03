#include "Logic.h"
#include <iostream>

bool Base::isConditionMet(byte condition, word cpsrContents) {
	bool retVal = false;
	
	switch (condition & 0xF) {
		case 0x0:
			retVal = cpsrContents & FLAG_Z;
			break;
		case 0x1:
			retVal = !(cpsrContents & FLAG_Z);
			break;
		case 0x2:
			retVal = cpsrContents & FLAG_C;
			break;
		case 0x3:
			retVal = !(cpsrContents & FLAG_C);
			break;
		case 0x4:
			retVal = cpsrContents & FLAG_N;
			break;
		case 0x5:
			retVal = !(cpsrContents & FLAG_N);
			break;
		case 0x6:
			retVal = cpsrContents & FLAG_V;
			break;
		case 0x7:
			retVal = !(cpsrContents & FLAG_V);
			break;
		case 0x8:
			retVal = (cpsrContents & FLAG_C) && !(cpsrContents & FLAG_Z);
			break;
		case 0x9:
			retVal = !(cpsrContents & FLAG_C) || (cpsrContents & FLAG_Z);
			break;
		case 0xA:
			retVal = ((cpsrContents & FLAG_N) >> 3) == (cpsrContents & FLAG_V);
			break;
		case 0xB:
			retVal = ((cpsrContents & FLAG_N) >> 3) != (cpsrContents & FLAG_V);
			break;
		case 0xC:
			retVal = (((cpsrContents & FLAG_N) >> 3) == (cpsrContents & FLAG_V)) && !(cpsrContents & FLAG_Z);
			break;
		case 0xD:
			retVal = (((cpsrContents & FLAG_N) >> 3) != (cpsrContents & FLAG_V)) || (cpsrContents & FLAG_Z);
			break;
		case 0xE:
			retVal = true;
			break;
		case 0xF:
			retVal = false;
			break;
	}
	
	return retVal;
}

word Base::shift(byte data, word reg, Base::CPU *cpu, bool set_cond) {
	word result = reg;
	byte amt = data & 1 ? (cpu->reg((data >> 4) & 0xF).data.reg32 & 0xFF) : ((data >> 3) & 0x1F);
	
	if(data & 1) {
    	if(((data >> 4) & 0xF) == 0xF) {
			amt += cpu->reg(CPSR) & FLAG_T ? 2 : 4;
		}
	}

	bool carryOut = false;
	
	switch((data >> 1) & 0x3) {
		case 0: // Logical left
			carryOut = amt == 0 ? (cpu->reg(CPSR).data.reg32 & FLAG_C) : (result & (1 << (32 - amt))); // If amount is 0, don't change FLAG_C
			result <<= amt;
			break;
		case 1: // Logical right
			carryOut = result & (1 << (amt - 1));
			result >>= amt;
			break;
		case 2: // arithmetic right
			if(amt == 0 && !(data & 1)) {
				result = result & 0x80000000 ? 0xFFFFFFFF : 0;
			}
			carryOut = result & (1 << (amt - 1));
			result >>= amt;
			if(reg >> 31) result |= (0xFFFFFFFF << (32 - amt));
			break;
		case 3: // rotate right
			if(amt == 0) { // RRX
				carryOut = result & 1;
				result = (result >> 1) | (cpu->reg(CPSR).data.reg32 & FLAG_C ? 0x10000000 : 0);
			} else {
				carryOut = result & (1 << (amt - 1));
				result = (result >> amt) | (result << (32 - amt));
			}
			break;
	}
	
	if(set_cond) {
		if(carryOut) cpu->reg(CPSR).data.reg32 |= FLAG_C;
		else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
	}

	return result;
}

word Base::rotate(byte value, byte rotate, Base::CPU *cpu, bool set_cond) {
	word amt = rotate * 2;
	word res = value;
	
	res = (res >> amt) | (res << (32 - amt));
	
	if(set_cond && amt != 0) {
		if(res & 0x80000000) cpu->reg(CPSR).data.reg32 |= FLAG_C;
		else cpu->reg(CPSR).data.reg32 &= ~FLAG_C;
	}

	return res;
}

#include "RegisterSet.h"

Base::RegisterSet::RegisterSet() {
	allocateRegisters();
	for(int i = 0; i < 37; i++) allRegisters[i]->data.reg32 = 0; // memset to 0
	for(int i = 0; i < 6; i++) registerBanks[i][SP]->data.reg32 = 0x03007F00;
}

Base::RegisterSet::~RegisterSet() {
	deallocateRegisters();
}

Base::RegisterSet::RegisterSet(Base::RegisterSet &cpy) {
	allocateRegisters();
	for(int i = 0; i < 37; i++) allRegisters[i]->data.reg32 = cpy.allRegisters[i]->data.reg32;
}

void Base::RegisterSet::allocateRegisters() {
	registerBanks.clear();
	
	allRegisters = new Base::Register*[37];
	
	for(int i = 0; i < 37; i++) allRegisters[i] = new Base::Register();
	
	for(int i = 0; i < 6; i++) {
		std::vector<Base::Register*> bank;
		for(int j = 0; j < 8; j++) bank.push_back(allRegisters[j]); // Registers R0-R7
		
		if(i == 1) for(int j = 8; j < 13; j++) bank.push_back(allRegisters[j + 9]); // MODE_FIQ (R8-R12 banked, after initial 17 registers)
		else for(int j = 8; j < 13; j++) bank.push_back(allRegisters[j]); // All other modes (R8-R12 normal)

		if(i != 0) for(int j = 0; j < 2; j++) bank.push_back(allRegisters[20 + j + i * 2]);
		else for(int j = 13; j < 15; j++) bank.push_back(allRegisters[j]);
		
		bank.push_back(allRegisters[PC]); // PC is not banked
		bank.push_back(allRegisters[CPSR]); // CPSR is not banked
		
		if(i != 0) bank.push_back(allRegisters[31 + i]);
		else bank.push_back(allRegisters[CPSR]); // No SPSR register... this line should be deleted, but is in for now as it might prevent some crashes
	
		registerBanks.push_back(bank);
	}
    currentRegisterBank = registerBanks[0];
}

void Base::RegisterSet::deallocateRegisters() {
	registerBanks.clear();
	for(int i = 0; i < 37; i++) delete allRegisters[i];
	delete[] allRegisters;
}

void Base::RegisterSet::setRegisterBank(word mode) {
	switch (mode) {
		case MODE_USER:
		case MODE_SYSTEM:
			currentRegisterBank = registerBanks[BANK_USER_SYSTEM];
			break;
		case MODE_FIQ:
			currentRegisterBank = registerBanks[BANK_FIQ];
			break;
		case MODE_SUPERVISOR:
			currentRegisterBank = registerBanks[BANK_SUPERVISOR];
			break;
		case MODE_ABORT:
			currentRegisterBank = registerBanks[BANK_ABORT];
			break;
		case MODE_IRQ:
			currentRegisterBank = registerBanks[BANK_IRQ];
			break;
		case MODE_UNDEFINED:
			currentRegisterBank = registerBanks[BANK_UNDEFINED];
			break;
		default:
			printf("[ERROR] Unknown mode: %.02X\n", mode);
			break;
	}
	
	currentRegisterBank[CPSR]->data.reg32 &= 0xFFFFFE0;
	currentRegisterBank[CPSR]->data.reg32 |= mode;
}

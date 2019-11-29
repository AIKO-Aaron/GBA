#include "CPU.h"

Base::CPU::CPU() {
	set = new Base::RegisterSet();
	mmu = new Base::MMU();
    timers = new Base::Timers(mmu);
    mmu->timers = timers;
	gpu = new Base::GPU(mmu);
		
	reset();
}

void Base::CPU::reset() {
	set->registerBanks[BANK_SUPERVISOR][LR]->data.reg32 = set->currentRegisterBank[PC]->data.reg32;
	set->registerBanks[BANK_SUPERVISOR][SPSR]->data.reg32 = set->currentRegisterBank[CPSR]->data.reg32;
	
	set->setRegisterBank(MODE_SUPERVISOR);
	set->currentRegisterBank[CPSR]->data.reg32 &= ~FLAG_T;
	set->currentRegisterBank[PC]->data.reg32 = 0;
}

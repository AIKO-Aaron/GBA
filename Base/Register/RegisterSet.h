#pragma once

#include "Register.h"
#include <vector>

namespace Base {
	
	class RegisterSet {
	private:
		Register **allRegisters = nullptr;
		
		void allocateRegisters();
		void deallocateRegisters();
		
	public:
		std::vector<std::vector<Register*>> registerBanks;
		std::vector<Register*> currentRegisterBank;
		
		RegisterSet();
		~RegisterSet();
		
		RegisterSet(RegisterSet &cpy);
		
		void setRegisterBank(word mode);
		inline Register &operator[](word index) { return *currentRegisterBank[index]; }
	};
	
}

#pragma once

#include "GBABase.h"
#include "GBADecompiler.h"

namespace Debugger {
	
	class Debugger {
	protected:
        
	public:
        Debugger();
        Debugger(Base::CPU *_cpu);
        ~Debugger();

        Base::CPU *cpu = nullptr;
        
        word fetchNextInstruction();
        virtual void executeNextInstruction(bool disass = true) = 0;
        virtual void printTrace() = 0;
        void printRegisters();
	};
	
}

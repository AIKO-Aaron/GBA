#pragma once

#include "../Register/RegisterSet.h"
#include "../MMU/MMU.h"
#include "../Timers/Timers.h"
#include "../GPU/GPU.h"

namespace Base {
	
	class CPU {
	public:
        RegisterSet *set = nullptr;
        
        MMU *mmu = nullptr;
        Timers *timers = nullptr;
		GPU *gpu = nullptr;
		uint32_t cycle_count = 0;

	public:
		CPU();
		
		void reset();

        inline void update_cycles(int num) { timers->add_cycles(num); gpu->update_cycles(num); }

        inline void update_gpu() { 
			gpu->update();
            timers->update();

			if(pc().data.reg32 == 0x0000013C) {

				printf("Returned from interrupt to: %.08X\n", reg(LR).data.reg32);
			}
            
			bool interrupts_enabled = r8(0x04000208) & 1;
			bool irq_enabled = !((*set)[CPSR].data.reg32 & FLAG_I);
			hword _if = r16(0x04000202);
			hword _ie = r16(0x04000200);
			hword interrupts = _if & _ie;

			if(interrupts_enabled && interrupts && irq_enabled) {			
				word old_cpsr = (*set)[CPSR].data.reg32;
				set->setRegisterBank(MODE_IRQ);
				(*set)[LR].data.reg32 = (*set)[PC].data.reg32 + 4; // Return address				
				(*set)[PC].data.reg32 = 0x18;
				(*set)[SPSR].data.reg32 = old_cpsr; // Clear T bit
				(*set)[CPSR].data.reg32 &= 0xFFFFFF40 | MODE_IRQ;
				(*set)[CPSR].data.reg32 |= FLAG_I | FLAG_F;

				printf("Interrupt occured: %.04X\n", interrupts);

				mmu->halted = false;
			}
		}

		inline void render() { gpu->render(this); }
        
        inline Register &pc() { return (*set)[PC]; }
        inline Register &reg(byte reg) { return (*set)[reg]; }
		
		inline byte r8(word address) { return mmu->r8(address); }
		inline hword r16(word address) { return mmu->r16(address); }
		inline word r32(word address) { return mmu->r32(address); }
		
		inline byte r8(Register* address) { return mmu->r8(address->data.reg32); }
		inline hword r16(Register* address) { return mmu->r16(address->data.reg32); }
		inline word r32(Register* address) { return mmu->r32(address->data.reg32); }

		inline byte r8(Register address) { return mmu->r8(address.data.reg32); }
		inline hword r16(Register address) { return mmu->r16(address.data.reg32); }
		inline word r32(Register address) { return mmu->r32(address.data.reg32); }
        
        inline void w8(word address, byte val) { mmu->w8(address, val); }
        inline void w16(word address, hword val) { mmu->w16(address, val); }
        inline void w32(word address, word val) { mmu->w32(address, val); }
        
        inline void w8(Register* address, byte val) { mmu->w8(address->data.reg32, val); }
        inline void w16(Register* address, hword val) { mmu->w16(address->data.reg32, val); }
        inline void w32(Register* address, word val) { mmu->w32(address->data.reg32, val); }
        
        inline void w8(Register address, byte val) { mmu->w8(address.data.reg32, val); }
        inline void w16(Register address, hword val) { mmu->w16(address.data.reg32, val); }
        inline void w32(Register address, word val) { mmu->w32(address.data.reg32, val); }
		
	};
	
}

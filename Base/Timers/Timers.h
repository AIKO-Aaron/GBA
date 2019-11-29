#pragma once

#include "../MMU/MMU.h"

namespace Base {
    
    class Timers {
    private:
        MMU *mmu;
        
        word t0 = 0, t1 = 0, t2 = 0, t3 = 0;
    public:
        Timers(MMU* m) : mmu(m) {}
        
        word t0_reload = 0, t1_reload = 0, r2_reload = 0, t3_reload = 0;
        
        void add_cycles(int num) {
            if(mmu->r8(0x4000102) & 0x80) t0 += num;
            if(mmu->r8(0x4000106) & 0x80) t1 += num;
            if(mmu->r8(0x400010A) & 0x80) t2 += num;
            if(mmu->r8(0x400010E) & 0x80) t3 += num;
        }
        void update();
    };
    
}

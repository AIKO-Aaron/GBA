#include "Timers.h"

void Base::Timers::update() {
    byte tm3_cnt = mmu->r8(0x0400010E);
    word prescalar = (tm3_cnt & 0x3) == 0 ? 1 : (tm3_cnt & 0x3) == 1 ? 64 : (tm3_cnt & 0x3) == 2 ? 256 : 1024;
    if(tm3_cnt & 0x04) printf("Count up timing... WTF is this????\n");
    if(tm3_cnt & 0x40) printf("Timer IRQ is enabled....\n");
    
    if(t3 >= prescalar) {
        t3 -= prescalar;
        hword data = mmu->r16(0x0400010C);
        if(++data == 0) {
            printf("Oh wow, an overflow...\n");
            data = t3_reload;
        }
        mmu->w16(0x0400010C, data);
    }
}

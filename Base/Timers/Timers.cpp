#include "Timers.h"

void Base::Timers::update() {
    byte tm0_cnt = mmu->r8(0x04000102);
    if (tm0_cnt & 0x80) {
        word tm0_prescalar = (tm0_cnt & 0x3) == 0 ? 1 : (tm0_cnt & 0x3) == 1 ? 64 : (tm0_cnt & 0x3) == 2 ? 256 : 1024;

        if (tm0_cnt & 0x04) printf("Count up timing... WTF is this????\n");
        if (tm0_cnt & 0x40) printf("Timer IRQ is enabled....\n");

        if (t0 >= tm0_prescalar) {
            t0 -= tm0_prescalar;
            hword data = mmu->r16(0x04000100);
            if (++data == 0) {
                //printf("Counter 0 has overflowed...\n");
                mmu->timer_dma(false);
                data = t0_reload;
            }
            mmu->w16(0x04000100, data);
        }
    }
    else t0 = 0;

    byte tm3_cnt = mmu->r8(0x0400010E);
    if (tm3_cnt & 0x80) {
        word tm3_prescalar = (tm3_cnt & 0x3) == 0 ? 1 : (tm3_cnt & 0x3) == 1 ? 64 : (tm3_cnt & 0x3) == 2 ? 256 : 1024;

        if (tm3_cnt & 0x04) printf("Count up timing... WTF is this????\n");
        if (tm3_cnt & 0x40) printf("Timer IRQ is enabled....\n");

        if (t3 >= tm3_prescalar) {
            t3 -= tm3_prescalar;
            hword data = mmu->r16(0x0400010C);
            if (++data == 0) {
                printf("Oh wow, an overflow...\n");
                data = t3_reload;
            }
            mmu->w16(0x0400010C, data);
        }
    }
    else t3 = 0;
}

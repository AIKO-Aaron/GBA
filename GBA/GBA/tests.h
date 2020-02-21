#pragma once

#include <stdio.h>
#include <assert.h>
#include "GBABase.h"
#include "GBADebugger.h"
#include "GBADecompiler.h"

#define NUM_REGISTERS 17
#define WITH_DISASSEMBLY true

namespace Test {

	typedef union {
		word registers[NUM_REGISTERS];
		struct {
			word r0;
			word r1;
			word r2;
			word r3;
			word r4;
			word r5;
			word r6;
			word r7;
			word r8;
			word r9;
			word r10;
			word r11;
			word r12;
			word sp;
			word lr;
			word pc;
			word cpsr;
		} reg;
	} register_test_set;

	typedef struct {
		word instr = 0x00000000;
		word tc_id = 0; // For identification
		register_test_set in_regs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		register_test_set out_regs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0};
		void (*callback)(Base::CPU *_cpu) = nullptr;
		bool disass = false;
	} test_case;

	bool run_tests(std::vector<test_case> tcs);

	void arm_branch_tests();
	void arm_bx_tests();
	void arm_data_processing_tests();
	void arm_ldr_str();
	void arm_stm_ldm();
	void arm_mul();

	void thumb_mov();
	void thumb_add_sub();
	void thumb_cmp();
	void thumb_alu();
	void thumb_bx();
	void thumb_ldr_str();
	void thumb_ldr_special();
	void thumb_lda();
	void thumb_push_pop();
	void thumb_branch();

	void all_tests();

}
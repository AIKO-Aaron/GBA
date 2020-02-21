#include "tests.h"

bool Test::run_tests(std::vector<Test::test_case> tcs) {
	Base::CPU *_cpu = new Base::CPU();
	Debugger::Interpreter *interp = new Debugger::Interpreter(_cpu);

	word* test_memory = (word*) malloc(sizeof(byte) * 4);

	_cpu->mmu->memory[0] = (byte*) test_memory;


	// Everything set up....
	for(test_case tc : tcs) {
		test_memory[0] = tc.instr;
		for(int i = 0; i < NUM_REGISTERS; i++) _cpu->reg(i).data.reg32 = tc.in_regs.registers[i];

		interp->executeNextInstruction(false);

		word changed = 0;
		for(int i = 0; i < NUM_REGISTERS; i++) changed |= (_cpu->reg(i).data.reg32 ^ tc.out_regs.registers[i]) ? (1 << i) : 0;
		
		if(changed) {

			test_memory[0] = tc.instr;
			for(int i = 0; i < NUM_REGISTERS; i++) _cpu->reg(i).data.reg32 = tc.in_regs.registers[i];
			interp->executeNextInstruction(true);

			printf("[TESTS] For Instruction: %.08X:\n", tc.instr);
			printf("[TESTS] Output Registers not what they are supposed to be:\n");
			for(int i = 0; i < NUM_REGISTERS; i++) {
				if(changed & (1 << i)) printf("\tRegister %s (%d) is %.08X instead of %.08X\n", Decompiler::reg_names[i], i, _cpu->reg(i).data.reg32, tc.out_regs.registers[i]);
			}
			return false;
		}
	}

	delete interp;
	return true;
}


void Test::arm_branch_tests() {
	std::vector<Test::test_case> test_cases;

	word branch = 0xEA000000;
	Test::test_case tc;

	// B <off>
	for(int i = 0; i < 0xFF; i++) {
		tc.instr = branch | i;
		tc.out_regs.reg.pc = 0x00000008 + (i << 2);
		test_cases.push_back(tc);
	}

	// B -<off>
	branch = 0xEA800000;
	for(int i = 0; i < 0xFF; i++) {
		tc.instr = branch | i;
		tc.out_regs.reg.pc = 0xFE000008 + (i << 2);
		test_cases.push_back(tc);
	}

	// BL <off>
	tc.in_regs.reg.lr = 0xDEADBABE;
	tc.out_regs.reg.lr = 0x00000004;

	branch = 0xEB000000;
	for(int i = 0; i < 0xFF; i++) {
		tc.instr = branch | i;
		tc.out_regs.reg.pc = 0x00000008 + (i << 2);
		test_cases.push_back(tc);
	}

	if(run_tests(test_cases)) printf("[+] ARM Branch: OK\n");
	else printf("[-] ARM Branch: FAIL\n");
}

void Test::arm_bx_tests() {
	std::vector<Test::test_case> test_cases;
	word bx = 0xE12FFF10;
	Test::test_case tc;

	// BX Rx (to ARM code)
	tc.out_regs.reg.pc = 0xDEADBABE;

	for(int i = 0; i < 15; i++) {
		tc.in_regs.registers[i] = 0xDEADBABE;
		tc.out_regs.registers[i] = 0xDEADBABE;
		tc.instr = bx | i;

		test_cases.push_back(tc);
		
		tc.in_regs.registers[i] = 0;
		tc.out_regs.registers[i] = 0;
	}

	tc.out_regs.reg.pc = 0xDEADC0FE;
	tc.out_regs.reg.cpsr |= FLAG_T;

	for(int i = 0; i < 15; i++) {
		tc.in_regs.registers[i] = 0xDEADC0FF;
		tc.out_regs.registers[i] = 0xDEADC0FF;
		tc.instr = bx | i;

		test_cases.push_back(tc);
		
		tc.in_regs.registers[i] = 0;
		tc.out_regs.registers[i] = 0;
	}

	if(run_tests(test_cases)) printf("[+] ARM BX: OK\n");
	else printf("[-] ARM BX: FAIL\n");
}

void Test::arm_data_processing_tests() {
	std::vector<Test::test_case> test_cases;
	Test::test_case tc;
	word ins = 0xE2000000;

	// No Flags set...

	// AND R0, R0, #XXX
	// AND with immidiate
	tc.in_regs.reg.r0 = 0xFFFFFFFF;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out;

		test_cases.push_back(tc);
	}

	// EOR R0, R0, #XXX
	ins = 0xE2200000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = 0xFFFFFFFF ^ out;

		test_cases.push_back(tc);
	}

	// SUB R0, R0, #XXX
	ins = 0xE2400000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = 0xFFFFFFFF - out;

		test_cases.push_back(tc);
	}

	// RSB R0, R0, #XXX
	ins = 0xE2600000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out - 0xFFFFFFFF;

		test_cases.push_back(tc);
	}

	// ADD R0, R0, #XXX
	ins = 0xE2800000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out + 0xFFFFFFFF;

		test_cases.push_back(tc);
	}

	// ADC R0, R0, #XXX
	tc.in_regs.reg.cpsr |= FLAG_C;
	tc.out_regs.reg.cpsr |= FLAG_C;

	ins = 0xE2A00000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out + 0xFFFFFFFF + 1;

		test_cases.push_back(tc);
	}

	tc.in_regs.reg.cpsr &= ~FLAG_C;
	tc.out_regs.reg.cpsr &= ~FLAG_C;

	// SBC R0, R0, #XXX
	ins = 0xE2C00000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = 0xFFFFFFFF - out - 1;

		test_cases.push_back(tc);
	}

	// RSC R0, R0, #XXX
	ins = 0xE2E00000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out - 0xFFFFFFFF - 1;

		test_cases.push_back(tc);
	}

	// ORR R0, R0, #XXX
	ins = 0xE3800000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out | 0xFFFFFFFF;

		test_cases.push_back(tc);
	}

	// MOV R0, #XXX
	ins = 0xE3A00000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out;

		test_cases.push_back(tc);
	}

	// BIC R0, R0, #XXX
	ins = 0xE3C00000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = 0xFFFFFFFF & ~out;

		test_cases.push_back(tc);
	}

	// MVN R0, #XXX
	ins = 0xE3E00000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = ~out;

		test_cases.push_back(tc);
	}


	// With the S Bit now....
	ins = 0xE2100000;

	// AND R0, R0, #XXX
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out;

		tc.out_regs.reg.cpsr = 0;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!out) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// EOR R0, R0, #XXX
	ins = 0xE2300000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = 0xFFFFFFFF ^ out;

		tc.out_regs.reg.cpsr = 0;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// SUB R0, R0, #XXX
	ins = 0xE2500000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = 0xFFFFFFFF - out;

		tc.out_regs.reg.cpsr = 0;
		if(0xFFFFFFFF >= out) tc.out_regs.reg.cpsr |= FLAG_C;
		if((1 ^ (out >> 31)) && (1 ^ (tc.out_regs.reg.r0 >> 31))) tc.out_regs.reg.cpsr |= FLAG_V;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// RSB R0, R0, #XXX
	ins = 0xE2700000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out - 0xFFFFFFFF;

		tc.out_regs.reg.cpsr = 0;
		if(out >= 0xFFFFFFFF) tc.out_regs.reg.cpsr |= FLAG_C;
		if((1 ^ (out >> 31)) && ((out >> 31) ^ (tc.out_regs.reg.r0 >> 31))) tc.out_regs.reg.cpsr |= FLAG_V;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// ADD R0, R0, #XXX
	ins = 0xE2900000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out + 0xFFFFFFFF;

		tc.out_regs.reg.cpsr = 0;
		if(out >= 1) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!(1 ^ (out >> 31)) && ((out >> 31) ^ (tc.out_regs.reg.r0 >> 31))) tc.out_regs.reg.cpsr |= FLAG_V;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// ADC R0, R0, #XXX
	ins = 0xE2B00000;
	tc.in_regs.reg.cpsr |= FLAG_C;

	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		out++;
		tc.out_regs.reg.r0 = out + 0xFFFFFFFF;

		tc.out_regs.reg.cpsr = 0;
		if(out >= 1) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!(1 ^ (out >> 31)) && ((out >> 31) ^ (tc.out_regs.reg.r0 >> 31))) tc.out_regs.reg.cpsr |= FLAG_V;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}
	tc.in_regs.reg.cpsr &= ~FLAG_C;

	// SBC R0, R0, #XXX
	ins = 0xE2D00000;

	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = 0xFFFFFFFE - out;

		tc.out_regs.reg.cpsr = 0;
		if(0xFFFFFFFF >= out) tc.out_regs.reg.cpsr |= FLAG_C;
		if((1 ^ (out >> 31)) && (1 ^ (tc.out_regs.reg.r0 >> 31))) tc.out_regs.reg.cpsr |= FLAG_V;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}


	if(run_tests(test_cases)) printf("[+] ARM Data Processing: OK\n");
	else printf("[-] ARM Data Processing: FAIL\n");
}

void Test::all_tests() {
	Test::arm_branch_tests();
	Test::arm_bx_tests();
	Test::arm_data_processing_tests();
}
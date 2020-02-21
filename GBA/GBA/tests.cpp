#include "tests.h"

#define START_MODULE(name) void Test::name() { std::vector<Test::test_case> test_cases; Test::test_case tc;
#define END_MODULE(name) if(run_tests(test_cases)) printf("[+] %-40s: OK\n", #name); else printf("[-] %-40s: FAIL\n", #name); }

bool Test::run_tests(std::vector<Test::test_case> tcs) {
	Base::CPU *_cpu = new Base::CPU();
	Debugger::Interpreter *interp = new Debugger::Interpreter(_cpu);

	word* test_memory = (word*) malloc(sizeof(byte) * 4);

	_cpu->mmu->memory[0] = (byte*) test_memory;


	// Everything set up....
	for(test_case tc : tcs) {
		test_memory[0] = tc.instr;
		for(int i = 0; i < NUM_REGISTERS; i++) _cpu->reg(i).data.reg32 = tc.in_regs.registers[i];

		interp->executeNextInstruction(tc.disass);

		word changed = 0;
		for(int i = 0; i < NUM_REGISTERS; i++) changed |= (_cpu->reg(i).data.reg32 ^ tc.out_regs.registers[i]) ? (1 << i) : 0;
		
		if(tc.callback) tc.callback(_cpu);
		if(changed) {
			test_memory[0] = tc.instr;
			for(int i = 0; i < NUM_REGISTERS; i++) _cpu->reg(i).data.reg32 = tc.in_regs.registers[i];
			interp->executeNextInstruction(true);


			printf("[TESTS] For Instruction: %.08X: ID: %.08X\n", tc.instr, tc.tc_id);
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

//void Test::arm_branch_tests() {
START_MODULE(arm_branch_tests)
	word branch = 0xEA000000;

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
END_MODULE(arm_branch_tests)

START_MODULE(arm_bx_tests)
	word bx = 0xE12FFF10;

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

END_MODULE(arm_bx_tests)

START_MODULE(arm_data_processing_tests)
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

	// RSC R0, R0, #XXX
	ins = 0xE2F00000;

	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out;

		tc.out_regs.reg.cpsr = 0;
		if(0xFFFFFFFF >= out) tc.out_regs.reg.cpsr |= FLAG_C;
		if((1 ^ (out >> 31)) && (1 ^ (tc.out_regs.reg.r0 >> 31))) tc.out_regs.reg.cpsr |= FLAG_V;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	tc.out_regs.reg.r0 = 0xFFFFFFFF;

	// TST R0, R0, #XXX
	ins = 0xE3100000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		word res = out;

		tc.out_regs.reg.cpsr = 0;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!out) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// TEQ R0, R0, #XXX
	ins = 0xE3300000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		word res = 0xFFFFFFFF ^ out;

		tc.out_regs.reg.cpsr = 0;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!res) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(res & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// CMP R0, R0, #XXX
	ins = 0xE3500000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		word res = 0xFFFFFFFF - out;

		tc.out_regs.reg.cpsr = 0;
		if(0xFFFFFFFF >= out) tc.out_regs.reg.cpsr |= FLAG_C;
		if((1 ^ (out >> 31)) && (1 ^ (res >> 31))) tc.out_regs.reg.cpsr |= FLAG_V;
		if(!res) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(res & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// CMN R0, R0, #XXX
	ins = 0xE3700000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		word res = out + 0xFFFFFFFF;

		tc.out_regs.reg.cpsr = 0;
		if(out >= 1) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!(1 ^ (out >> 31)) && ((out >> 31) ^ (res >> 31))) tc.out_regs.reg.cpsr |= FLAG_V;
		if(!res) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(res & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// ORR R0, R0, #XXX
	ins = 0xE3900000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out | 0xFFFFFFFF;

		tc.out_regs.reg.cpsr = 0;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// MOV R0, #XXX
	ins = 0xE3B00000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = out;

		tc.out_regs.reg.cpsr = 0;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// BIC R0, R0, #XXX
	ins = 0xE3D00000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = 0xFFFFFFFF & ~out;

		tc.out_regs.reg.cpsr = 0;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

	// MVN R0, #XXX
	ins = 0xE3F00000;
	for(int i = 0; i < 0xFFF; i++) {
		tc.instr = ins | i;

		word out = i & 0xFF;
		word s = (i >> 8) * 2;
		if(s) out = (out >> s) | (out << (32 - s));
		tc.out_regs.reg.r0 = ~out;

		tc.out_regs.reg.cpsr = 0;
		if(out & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_C;
		if(!tc.out_regs.reg.r0) tc.out_regs.reg.cpsr |= FLAG_Z;
		if(tc.out_regs.reg.r0 & 0x80000000) tc.out_regs.reg.cpsr |= FLAG_N;

		test_cases.push_back(tc);
	}

END_MODULE(arm_data_processing_tests)

START_MODULE(arm_ldr_str)
	tc.instr = 0xE7C01002;
	tc.in_regs.reg.r0 = 0x02000000; // Address for write
	tc.out_regs.reg.r0 = 0x02000000; // Address for write

	// STRB & LDRB

	for(int i = 0; i < 0xFF; i++) {
		tc.in_regs.reg.r1 = i;
		tc.in_regs.reg.r2 = i;

		tc.out_regs.reg.r1 = i;
		tc.out_regs.reg.r2 = i;

		test_cases.push_back(tc);
	}

	tc.instr = 0xE7D01002;
	for(int i = 0; i < 0xFF; i++) {
		tc.in_regs.reg.r1 = 0;
		tc.in_regs.reg.r2 = i;

		tc.out_regs.reg.r1 = i;
		tc.out_regs.reg.r2 = i;

		tc.tc_id = i;

		test_cases.push_back(tc);
	}

	// STR & LDR

	tc.instr = 0xE7801002;
	for(int i = 0; i < 0xFF; i++) {
		tc.in_regs.reg.r1 = (0xDEADBABE << (i % 32)) | ((0xDEADBABE >> (32 - (i % 32))));
		tc.in_regs.reg.r2 = i * 4;

		tc.out_regs.reg.r1 = (0xDEADBABE << (i % 32)) | ((0xDEADBABE >> (32 - (i % 32))));
		tc.out_regs.reg.r2 = i * 4;

		test_cases.push_back(tc);
	}

	tc.instr = 0xE7901002;
	for(int i = 0; i < 0xFF; i++) {
		tc.in_regs.reg.r1 = 0;
		tc.in_regs.reg.r2 = i * 4;

		tc.out_regs.reg.r1 = (0xDEADBABE << (i % 32)) | ((0xDEADBABE >> (32 - (i % 32))));
		tc.out_regs.reg.r2 = i * 4;

		tc.tc_id = i;

		test_cases.push_back(tc);
	}

	// STRH & LDRH

	tc.instr = 0xE18010B2;
	for(int i = 0; i < 0xFF; i++) {
		tc.in_regs.reg.r1 = ((0xBABE << (i % 16)) | ((0xBABE >> (16 - (i % 16))))) & 0xFFFF;
		tc.in_regs.reg.r2 = i * 2;

		tc.out_regs.reg.r1 = ((0xBABE << (i % 16)) | ((0xBABE >> (16 - (i % 16))))) & 0xFFFF;
		tc.out_regs.reg.r2 = i * 2;

		test_cases.push_back(tc);
	}

	tc.instr = 0xE19010B2;
	for(int i = 0; i < 0xFF; i++) {
		tc.in_regs.reg.r1 = 0;
		tc.in_regs.reg.r2 = i * 2;

		tc.out_regs.reg.r1 = ((0xBABE << (i % 16)) | ((0xBABE >> (16 - (i % 16))))) & 0xFFFF;
		tc.out_regs.reg.r2 = i * 2;

		tc.tc_id = i;

		test_cases.push_back(tc);
	}
END_MODULE(arm_ldr_str)

int data[16];
START_MODULE(arm_stm_ldm)
	tc.in_regs.reg.r0 = 0x02000000;
	tc.out_regs.reg.r0 = 0x02000000;
	data[0] = 0x02000000;

	for(int i = 1; i < 14; i++) {
		word r = (word) rand();
		tc.in_regs.registers[i] = r;
		tc.out_regs.registers[i] = r;
		data[i] = r;
	}

	data[15] = 0xC;

	// Store all registers at the address of r0
	tc.instr = 0xE880FFFF;
	tc.callback = [] (Base::CPU *_cpu) -> void { 
		for(int i = 0; i < 16; i++) {
			if(data[i] != _cpu->r32(0x02000000 + 4*i)) printf("[-] Bug in STM/LDM: Could not load same values as were stored\n");
		}
	};
	test_cases.push_back(tc);
	tc.callback = nullptr;
END_MODULE(arm_stm_ldm)

START_MODULE(arm_mul)

END_MODULE(arm_mul)

START_MODULE(thumb_mov)
	tc.in_regs.reg.cpsr |= FLAG_T;
	tc.out_regs.reg.cpsr |= FLAG_T;
	tc.out_regs.reg.pc = 2;

	tc.in_regs.reg.r0 = 0x00000008;
	tc.in_regs.reg.r1 = 0x00000010;
	tc.in_regs.reg.r2 = 0x00000100;
	tc.in_regs.reg.r3 = 0x00001000;
	tc.in_regs.reg.r4 = 0x00010000;
	tc.in_regs.reg.r5 = 0x00100000;
	tc.in_regs.reg.r6 = 0x01000000;
	tc.in_regs.reg.r7 = 0x10000000;

	tc.out_regs.reg.r0 = 0x00000008;
	tc.out_regs.reg.r1 = 0x00000010;
	tc.out_regs.reg.r2 = 0x00000100;
	tc.out_regs.reg.r3 = 0x00001000;
	tc.out_regs.reg.r4 = 0x00010000;
	tc.out_regs.reg.r5 = 0x00100000;
	tc.out_regs.reg.r6 = 0x01000000;
	tc.out_regs.reg.r7 = 0x10000000;

	for(int i = 0; i < 0x20; i++) { // Immidiate
		for(int rs = 0; rs < 8; rs++) {
			for(int rd = 0; rd < 8; rd++) {
				// LSL
				tc.instr = (i << 6) | (rs << 3) | rd;
				tc.out_regs.registers[rd] = tc.in_regs.registers[rs] << i;
				test_cases.push_back(tc);

				// LSR
				tc.instr |= (1 << 11);
				tc.out_regs.registers[rd] = tc.in_regs.registers[rs] >> i;
				test_cases.push_back(tc);

				// ASR
				tc.instr &= ~(1 << 11);
				tc.instr |= (1 << 12);
				tc.out_regs.registers[rd] = (tc.in_regs.registers[rs] >> i) | ((tc.in_regs.registers[rs] & 0x80000000) ? (0xFFFFFFFF << (32 - i)) : 0);
				test_cases.push_back(tc);

				tc.out_regs.registers[rd] = tc.in_regs.registers[rd];
			}
		}
	}

END_MODULE(thumb_mov)

START_MODULE(thumb_add_sub)
END_MODULE(thumb_add_sub)

START_MODULE(thumb_cmp)
END_MODULE(thumb_cmp)

START_MODULE(thumb_alu)
END_MODULE(thumb_alu)

START_MODULE(thumb_bx)
END_MODULE(thumb_bx)

START_MODULE(thumb_ldr_str)
END_MODULE(thumb_ldr_str)

START_MODULE(thumb_ldr_special)
END_MODULE(thumb_ldr_special)

START_MODULE(thumb_lda)
END_MODULE(thumb_lda)

START_MODULE(thumb_push_pop)
END_MODULE(thumb_push_pop)

START_MODULE(thumb_branch)
END_MODULE(thumb_branch)

void Test::all_tests() {
	printf("[@] Commencing ARM Tests:\n");
	Test::arm_branch_tests();
	Test::arm_bx_tests();
	Test::arm_data_processing_tests();
	Test::arm_ldr_str();
	Test::arm_stm_ldm();
	Test::arm_mul();

	printf("[@] Commencing THUMB Tests:\n");
	Test::thumb_mov();
	Test::thumb_add_sub();
	Test::thumb_cmp();
	Test::thumb_alu();
	Test::thumb_bx();
	Test::thumb_ldr_str();
	Test::thumb_ldr_special();
	Test::thumb_lda();
	Test::thumb_push_pop();
	Test::thumb_branch();
}
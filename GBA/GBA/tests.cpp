#include "tests.h"

#define NOP_THUMB 0xE7FF;

void Test::test_cpu_thumb() {
	Base::CPU *_cpu = new Base::CPU();
	Debugger::Interpreter* interpreter = new Debugger::Interpreter(_cpu);

	int num_thumb_instructions = 10;
	byte* test_memory = (byte*) malloc(sizeof(byte) * 2 * num_thumb_instructions);
	hword* instr = (hword*)test_memory;
	_cpu->mmu->memory[0] = test_memory;
	_cpu->reg(CPSR).data.reg32 |= FLAG_T;
	
	for (int i = 0; i < num_thumb_instructions; i++) instr[i] = NOP_THUMB;

	// thumb_test_unconditional_jump(interpreter, instr);
	// thumb_test_mve_shifted_register(interpreter, instr);
}

void Test::test_cpu_arm() {

}

void Test::thumb_test_unconditional_jump(Debugger::Interpreter* interpreter, hword *instr) {
	for (int offset = 0; offset < 0x800; offset++) {
		interpreter->cpu->reg(PC).data.reg32 = 0; // start at 0
		instr[0] = 0xE000 | offset; // offset max 0x7FF

		interpreter->executeNextInstruction();

		word new_pc = interpreter->cpu->pc().data.reg32;

		if (offset < 0x400) assert(new_pc == 4 + 2 * offset);
		else assert(new_pc == 4 + 2 * ((offset) | 0xFFFFFC00));
	}
}

void Test::thumb_test_move_shifted_register(Debugger::Interpreter* interpreter, hword* instr) {
	// LSL
	hword ci = 0x0001; // source = 0, dest = 1, immidiate value = 0
	interpreter->cpu->reg(R0).data.reg32 = 1;
	for (int i = 0; i < 32; i++) {
		interpreter->cpu->reg(PC).data.reg32 = 0; // start at 0
		instr[0] = ci | (i << 6);
		interpreter->executeNextInstruction();
		assert(interpreter->cpu->reg(R0).data.reg32 == 1 && interpreter->cpu->reg(R1).data.reg32 == (1 << i));
	}

	// LSR
	ci = 0x0801; // source = 0, dest = 1, immidiate value = 0
	interpreter->cpu->reg(R0).data.reg32 = 0x80000000;
	for (int i = 0; i < 32; i++) {
		interpreter->cpu->reg(PC).data.reg32 = 0; // start at 0
		instr[0] = ci | (i << 6);
		interpreter->executeNextInstruction();
		assert(interpreter->cpu->reg(R0).data.reg32 == 0x80000000 && interpreter->cpu->reg(R1).data.reg32 == (1 << (31 - i)));
	}

	// ASR
	ci = 0x1001; // source = 0, dest = 1, immidiate value = 0
	interpreter->cpu->reg(R0).data.reg32 = 0xC0000000;
	for (int i = 0; i < 32; i++) {
		interpreter->cpu->reg(PC).data.reg32 = 0; // start at 0
		instr[0] = ci | (i << 6);
		interpreter->executeNextInstruction();
		interpreter->printRegisters();
		assert(interpreter->cpu->reg(R0).data.reg32 == 0xC0000000 && interpreter->cpu->reg(R1).data.reg32 == ((0xC0000000 >> i) | (0xFFFFFFFF << (32 - i))));
	}
}

void Test::thumb_test_add_sub(Debugger::Interpreter* interpreter, hword* instr) {
	for (int i = 0; i < 32; i++) {
		interpreter->cpu->reg(PC).data.reg32 = 0; // start at 0
		instr[0] = 0x1800;
		interpreter->executeNextInstruction();
		assert(interpreter->cpu->reg(R0).data.reg32 == 1 && interpreter->cpu->reg(R1).data.reg32 == (1 << i));
	}
}
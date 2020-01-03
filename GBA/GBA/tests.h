#pragma once

#include <stdio.h>
#include <assert.h>
#include "GBABase.h"
#include "GBADebugger.h"
#include "GBADecompiler.h"

namespace Test {

	void test_cpu_thumb();
	void test_cpu_arm();

	void thumb_test_unconditional_jump(Debugger::Interpreter *i, hword *instr);
	void thumb_test_move_shifted_register(Debugger::Interpreter* interpreter, hword* instr);
	void thumb_test_add_sub(Debugger::Interpreter* interpreter, hword* instr);

}
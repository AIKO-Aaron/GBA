#pragma once

// #define _DEBUG_JUMPS

#ifdef _DEBUG_JUMPS
#  define JUMP_TAKEN(x, y) printf("[DEBUGGER][DEBUG] Jump taken to from %.08X to %.08X\n", x, y)
#else
#  define JUMP_TAKEN(x, y)
#endif

#include "Base/DataTypes.h"
#include "Register/Register.h"
#include "Register/RegisterSet.h"
#include "Base/Logic.h"
#include "MMU/MMU.h"
#include "GPU/GPU.h"
#include "CPU/CPU.h"

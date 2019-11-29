#include "Register.h"

Base::Register::Register() {
	data.reg32 = 0;
}

Base::Register::Register(word initValue) {
	data.reg32 = initValue;
}

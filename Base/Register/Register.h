#pragma once

#include "../Base/DataTypes.h"

namespace Base {
	
	union RegisterData {
		word reg32;
		hword reg16[2];
		byte reg8[4];
	};
	
	class Register {
	public:
		RegisterData data;

	public:
		Register();
		Register(word initValue);
		
		~Register() {}
		Register(Register &r) { data.reg32 = r.data.reg32; }
		inline void operator=(Register &r) { data.reg32 = r.data.reg32; }

		inline void print() { printf("[INFO] Register contents: 0x%.08X\n", data.reg32); }
		
		inline void operator=(word value) { data.reg32 = value; }
		
		inline void operator &=(word value) { data.reg32 &= value; }
		inline void operator |=(word value) { data.reg32 |= value; }
		inline void operator +=(word value) { data.reg32 += value; }
		inline void operator -=(word value) { data.reg32 -= value; }
		inline void operator *=(word value) { data.reg32 *= value; }
		inline void operator /=(word value) { data.reg32 /= value; }
		
		inline word operator &(word value) { return data.reg32 & value; }
		inline word operator |(word value) { return data.reg32 | value; }
		inline word operator +(word value) { return data.reg32 + value; }
		inline word operator -(word value) { return data.reg32 - value; }
		inline word operator *(word value) { return data.reg32 * value; }
		inline word operator /(word value) { return data.reg32 / value; }
        
        inline void operator &=(Register value) { data.reg32 &= value.data.reg32; }
        inline void operator |=(Register value) { data.reg32 |= value.data.reg32; }
        inline void operator +=(Register value) { data.reg32 += value.data.reg32; }
        inline void operator -=(Register value) { data.reg32 -= value.data.reg32; }
        inline void operator *=(Register value) { data.reg32 *= value.data.reg32; }
        inline void operator /=(Register value) { data.reg32 /= value.data.reg32; }
        
        inline word operator &(Register value) { return data.reg32 & value.data.reg32; }
        inline word operator |(Register value) { return data.reg32 | value.data.reg32; }
        inline word operator +(Register value) { return data.reg32 + value.data.reg32; }
        inline word operator -(Register value) { return data.reg32 - value.data.reg32; }
        inline word operator *(Register value) { return data.reg32 * value.data.reg32; }
        inline word operator /(Register value) { return data.reg32 / value.data.reg32; }
	};
	
}

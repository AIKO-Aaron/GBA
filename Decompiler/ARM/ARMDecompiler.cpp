#include "ARMDecompiler.h"

const char** Decompiler::conditions = new const char*[16] {
	"EQ",
	"NE",
	"CS",
	"CC",
	"MI",
	"PL",
	"VS",
	"VC",
	"HI",
	"LS",
	"GE",
	"LT",
	"GT",
	"LE",
	"",
	"NE"
};

const char** Decompiler::reg_names = new const char*[18] {
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "SP",
    "LR",
    "PC",
    "CPSR",
    "SPSR"
};

const char** Decompiler::reg_names_with_constant_length = new const char*[18] {
    "  R0",
    "  R1",
    "  R2",
    "  R3",
    "  R4",
    "  R5",
    "  R6",
    "  R7",
    "  R8",
    "  R9",
    " R10",
    " R11",
    " R12",
    "  SP",
    "  LR",
    "  PC",
    "CPSR",
    "SPSR"
};

const char *chars = new char[16] {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
std::string Decompiler::int_to_hex(word num, int str_len) {
    std::string r;
    while(num > 0) {
        r = chars[num % 16] + r;
        num /= 16;
    }
    while(r.length() < (size_t) str_len) r = '0' + r;
    return r;
}

std::string Decompiler::decompileARM(word instruction, Base::CPU *cpu, bool print) {
	if(print) printf("[DECOMPILER][%.08X] > %.08X ==> ", cpu->pc().data.reg32 - 4, instruction);
    // Condition = conditions[(instruction >> 28) & 0xF]
    std::string disassembled = "";
    
	if (instruction & 0x08000000) {
		if (instruction & 0x04000000) {
			if ((instruction & 0x0F000000) == 0x0F000000) {
                disassembled = "SWI";
			}
			else {
				if (instruction & 0x02000000) {
					if (instruction & 0x00000010) {
						//printf("CoProcessor Register transfer");
					}
					else {
						//printf("CoProcessor Data operation");
					}
				}
				else {
					//printf("CoProcessor Data transfer");
				}
			}
		}
		else {
			if (instruction & 0x02000000) {
                disassembled = "B";
                if(instruction & 0x01000000) disassembled = disassembled + "L";
                disassembled = disassembled + conditions[(instruction >> 28) & 0xF];
                word addr = (instruction & 0x00FFFFFF) << 2;
                if(addr & 0x02000000) addr |= 0xFC000000;
                addr += cpu->pc().data.reg32 + 4;
                disassembled = disassembled + " " + int_to_hex(addr);
			}
			else {
                if(instruction & 0x00100000) disassembled = "LDM";
                else disassembled = "STM";
                
                if((instruction & 0x01800000) == 0x00800000) disassembled = disassembled + "IA";
                else if((instruction & 0x01800000) == 0x01800000) disassembled = disassembled + "IB";
                else if((instruction & 0x01800000) == 0x00000000) disassembled = disassembled + "DA";
                else if((instruction & 0x01800000) == 0x01000000) disassembled = disassembled + "DB";

                disassembled = disassembled + conditions[(instruction >> 28) & 0xF];
                
                disassembled = disassembled + " " + reg_names[(instruction >> 16) & 0xF] + ", {";

                for(int i = 0; i < 15; i++) if(instruction & (1 << i)) disassembled = disassembled + reg_names[i] + (((instruction & 0xFFFF) >> (i+1) ) ? ", " : "");
                disassembled = disassembled + "}";
			}
		}
	}
	else {
		if (instruction & 0x04000000) {
            if ((instruction & 0x02000010) == 0x02000010) {
				disassembled = "Undefined";
			}
			else {
                if(instruction & 0x00100000) disassembled = "LDR";
                else disassembled = "STR";
                if(instruction & 0x00400000) disassembled = disassembled + "B";
                if(instruction & 0x00200000) disassembled = disassembled + "W";
                disassembled = disassembled + conditions[(instruction >> 28) & 0xF];
                
                disassembled = disassembled + " " + reg_names[(instruction >> 12) & 0xF] + ", ";
                disassembled = disassembled + "[" + reg_names[(instruction >> 16) & 0xF] + " ";
                if(!(instruction & 0x02000000)) disassembled = disassembled + "#" + int_to_hex(instruction & 0xFFF, 3) + "]";
                else {
                    const char *shift = (instruction & 0x40) ? (instruction & 0x20 ? "ROR" : "ASR") : (instruction & 0x20 ? "LSR" : "LSL");
                    if(instruction & 0x10) { // Shift by reg
                        disassembled = disassembled + shift + " " + reg_names[(instruction >> 8) & 0xF] + "]";
                    } else { // Shift by immidiate
                        word amt = (instruction >> 7) & 0x1F;
                        if(amt) {
                            disassembled = disassembled + shift + " " + int_to_hex(amt, 8);
                        } else disassembled = disassembled + "]";
                    }
                }
			}
		}
		else {
			if ((instruction & 0x020000F0) == 0x00000090) {
				if (instruction & 0x01000000) {
                    disassembled = "SWP";
                    disassembled = disassembled + conditions[(instruction >> 28) & 0xF];
                    if(instruction & 0x00400000) disassembled += "B";
                    disassembled = disassembled + " " + reg_names[(instruction >> 12) & 0xF] + ", " + reg_names[(instruction >> 0) & 0xF] + ", [" + reg_names[(instruction >> 16) & 0xF] + "]";
				}
				else if (instruction & 0x00800000) {
					if(instruction & 0x00200000) disassembled = (instruction & 0x00400000) ? "SMLAL" : "UMLAL";
                    else disassembled = (instruction & 0x00400000) ? "SMULL" : "UMULL";
                    if(instruction & (1 << 20)) disassembled += "S ";
                    disassembled += reg_names[(instruction >> 12) & 0xF] + std::string(", ") + reg_names[(instruction >> 16) & 0xF] + ", ";
                    disassembled += reg_names[(instruction & 0xF)] + std::string(", ") + reg_names[(instruction >> 8) & 0xF];
				}
				else {
                    if(instruction & 0x00200000) disassembled = (instruction & 0x00400000) ? "MLAS" : "MLA";
                    else disassembled = (instruction & 0x00100000) ? "MULS" : "MUL";

                    if(instruction & 0x00200000) disassembled = disassembled + reg_names[(instruction >> 16) & 0xF] + ", " + reg_names[(instruction & 0xF)] + ", " + reg_names[(instruction >> 8) & 0xF];
                    else disassembled = disassembled + reg_names[(instruction >> 16) & 0xF] + ", " + reg_names[(instruction & 0xF)] + ", " + reg_names[(instruction >> 8) & 0xF] + ", " + reg_names[(instruction >> 12) & 0xF];
 				}
			}
			else if ((instruction & 0x020000F0) == 0x00000090) {
                uint32_t offset = 0;
				if (instruction & 0x00400000) offset = (instruction & 0xF) | ((instruction & 0xF00) >> 4);
				else offset = cpu->reg(instruction & 0xF).data.reg32;
                uint8_t op = (instruction >> 4) & 0xF;


                disassembled = instruction & (1 << 20) ? "LDR" : "STR";
                disassembled += instruction & (op == 2) ? "SB" : op == 3 ? "SH" : "H";

                disassembled += instruction & (1 << 21) ? "W" : "";
                disassembled += std::string(" ") + reg_names[(instruction >> 12) & 0xF] + ", [" + reg_names[(instruction >> 16) & 0xF] + ", ";
                disassembled += instruction & (1 << 23) ? "" : "-";
                disassembled += int_to_hex(offset) + "]";
			}
            else if ((instruction & 0x020000B0) == 0x000000B0) {
                disassembled = (instruction & 0x00100000) ? "LDRS" : "STRS";
                disassembled = disassembled + ((instruction & 0x00000020) ? "H" : "B");
                //printf("(NYI)");

                bool pre = instruction & 0x01000000;
                bool up = instruction & 0x00800000;
                bool writeback = (instruction & 0x00200000) || !pre;
                bool sig = instruction & 0x40;
                bool half = instruction & 0x20;

                disassembled += std::string(" ") + reg_names[(instruction >> 12) & 0xF] + ", [" + reg_names[(instruction >> 16) & 0xF] + ", ";


                if(!up) disassembled += "-";
                if (instruction & 0x00400000) disassembled += int_to_hex((instruction & 0xF) | (((instruction >> 8) & 0xF) << 4));
                else disassembled += reg_names[(instruction & 0xF)];
                disassembled += "]";
			}
			else if ((instruction & 0x0FFFFFF0) == 0x012FFF10) {
				disassembled = std::string("BX ") + reg_names[instruction & 0xF];
            } else if((instruction & 0x0F800000) == 0x01000000 && (instruction & 0x003F0000) == 0x000F0000 && (instruction & 0x00000FFF) == 0x00000000) {
                //printf("MRS");
                disassembled = "MRS";
                disassembled = disassembled + conditions[(instruction >> 28) & 0xF];
            } else if((instruction & 0x0F800000) == 0x01000000 && (instruction & 0x003FFFF0) == 0x0029F000) {
                if(instruction & (1 << 22)) disassembled = std::string("MSR SPSR, ") + reg_names[instruction & 0xF];
                else disassembled = std::string("MSR CPSR, ") + reg_names[instruction & 0xF];
            } else if((instruction & 0x0D800000) == 0x01000000 && (instruction & 0x003FF000) == 0x0028F000) {
                disassembled = "MSR PSR only";
            }
			else {
                bool imm = instruction & 0x02000000;
                bool s = instruction & 0x00100000;
                byte opcode = (instruction >> 21) & 0xF;
                
                switch (opcode) {
                    case 0x0:
                        disassembled = ("AND");
                        break;
                    case 0x1:
                        disassembled = ("EOR");
                        break;
                    case 0x2:
                        disassembled = ("SUB");
                        break;
                    case 0x3:
                        disassembled = ("RSB");
                        break;
                    case 0x4:
                        disassembled = ("ADD");
                        break;
                    case 0x5:
                        disassembled = ("ADC");
                        break;
                    case 0x6:
                        disassembled = ("SBC");
                        break;
                    case 0x7:
                        disassembled = ("RSC");
                        break;
                    case 0x8:
                        disassembled = ("TST");
                        break;
                    case 0x9:
                        disassembled = ("TEQ");
                        break;
                    case 0xA:
                        disassembled = ("CMP");
                        break;
                    case 0xB:
                        disassembled = ("CMN");
                        break;
                    case 0xC:
                        disassembled = ("ORR");
                        break;
                    case 0xD:
                        disassembled = ("MOV");
                        break;
                    case 0xE:
                        disassembled = ("BIC");
                        break;
                    case 0xF:
                        disassembled = ("MVN");
                        break;
                }
                
                disassembled = disassembled + conditions[(instruction >> 28) & 0xF];
                if(s && (opcode >= 0x8 && opcode <= 0xA)) disassembled = disassembled + "S";
                
                if(opcode <= 0x8 || opcode >= 0xA) disassembled = disassembled + " " + reg_names[(instruction >> 12) & 0xF] + ", ";
                if(opcode != 0xD) disassembled = disassembled + " " + reg_names[(instruction >> 16) & 0xF] + ", ";
                
                if(imm) disassembled = disassembled + " #" + int_to_hex(Base::rotate(instruction & 0xFF, (instruction >> 8) & 0xF, cpu, false), 8);
                else {
                    disassembled = disassembled + "[" + reg_names[instruction & 0xF] + " ";
                    const char *shift = (instruction & 0x40) ? (instruction & 0x20 ? "ROR" : "ASR") : (instruction & 0x20 ? "LSR" : "LSL");
                    if(instruction & 0x10) { // Shift by reg
                        disassembled = disassembled + " " + shift + " " + reg_names[(instruction >> 8) & 0xF] + "]";
                    } else { // Shift by immidiate
                        word amt = (instruction >> 7) & 0x1F;
                        if(amt) {
                            disassembled = disassembled + shift + " #" + int_to_hex(amt, 8) + "]";
                        } else disassembled = disassembled + "]";
                    }
                }
			}
		}
    }
    
    if(print) printf("%s\n", disassembled.c_str());
    return disassembled;
}

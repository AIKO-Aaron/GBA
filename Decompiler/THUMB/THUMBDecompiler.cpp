#include "THUMBDecompiler.h"


std::string Decompiler::decompileTHUMB(hword instruction, Base::CPU *cpu, bool print) {
    if(print) printf("Disassembling %.08X\n", instruction);
    if(print) printf("[DECOMPILER][%.08X] > %.04X ==> ", cpu->pc().data.reg32 - 2, instruction);
    
    std::string disassembled = "";

    if(instruction & 0x8000) {
        if(instruction & 0x4000) {
            if(instruction & 0x2000) {
                if(instruction & 0x1000) {
                    word off = ((instruction & 0x7FF) << 12);
                    if(instruction & 0x400) off |= 0xFF800000;
                    word lr = cpu->reg(PC).data.reg32 + 2 + off;
                    
                    hword p2 = cpu->r16(cpu->pc().data.reg32);
                    lr += (p2 & 0x7FF) << 1;

                    disassembled = std::string("BL #") + int_to_hex(lr);
                } else {
                    word offset = (instruction & 0x7FF) << 1;
                    if(offset & 0x0800) offset |= 0xFFFFF000;
                    disassembled = std::string("B #") + int_to_hex(offset + cpu->pc().data.reg32 + 2);
                }
            } else {
                if(instruction & 0x1000) {
                    if((instruction & 0x0F00) == 0x0F00) {
                        disassembled = std::string("SWI #") + int_to_hex(instruction & 0xFF, 2);
                    } else {
						word offset = (instruction & 0xFF) << 1;
						if(offset & 0x100) offset |= 0xFFFFFE00;
                        disassembled = std::string("B") + conditions[(instruction >> 8) & 0xF] + " " + int_to_hex(cpu->pc() + 2 + offset, 2);
                    }
                } else {
                    if(instruction & 0x0800) disassembled = std::string("LDMIA");
                    else disassembled = std::string("STMIA");
                
                    disassembled = disassembled + reg_names[(instruction >> 8) & 0x7] + "!, {";
                    for(int i = 0; i < 8; i++) if(instruction & (1 << i)) disassembled = disassembled + reg_names[i] + ((instruction & ((1 << i) - 1)) ? ", " : "");
                    disassembled = disassembled + "}";
                }
            }
        } else {
            if(instruction & 0x2000) {
                if(instruction & 0x1000) {
                    if(instruction & 0x0400) {
                        if(instruction & 0x0800) disassembled = std::string("POP ");
                        else disassembled = std::string("PUSH ");
                        
                        for(int i = 0; i < 8; i++) if(instruction & (1 << i)) disassembled = disassembled + reg_names[i] + ((instruction & 0x0100) || (((1<<i)-1) & instruction) ? ", " : "");
                        if(instruction & 0x0100) disassembled = disassembled + std::string(instruction & 0x0800 ? "PC" : "LR");
                    } else {
                        word offset = (instruction & 0x7F) << 2;
                        if(instruction & 0x0080) disassembled = std::string("SUB SP, ") + int_to_hex(offset, 3);
                        else disassembled = std::string("ADD SP, ") + int_to_hex(offset, 3);
                    }
                } else {
                    disassembled = std::string("ADR ") + reg_names[(instruction >> 8) & 0x7] + ", " + (instruction & 0x0800 ? "SP" : "PC") + ", " + int_to_hex((instruction & 0xFF) << 2);
                }
            } else {
                if(instruction & 0x1000) {
                    if(instruction & 0x0800) disassembled = std::string("LDR ") + reg_names[(instruction >> 8) & 0x7] + "[SP ";
                    else disassembled = std::string("STR ") + reg_names[(instruction >> 8) & 0x7] + ", [SP ";
                    
                    disassembled = disassembled + int_to_hex((instruction & 0xFF) << 2, 3) + "]";
                    
                } else {
                    if(instruction & 0x0800) disassembled = std::string("LDRH ");
                    else disassembled = std::string("STRH ");
                    disassembled = disassembled + reg_names[instruction & 0x7] + ", [" + reg_names[(instruction >> 3) & 0x7] + ", #" + int_to_hex(((instruction >> 6) & 0x1F) << 1, 2) + "] (" + int_to_hex(cpu->r32((((instruction >> 6) & 0x1F) << 1) + cpu->reg((instruction >> 3) & 0x7).data.reg32), 8) + ")";
                }
            }
        }
    } else {
        if(instruction & 0x4000) {
            if((instruction & 0xFC00) == 0x4000) {
                switch ((instruction >> 6) & 0xF) {
                    case 0x0:
                        disassembled = std::string("AND ");
                        break;
                    case 0x1:
                        disassembled = std::string("EOR ");
                        break;
                    case 0x2:
                        disassembled = std::string("LSL ");
                        break;
                    case 0x3:
                        disassembled = std::string("LSR ");
                        break;
                    case 0x4:
                        disassembled = std::string("ASR ");
                        break;
                    case 0x5:
                        disassembled = std::string("ADC ");
                        break;
                    case 0x6:
                        disassembled = std::string("SBC ");
                        break;
                    case 0x7:
                        disassembled = std::string("ROR ");
                        break;
                    case 0x8:
                        disassembled = std::string("TST ");
                        break;
                    case 0x9:
                        disassembled = std::string("NEG ");
                        break;
                    case 0xA:
                        disassembled = std::string("CMP ");
                        break;
                    case 0xB:
                        disassembled = std::string("CMN ");
                        break;
                    case 0xC:
                        disassembled = std::string("ORR ");
                        break;
                    case 0xD:
                        disassembled = std::string("MUL ");
                        break;
                    case 0xE:
                        disassembled = std::string("BIC ");
                        break;
                    case 0xF:
                        disassembled = std::string("MVN ");
                        break;
                    default:
                        break;
                }
                
                disassembled = disassembled + reg_names[instruction & 0x7] + ", " + reg_names[(instruction >> 3) & 0x7];
            } else if((instruction & 0xFC00) == 0x4400) {
				switch((instruction >> 8) & 3) {
					case 0:
                        disassembled = std::string("ADD ") + reg_names[(instruction & 0x7) | ((instruction >> 4) & 0x8)] + ", " + reg_names[(instruction >> 3) & 0xF];
						break;
					case 1:
                        disassembled = std::string("CMP ") + reg_names[(instruction & 0x7) | ((instruction >> 4) & 0x8)] + ", " + reg_names[(instruction >> 3) & 0xF];
						break;
					case 2:
                        disassembled = std::string("MOV ") + reg_names[(instruction & 0x7) | ((instruction >> 4) & 0x8)] + ", " + reg_names[(instruction >> 3) & 0xF];
						break;
					case 3:
						disassembled = std::string("BX ") + reg_names[(instruction >> 3) & 0xF];
						break;
				}
            } else if((instruction & 0xF800) == 0x4800) {
                word res = cpu->r32(((cpu->pc().data.reg32 + 2) & 0xFFFFFFFC) + ((instruction & 0xFF) << 2));
                disassembled = std::string("LDR ") + reg_names[(instruction >> 8) & 0x7] + ", =" + int_to_hex(res);
            } else if(instruction & 0x2000) {
                //printf("Load/Store with immidiate offset");
                if(instruction & 0x0800) disassembled = std::string("LDR") + (instruction & 0x1000 ? "B" : "");
                else disassembled = std::string("STR") + (instruction & 0x1000 ? "B" : "");

                word off = (instruction >> 6) & 0x1F;
                if(!(instruction & 0x1000)) off <<= 2;

                disassembled = disassembled + " " + reg_names[(instruction & 0x7)] + ", [" + reg_names[(instruction >> 3) & 0x7] + ", #" + int_to_hex(off, 0) + "]";
            } else if(instruction & 0x0200) {
                if((instruction & 0x0C00) == 0x0C00) disassembled = std::string("LDSH ");
                else if(instruction & 0x0800) disassembled = std::string("LDRH ");
                else if(instruction & 0x0400) disassembled = std::string("LDSB ");
                else disassembled = std::string("STRH ");

                disassembled = disassembled + reg_names[(instruction & 0x7)] + ", [" + reg_names[(instruction >> 3) & 0x7] + ", " + reg_names[(instruction >> 6) & 0x7] + "]";
            } else {
                disassembled = std::string(instruction & 0x0800 ? "LDR" : "STR");
                disassembled = disassembled + reg_names[(instruction & 0x7)] + ", [" + reg_names[((instruction >> 3) & 0x7)] + ", " + reg_names[((instruction >> 6) & 0x7)] + "]";
            }
        } else {
            if(instruction & 0x2000) {
                switch((instruction >> 11) & 3) {
                    case 0:
                        disassembled = std::string("MOV");
                        break;
                    case 1:
                        disassembled = std::string("CMP");
                        break;
                    case 2:
                        disassembled = std::string("ADD");
                        break;
                    case 3:
                        disassembled = std::string("SUB");
                        break;
                }
                
                disassembled = disassembled + " " + reg_names[(instruction >> 8) & 0x7] + ", " + int_to_hex((instruction & 0xFF), 2);
            } else {
				switch((instruction >> 11) & 3) {
					case 0:
                        disassembled = std::string("LSLS ");
                        disassembled = disassembled + reg_names[instruction & 0x7] + ", " + reg_names[(instruction >> 3) & 0x7] + ", #" + int_to_hex((instruction >> 6) & 0x1F);
						break;
					case 1:
                        disassembled = std::string("LSRS ");
                        disassembled = disassembled + reg_names[instruction & 0x7] + ", " + reg_names[(instruction >> 3) & 0x7] + ", #" + int_to_hex((instruction >> 6) & 0x1F);
						break;
					case 2:
                        disassembled = std::string("ASRS ");
                        disassembled = disassembled + reg_names[instruction & 0x7] + ", " + reg_names[(instruction >> 3) & 0x7] + ", #" + int_to_hex((instruction >> 6) & 0x1F);
						break;
					case 3:
						disassembled = std::string(instruction & 0x0200 ? "SUBS" : "ADDS");
                        disassembled = disassembled + " " + reg_names[instruction & 0x7] + ", " + reg_names[(instruction >> 3) & 0x7] + ", ";
						if(instruction & 0x0400) disassembled = disassembled + int_to_hex((instruction >> 6) & 0x7, 1);
						else disassembled = disassembled + reg_names[(instruction >> 6) & 0x7];
						break;
				}
            }
        }
    }
    
    if(print) printf("%s\n", disassembled.c_str());
    return disassembled;
}

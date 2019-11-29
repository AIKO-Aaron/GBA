#include "CLI.h"

#ifndef __APPLE__
#  define COLOR_RESET "\033[0m"
#  define COLOR_BOLD "\033[1m"
#  define COLOR_RED "\033[0;31m"
#else
#  define COLOR_RESET ""
#  define COLOR_BOLD ""
#  define COLOR_RED ""
#endif

void printPrompt(Debugger::Debugger *debugger) {
    for(int i = -2; i <= 2; i++) {
        if(i == 0) printf("%s > ", COLOR_BOLD);
        else printf("   ");
        for(int a = 0; a < 2 - abs(i); a++) printf(" ");
        Decompiler::decompileInstruction(debugger->cpu, i * ((debugger->cpu->reg(CPSR).data.reg32 & FLAG_T) ? 2 : 4));
        printf(COLOR_RESET);
    } 
    std::cout << COLOR_BOLD << "[DEBUGGER] > " << COLOR_RESET;
}

void Debugger::start_command_line(Debugger *debugger) {
    std::string user_input, last_prompt;
    char c;

    while(true) {
        printPrompt(debugger);
        c = std::cin.peek();
        if(c == '\n') std::getline(std::cin, user_input);

        std::cin >> user_input;
        last_prompt = user_input;
        c = std::cin.peek();
        const char *str = user_input.c_str();
        
        if(!memcmp(str, "q", 1)) break;
        else if(!memcmp(str, "h", 1)) {
            printf(COLOR_BOLD);
            std::cout << "[DEBUGGER][HELP] Debugger help: <arg=x> are optional and default to x" << std::endl;
            std::cout << "[DEBUGGER][HELP] d <i=1>\t\t\tDisassemble the next i instructions" << std::endl;
            std::cout << "[DEBUGGER][HELP] diff <i=1>\t\t\tExecute next i instructions & Create diff of registers & (TODO) memory" << std::endl;
            std::cout << "[DEBUGGER][HELP] h\t\t\t\tPrint this message" << std::endl;
            std::cout << "[DEBUGGER][HELP] n <i=1>\t\t\tExecute & disassemble the next i instructions" << std::endl;
            std::cout << "[DEBUGGER][HELP] q\t\t\t\tQuit the debugger" << std::endl;
            std::cout << "[DEBUGGER][HELP] r addr\t\t\t\tRead a word at an address (hex)" << std::endl;
            std::cout << "[DEBUGGER][HELP] rb addr\t\t\t\tRead a byte at an address (hex)" << std::endl;
            std::cout << "[DEBUGGER][HELP] reg\t\t\t\tPrint the registers" << std::endl;
            std::cout << "[DEBUGGER][HELP] u addr\t\t\t\tRun until the address (hex) is reached" << std::endl;
            printf(COLOR_RESET);
        }
        else if(!memcmp(str, "diff", 1)) {
            word *old_regs = new word[17];
            for(int i = 0; i < 18; i++) old_regs[i] = debugger->cpu->reg(i).data.reg32;

            int num = 1;
            if(c != '\n') std::cin >> num;
            for(int i = 0; i < num; i++) debugger->executeNextInstruction();

            printf("[DEBUGGER] ");
            for(int i = 0; i < 18; i++) {
                if(old_regs[i] != debugger->cpu->reg(i).data.reg32) printf(COLOR_RED);
                else printf(COLOR_RESET);
                printf("%s (Old = %.08X, New = %.08X)\t", Decompiler::reg_names_with_constant_length[i], old_regs[i], debugger->cpu->reg(i).data.reg32);
                if(i % 3 == 2) printf("%s\n[DEBUGGER] ", COLOR_RESET);
            }
            printf("Memory: ...\n");

            delete[] old_regs;
        }
        else if(!memcmp(str, "d", 1)) {
            int num = 1;
            if(c != '\n') std::cin >> num;
            for(int i = 0; i < num; i++) Decompiler::decompileInstruction(debugger->cpu, i * ((debugger->cpu->reg(CPSR).data.reg32 & FLAG_T) ? 2 : 4));
        }
        else if(!memcmp(str, "n", 1)) {
            int num = 1;
            if(c != '\n') std::cin >> num;
            for(int i = 0; i < num; i++) debugger->executeNextInstruction();
        }
        else if(!memcmp(str, "reg", 1)) {
            debugger->printRegisters();
        }
        else if(!memcmp(str, "rb", 1)) {
            word num = 0;
            if(c != '\n') std::cin >> std::hex >> num;
            printf("[DEBUGGER][MEMORY] Reading memory at 0x%.08X ==> 0x%.08X\n", num, debugger->cpu->r8((word) num));
        }
        else if(!memcmp(str, "r", 1)) {
            word num = 0;
            if(c != '\n') std::cin >> std::hex >> num;
            printf("[DEBUGGER][MEMORY] Reading memory at 0x%.08X ==> 0x%.08X\n", num, debugger->cpu->r32((word) num));
        } else if(!memcmp(str, "u", 1)) {
            int num = debugger->cpu->pc().data.reg32;
            if(c != '\n') std::cin >> std::hex >> num;
            while(debugger->cpu->pc().data.reg32 != num) debugger->executeNextInstruction();
        } else {
            printf("[DEBUGGER] What am I supposed to do with %s?\n", user_input.c_str());
        }
    }
}

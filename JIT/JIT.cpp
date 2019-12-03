#include <iostream>
#include <stdint.h>
#include <sys/mman.h>

typedef int (*jit_code)(int arg);

namespace JIT {

    class Compiler {
    private:
        uint8_t *code = nullptr;
        uint32_t index = 0;

    public:
        Compiler(int max_length);

        void mov_eax_ebp_08();
        void add_r0_1();
        void ret();

        int exec(int arg);
    };
}

JIT::Compiler::Compiler(int max_length) {
    code = (uint8_t*) mmap(0, max_length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(code == (uint8_t*) -1) {
        printf("Error: %d\n", errno);
    }
}

void JIT::Compiler::mov_eax_ebp_08() {
    code[index++] = 0x89;
    code[index++] = 0xD0;
    //code[index++] = 0x18;
}

void JIT::Compiler::add_r0_1() {
    code[index++] = 0x83;
    code[index++] = 0xC0;
    code[index++] = 0x01;
}

void JIT::Compiler::ret() {
    code[index++] = 0xC3;
}

int JIT::Compiler::exec(int arg) {
    return ((jit_code)code)(arg);
}

int main(int argc, char** args) {
    std::cout << args[0] << std::endl;

    JIT::Compiler *c = new JIT::Compiler(4);
    c->mov_eax_ebp_08();
    c->add_r0_1();
    c->ret();

    int ret = c->exec(0xFF);
    printf("Got return vale: %.02X\n", ret);
}
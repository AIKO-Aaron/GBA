#include <iostream>
#include <stdint.h>
#include <sys/mman.h>
#include <cstring>

typedef int (*jit_code)(int arg);

namespace JIT {

    class Compiler {
    private:
        uint8_t *code = nullptr;
        uint32_t index = 0;

    public:
        Compiler(int max_length);
        Compiler(const char *code, int len);

        void mov_eax_ebp_08();
        void add_rax_1();
        void ret();

        inline void push_rax() { code[index++] = 0x50; }
        inline void push_rbx() { code[index++] = 0x53; }
        inline void push_rcx() { code[index++] = 0x51; }
        inline void push_rdx() { code[index++] = 0x52; }

        inline void pop_rax() { code[index++] = 0x58; }
        inline void pop_rbx() { code[index++] = 0x5B; }
        inline void pop_rcx() { code[index++] = 0x59; }
        inline void pop_rdx() { code[index++] = 0x5A; }

        inline void xor_rbx_rbx() { code[index++] = 0x48; code[index++] = 0x31; code[index++] = 0xDB; }

        inline void add_rbx_rax() { code[index++] = 0x48; code[index++] = 0x01; code[index++] = 0xC3; }

        inline void jnz(char amt) { code[index++] = 0x75; code[index++] = 0xfe - amt; }

        inline void mov_rcx_rax() { code[index++] = 0x48; code[index++] = 0x89; code[index++] = 0xC1; }
        inline void mov_rax_rbx() { code[index++] = 0x48; code[index++] = 0x89; code[index++] = 0xD8; }

        inline void dec_rcx() { code[index++] = 0x48; code[index++] = 0x83; code[index++] = 0xE9; code[index++] = 0x01; }

        int exec(int arg);
    };
}

JIT::Compiler::Compiler(int max_length) {
    code = (uint8_t*) mmap(0, max_length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(code == (uint8_t*) -1) {
        printf("Error: %d\n", errno);
    }
}

JIT::Compiler::Compiler(const char *c, int len) {
    code = (uint8_t*) mmap(0, len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(code == (uint8_t*) -1) {
        printf("Error: %d\n", errno);
    }
    memcpy(code, c, len);
}

void JIT::Compiler::mov_eax_ebp_08() {
    code[index++] = 0x89;
    code[index++] = 0xD0;
    //code[index++] = 0x18;
}

void JIT::Compiler::add_rax_1() {
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

const char *code = "\x53\x51\x48\x89\xC1\x48\x89\xC3\x48\x83\xE9\x01\x74\x05\x48\x01\xD8\xEB\xF5\x59\x5B\xC3";

int main(int argc, char** args) {
    /*JIT::Compiler *c = new JIT::Compiler(100);
    c->push_rbx();
    c->push_rcx();
    
    c->mov_rcx_rax();
    c->xor_rbx_rbx();

    c->add_rbx_rax();
    c->dec_rcx();
    c->jnz(7);
    
    c->mov_rax_rbx();
    c->pop_rcx();
    c->pop_rbx();
    c->ret();*/

    JIT::Compiler *c = new JIT::Compiler(code, 23);
    int ret = c->exec(16);
    printf("Got return value: %.08X\n", ret);

    delete c;
}
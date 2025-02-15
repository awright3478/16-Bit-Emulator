#include <stdio.h>
#include <assert.h>

#include "lab1.h"

/* these functions are already provided in emulate.c
 */
extern void store2(struct cpu *cpu, uint16_t data, uint16_t addr);
extern uint16_t load2(struct cpu *cpu, uint16_t addr);

/* set all registers, flags, and memory to zero
 */
void zerocpu(struct cpu *cpu)
{
    cpu->Z = cpu->N = 0;
    cpu->PC = cpu->SP = 0;
    for (int i = 0; i < 8; i++)
        cpu->R[i] = 0;
    memset(cpu->memory, 0, 64*1024);
}

/* 0000 : 5073      : ADD R3 + R6 -> R1
 */
void test1(struct cpu *cpu)
{
    zerocpu(cpu);
    cpu->R[3] = 5;
    cpu->R[6] = 10;
    store2(cpu, 0x5073, 0);

    int val = emulate(cpu);
    assert(val == 0);
    //assert(cpu->R[1] == 15);
}

/* 0002 : 3402 0008 : STORE.B R2 -> *0x0008
 */
void test2(struct cpu *cpu)
{
    zerocpu(cpu);
    cpu->PC = 0;
    cpu->R[2] = 0x1234;

    uint16_t program[] = {
        0x3402, 0x0008, 0xF000
    };
    for(int i = 0; i < sizeof(program)/2; i++){
        store2(cpu, program[i], i*2);
    }


    int val = emulate(cpu);
    assert(val == 0);
    assert(cpu->R[2] == 0x1234);
    assert(cpu->memory[0x0007] == 0);
    assert(cpu->memory[0x0008] == 0x34);
    assert(cpu->memory[0x0009] == 0);
}

struct cpu cpu;

int main(int argc, char **argv)
{
    test1(&cpu);
    test2(&cpu);

    printf("all tests PASS\n");
}

/*
 * file:        emulate-soln.c
 * description: solution to Lab 1
 */
//Andrew Wright

#include <stdio.h>
#include <stdlib.h>

#include "lab1.h"

void store2(struct cpu *cpu, uint16_t data, uint16_t addr) {
    cpu->memory[addr] = data & 0xFF;
    cpu->memory[addr+1] = (data >> 8) & 0xFF;
}

uint16_t load2(struct cpu *cpu, uint16_t addr) {
    return (cpu->memory[addr] | (cpu->memory[addr+1] << 8));
}
void handleFlags(struct cpu *cpu, uint16_t val) {
    cpu->Z = (val == 0);
    cpu->N = (val & 0x8000) != 0;
}

bool check_jmp_cond(struct cpu *cpu, uint16_t cond) {
    bool jump = false;
    if (cond == 0) {
        jump = true;
    }
    else if (cond == 1) {
        jump = cpu->Z == 1;
    }
    else if (cond == 2) {
        jump = cpu->N == 1;
    }
    else if (cond == 3) {
        jump = cpu->N == 0 && cpu->Z == 0;
    }
    else if (cond == 4) {
        jump = cpu->N == 1 || cpu->Z == 1;
    }
    else if (cond == 5) {
        jump = cpu->N == 0;
    }
    else {
        return false;
    }
    return jump;
}

/* emulate(struct cpu*) - emulate a single instruction
 *     - returns 1 for halt, 0 for no halt 
 */
bool emulate(struct cpu *cpu)
{
    uint16_t insn = load2(cpu, cpu->PC);
    uint16_t Ra = insn & 0x0007;
    uint16_t Rb = (insn & 0x0038) >> 3;
    uint16_t Rc = (insn & 0x01C0) >> 6;
    bool is_byte = (insn & 0x0400) != 0 ;
    bool is_indirect = (insn & 0x0800) != 0;
    uint16_t loaded = load2(cpu, cpu->PC + 2);
    uint16_t op = (insn & 0xF000);
    uint16_t cond = (insn & 0x0E00) >> 9;
    uint16_t addr;
    switch(op){
        case 0x1000: { //Set
            cpu->R[Ra] = loaded;
            cpu->PC += 4;
            return false;
        }
        case 0x2000: {//LOAD
            if (is_indirect){
                addr = cpu->R[Rb];
                cpu -> PC +=2;
            } else {
                addr = loaded;
                cpu->PC+= 4;
            }

            if (is_byte) {
                cpu->R[Ra] = cpu->memory[addr];
            } else {
                cpu->R[Ra] = load2(cpu, addr);
            }
            return false;
        }
        case 0x3000: {//STORE
            
            if (is_indirect) {
                addr = cpu->R[Rb];
                cpu->PC+=2;
            } else {
                addr = loaded;
                cpu->PC+=4;
            }
            if(is_byte) {
                cpu->memory[addr] = cpu->R[Ra] & 0xFF;
            } else {
                store2(cpu, cpu->R[Ra], addr);
            }
            return false;  
        }
        case 0x4000: {//MOVE
            uint16_t Rd = (insn & 0x000F);
            uint16_t Rs = (insn & 0x00F0) >> 4;
            uint16_t value;
            if (Rd == 8) {
                value = cpu->R[Rs];
            } else {
                value = cpu->R[Rd];
            }
            if (Rs == 8) {
                cpu->SP = value;
            }
            else {
                cpu->R[Rs] = value;
            }
            cpu->PC+=2;
            return false;
        }
        case 0x5000: { //ALU
            uint16_t ooo = (insn & 0x0E00) >> 9;
            uint16_t res;
            if (ooo == 0){
                res = cpu->R[Ra] + cpu->R[Rb];
                cpu->R[Rc] = res;
                handleFlags(cpu, res);
            }
            else if (ooo == 1){
                res = cpu->R[Ra] - cpu->R[Rb];
                cpu->R[Rc] = res;
                handleFlags(cpu, res);
            }
            else if (ooo == 2){
                res = cpu->R[Ra] & cpu->R[Rb];
                cpu->R[Rc] = res;
                handleFlags(cpu, res);
            }
            else if (ooo == 3) {
                res = cpu->R[Ra] | cpu->R[Rb];
                cpu->R[Rc] = res;
                handleFlags(cpu, res);
            }
            else if (ooo == 4){
                res = cpu->R[Ra] ^ cpu->R[Rb];
                cpu->R[Rc] = res;
                handleFlags(cpu, res);
            }
            else if (ooo == 5){
                res = cpu->R[Ra] >> 1;
                cpu->R[Rc] = res;
                handleFlags(cpu, res);
            }
            else if (ooo == 6){
                res = cpu->R[Ra] - cpu->R[Rb];
                handleFlags(cpu, res);
            }
            else if (ooo == 7){
                handleFlags(cpu, cpu->R[Ra]);
            }
        cpu->PC+=2;
        return false;
        }
        case 0x6000: {//JMP_ABS
            if (check_jmp_cond(cpu, cond)) {
                 uint16_t addr = loaded;
                cpu->PC = addr;
            }
            else {
                cpu->PC+=4;
            }
            return false;
        }
        case 0x7000: {//JMP_IND
            if (check_jmp_cond(cpu, cond)) {
                cpu->PC = cpu->R[Ra];
            } else {
                cpu->PC+=2;
            }
            return false;
        }
        case 0x8000: {//CALL (abs)
            if (cpu->SP < 2) {
                return true;
            }
            cpu->SP-=2;
            store2(cpu, cpu->PC+4, cpu->SP);
            cpu->PC = loaded;
            return false;
        }
        case 0x9000: {//CALL (register indirect)
            if (cpu->SP < 2) {
                return true;
            }
            cpu->SP-=2;
            store2(cpu, cpu->PC + 2, cpu->SP);
            cpu->PC = cpu->R[Ra];
            return false;
        }
        case 0xA000: {//RET
            cpu ->PC = load2(cpu, cpu->SP);
            cpu->SP+=2;
            return false;
        }
        case 0xB000:{ //PUSH
            if (cpu->SP < 2) {
                return true;
            }
            cpu->SP-=2;
            store2(cpu, cpu->R[Ra], cpu->SP);
            cpu->PC+=2;
            return false;
        }
        case 0xC000: {//POP
            cpu->R[Ra] = load2(cpu, cpu->SP);
            cpu->SP+=2;
            cpu->PC+=2;
            return false;
        }
        case 0xD000: {//IN
            cpu->R[Ra] = fgetc(stdin);
            cpu->PC+=2;
            return false;
        }
        case 0xE000: {//OUT
            fputc(cpu->R[Ra], stdout);
            cpu->PC+=2;
            return false;
        }
        case 0xF000:{ //HALT
            return true;
        }    
        return false;
    }
}



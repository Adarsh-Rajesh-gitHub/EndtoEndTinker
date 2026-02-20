#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#define FAIL do {fprintf(stderr, "Simulation error\n"); return 1;} while(0)


//memory of the simulation
uint8_t memory[512*1024] = {0};
uint64_t registers[32];
//31st register is stack ptr

//returns true if mem bad
bool badMem(uint64_t mem, int amtNeeded) {
    return !(mem >= 0x1000 && mem <= 512*1024-(amtNeeded) && mem%4 == 0);
}

int readStrict(uint64_t *out) {
    char buf[256];
    if(scanf("%255s", buf) != 1) return 0; 
    if(buf[0] == '-' || buf[0] == '+') return 0;
    for(size_t i = 0; buf[i]; i++) {
        if(!isdigit((unsigned char)buf[i])) return 0;
    }
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(buf, &end, 10);
    if(errno != 0 || end == NULL || *end != '\0') return 0;

    *out = (uint64_t)v;
    return 1;
}
//check if opcode, registers, label, pc are within bounds, check if insturction is empty in right place
int checkBounds(uint32_t instruction, uint32_t op, uint32_t rd, uint32_t rs, uint32_t rt, uint32_t L, uint64_t *pc) {
    if(rd < 0 || rd > 31 || rs < 0 || rs > 31 || rt < 0 || rt > 31
        || op < 0 || op > 29 || L > 0xFFF || badMem(*pc, 4)) return 1;
            switch(op) {
        case 0: if(L!=0) return 1; break;
        case 1: if(L!=0) return 1; break;
        case 2: if(L!=0) return 1; break;
        case 3: if(rt!= 0 || L!=0) return 1; break;
        case 4: if(L!=0) return 1; break;
        case 5: if(rs!=0 || rt!=0) return 1; break;
        case 6: if(L!=0) return 1; break;
        case 7: if(rs!=0 || rt!=0) return 1; break;
        case 8: if(rs!=0 || rt!=0 || L!=0) return 1; break;
        case 9: if(rs!=0 || rt!=0 || L!=0) return 1; break;
        case 10: if(rd!=0 || rs!=0 || rt!=0) return 1; break;
        case 11: if(rt!= 0 || L!=0) return 1; break;
        case 12: if(L!=0) return 1; break;
        case 13: if(rd!=0 || rs!=0 || rt!=0 || L!=0) return 1; break;
        case 14: if(L!=0) return 1; break;
        case 15: 
            break;
        case 16:if(rt!=0) return 1; break;
        case 17:if(rt!=0 || L!=0) return 1; break;
        case 18:if(rs!=0 || rt!=0) return 1; break;
        case 19:if(rt!=0) return 1; break;
        case 20:if(L!=0) return 1; break;
        case 21:if(L!=0) return 1; break;
        case 22:if(L!=0) return 1; break;
        case 23:if(L!=0) return 1; break;
        case 24: if(L!=0) return 1; break;
        case 25: if(rs != 0 || rt != 0) return 1; break;
        case 26: if(L!=0) return 1; break;
        case 27: if(rs != 0 || rt != 0)return 1; break;
        case 28: if(L!=0) return 1; break;
        case 29: if(L!=0) return 1; break;
        default:
            return 1;
    }        
    return 0;
}


//cpu can operate on same bits between signed and unsigned so only castin necessary(mult and div)
//for floating pt casting changes the bits internally before performing operation
//ex if uint32_t has 00...01 then it wil change it to floating poitn represnation of 1.0 and then do op which mutates orginal val 
//so memcpy used to put it in float type then do op and then memcpy it back into the uint32_t
//memocopy  memcpy, two new operands
int execute(uint32_t instruction, uint64_t *pc) {
    uint32_t op = (instruction >> 27) & 0x1F;
    uint32_t rd = (instruction >> 22) & 0x1F;
    uint32_t rs = (instruction >> 17) & 0x1F;
    uint32_t rt = (instruction >> 12) & 0x1F;
    uint32_t L = (instruction) & 0xFFF;
    uint64_t temp;
    //checking if valid instruction:
    if(checkBounds(instruction, op, rd, rs, rt, L, pc)) {
        return 1;
    }
    //for brr(as only there is label signed)
    int32_t Ls = (int32_t)(L << 20);
    Ls>>=20;
    *pc+=4;
    int64_t address;
    double rss;
    double rtt;
    switch(op) {
        case 0: registers[rd] = registers[rs] & registers[rt]; break;
        case 1: registers[rd] = registers[rs] | registers[rt]; break;
        case 2: registers[rd] = registers[rs] ^ registers[rt]; break;
        case 3: registers[rd] = ~registers[rs]; break;
        case 4: registers[rd] = registers[rs] >> registers[rt]; break;
        case 5: registers[rd] >>= L; break;
        case 6: registers[rd] = registers[rs] << registers[rt]; break;
        case 7: registers[rd] <<= L; break;
        case 8: *pc = registers[rd]; break;
        case 9: *pc += registers[rd]-4; break;
        case 10: *pc+= Ls-4; break;
        case 11: 
            if(registers[rs] != 0)  *pc = registers[rd]; 
            break;
        case 12: //call
            // uint32_t inst;
            // temp = *pc;
            // registers[31]-=8;
            // memcpy(&memory[registers[31]], &temp, sizeof(uint64_t));
            // *pc = registers[rd];
            if(badMem(registers[31]-8, 8)) return 1;
            memcpy(&memory[registers[31]-8], pc, sizeof(uint64_t));
            *pc=registers[rd];
            // memcpy(&inst, &memory[*pc], sizeof(uint32_t));
            // execute(inst, pc);
            break;
        case 13: //return
            // temp;
            // memcpy(&temp, &memory[registers[31]], sizeof(uint64_t));
            // *pc = temp;
            // registers[31] += 8;    uint64_t ret;
            if(badMem(registers[31]-8, 8)) return 1;
            memcpy(pc,&memory[registers[31]-8],8);
            break;
        case 14:
            if((int64_t)registers[rs] > (int64_t)registers[rt]) {
                *pc = registers[rd];
            }
            break;
        case 15: 
            if(L == 0) return 2;
            else if(L == 3) {
                if(registers[rs] == 0) {
                    // uint64_t input;
                    // if(1 != scanf("%llu", &input)) {
                    //     return 1; 
                    // }
                    // registers[rd] = input;
                        uint64_t input;
                        if(!readStrict(&input)) return 1;
                        registers[rd] = input;
                }
            }
            else if(L == 4) {
                    if(registers[rd] == 1) {
                    printf("%llu\n", registers[rs]);
                }
            } 
            else return 1;
            break;
        case 16: 
            address = (int64_t)registers[rs] + (int64_t)Ls;
            if(badMem((uint64_t)address, 8)) return 1;
             memcpy(&registers[rd], &memory[registers[rs] + Ls], sizeof(uint64_t)); break;
            // int64_t addr=(int64_t)registers[rs]+convt(L);
            // if(addr<0||addr+8>(int64_t)sizeof(memory)||(addr&7)) return 1;
            // memcpy(&registers[rd],&memory[addr],8);
            break;
        case 17:
            registers[rd] = registers[rs]; break;
        case 18:
            // temp = 0;
            // temp = registers[rd] & ((1ULL << 52)-1);
            // registers[rd] = 0;
            // registers[rd] = ((uint64_t)L & 0xFFF) << 52;
            // registers[rd] |= temp;
            registers[rd] >>= 12;
            registers[rd] <<= 12;
            registers[rd] |= L;
            break;
        case 19:
            int64_t addresss = (int64_t)registers[rd] + (int64_t)Ls;
            // if(badMem((uint64_t)address, 8)) return 1;
            //if(badMem(registers[rd] + Ls, 8)) return 1;
            if((registers[rd] + Ls)%4 != 0 || addresss < 0x0000) return 1;
            memcpy(&memory[registers[rd] + Ls], &registers[rs], sizeof(uint64_t)); break;
            // int64_t addrr=(int64_t)registers[rd]+convt(L);
            // if(addrr<0||addrr+8>(int64_t)sizeof(memory)||(addrr&7)) return 1;
            // memcpy(&memory[addrr],&registers[rs],8);
            break;
        case 20:
            memcpy(&rss, &registers[rs], sizeof(uint64_t));
            memcpy(&rtt, &registers[rt], sizeof(uint64_t));
            rtt+=rss;
            memcpy(&registers[rd], &rtt, sizeof(double));
            break;
        case 21:
            memcpy(&rss, &registers[rs], sizeof(uint64_t));
            memcpy(&rtt, &registers[rt], sizeof(uint64_t));
            rss-=rtt;
            memcpy(&registers[rd], &rss, sizeof(double));
            break;
        case 22:
            memcpy(&rss, &registers[rs], sizeof(uint64_t));
            memcpy(&rtt, &registers[rt], sizeof(uint64_t));
            rtt*=rss;
            memcpy(&registers[rd], &rtt, sizeof(double));
            break;
        case 23:
            memcpy(&rss, &registers[rs], sizeof(uint64_t));
            memcpy(&rtt, &registers[rt], sizeof(uint64_t));
            if(rtt == 0) return 1;
            rss/=rtt;
            memcpy(&registers[rd], &rss, sizeof(double));
            break;
        case 24: registers[rd] = registers[rs] + registers[rt]; break;
        case 25: registers[rd] = registers[rd] + L; break;
        case 26: registers[rd] = registers[rs] - registers[rt]; break;
        case 27: registers[rd] = registers[rd] - L; break;
        case 28: 
            __int128 prod=(__int128)(int64_t)registers[rs]*(__int128)(int64_t)registers[rt];
            registers[rd]=(uint64_t)prod;
            break;
        case 29: 
            int64_t rttt=(int64_t)registers[rt];
            if(rttt == 0) return 1;
            registers[rd]=(uint64_t)((int64_t)registers[rs]/rttt);
            break;
        default:
            return 1;
    }
    return 0;
}


#ifndef TESTING
int main(int argc, char *argv[]) {

    //file input 
    
    if(argc != 2) {
        fprintf(stderr, "Invalid tinker filepath\n");
        return 1;
    }
    char* inputFile = argv[1];
    FILE *fp = fopen(inputFile, "rb");
    if(!fp) {
        fprintf(stderr, "Invalid tinker filepath\n");
        return 1;
    }   




    //reads in input from file and validation
    uint64_t pc;
    //size_t itemsRead = fread(&memory[pc], sizeof(uint32_t), (sizeof(memory)-pc)/sizeof(uint32_t), fp);
    uint64_t fileType;
    uint64_t codeStart;
    uint64_t codeSize;
    uint64_t dataStart;
    uint64_t dataSize;
    if(fread(&fileType, sizeof(uint64_t), 1, fp) != 1) FAIL;
    if(fread(&codeStart, sizeof(uint64_t), 1, fp) != 1) FAIL;
    if(fread(&codeSize, sizeof(uint64_t), 1, fp) != 1) FAIL;
    if(fread(&dataStart, sizeof(uint64_t), 1, fp) != 1) FAIL;
    if(fread(&dataSize, sizeof(uint64_t), 1, fp) != 1) FAIL;
    if (badMem(codeStart,(int)codeSize)) FAIL;
    if(codeSize % 4 != 0) FAIL;
    if(dataSize % 8 != 0) FAIL;
    if(dataStart % 8 != 0) FAIL;
    if(codeSize > dataStart - codeStart) FAIL;
    if(dataSize > 512*1024-dataStart) FAIL;
    pc = codeStart;
    if(fread(&memory[pc], codeSize, 1, fp) != 1) FAIL;
    if(fread(&memory[dataStart],dataSize, 1, fp) != 1) FAIL;
    registers[31] = sizeof(memory);
    fclose(fp);

    while(1) {
        uint32_t instruction;
        memcpy(&instruction, &memory[pc], sizeof(uint32_t));
        int res = execute(instruction, &pc);

        if(res == 1) {
            fprintf(stderr, "Simulation error\n");
            return 1;
        }
        else if (res == 2) {
            return 0;
        }
        
    }


    return 0;
    




}
#endif
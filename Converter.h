#ifndef CONVERTER_H
#define CONVERTER_H

#include <stdint.h>
#include <stdbool.h>

bool badMem(uint64_t mem,int amtNeeded);
int checkBounds(uint32_t instruction,uint32_t op,uint32_t rd,uint32_t rs,uint32_t rt,uint32_t L,uint64_t *pc);
int readStrict(uint64_t *out);
extern uint8_t memory[512*1024];
extern uint64_t registers[32];
int execute(uint32_t instruction,uint64_t *pc);
#endif
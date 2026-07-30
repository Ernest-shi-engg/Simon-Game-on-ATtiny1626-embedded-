#ifndef LFSR_H
#define LFSR_H

#include <stdint.h>

#define LFSR_MASK (uint32_t)0xE2026E6B
#define LFSR_SEED (uint32_t)0x12838314

void init_lfsr(void);
void reset_lfsr(void);
uint8_t next_lfsr(void);
void advanced_lfsr(void);
void save_lfsr(void);
void set_lfsr_seed(uint32_t seed);

#endif
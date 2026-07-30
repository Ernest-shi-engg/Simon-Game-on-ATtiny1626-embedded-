#include "lfsr.h"

static uint32_t state_lfsr = LFSR_SEED;
static uint32_t round_start = LFSR_SEED;
static uint32_t saved_lfsr = LFSR_SEED;
static uint32_t curr_seed = LFSR_SEED;

void init_lfsr(void) {
    state_lfsr = curr_seed; 
    round_start = curr_seed;
    saved_lfsr = curr_seed;
}

void set_lfsr_seed(uint32_t seed) {
    curr_seed = seed;
    init_lfsr();
}

void reset_lfsr(void) {
    state_lfsr = round_start;
}

void save_lfsr(void) {
    saved_lfsr = state_lfsr; // save while simons fully played
}

void advanced_lfsr(void) {
    round_start = saved_lfsr; // start from saved position after fail
    state_lfsr = round_start;
}

uint8_t next_lfsr(void) {
    uint8_t bit = (uint8_t)(state_lfsr & 1U); //
    state_lfsr >>= 1; // right shift
    if(bit) {
        state_lfsr ^= LFSR_MASK;
    }
    return (uint8_t)(state_lfsr & 0x03U);
}
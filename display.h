#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdint.h>

void display_write(uint8_t data);
void initial_display (void);
void set_display_seg(uint8_t seg_l, uint8_t seg_r);
void swap_disp_digit(void);
void display_off(void);
void display_step(uint8_t step);
void display_success(void);
void display_fail(void);
void display_score(uint16_t score);
#endif

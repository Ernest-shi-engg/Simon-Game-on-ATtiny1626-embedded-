#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void initial_buzzer (void);
void tone_select(uint8_t tone);
void stop_tone(void);
void incre_octave(void);
void decre_octave(void);
void octave_reset(void);

#endif

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void initial_adc(void);
void update_playback_delay(void);
uint8_t adc_read(void);

#endif
#include "adc.h"
#include <stdint.h>
#include <avr/io.h>

#define MIN_PLAYBACK_DELAY 250
#define MAX_PLAYBACK_DELAY 2000

extern uint16_t PLAYBACK_delay;
extern uint16_t half_delay;

void initial_adc (void) {
    // set input potentiometre -- AIN2/PA2
    PORTA_DIRCLR = PIN2_bm;

    // enable ADC
    ADC0_CTRLA = ADC_ENABLE_bm;

    // ADC clock prescalar
    ADC0_CTRLB = ADC_PRESC_DIV2_gc;

    // reference voltage set as VDD
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc;

    ADC0_MUXPOS = ADC_MUXPOS_AIN2_gc; // read AIN2
}

uint8_t adc_read(void) {
    // read PA2 immidiate and convert voltage to digits 0-255
    ADC0_COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;

    // wait result from adc
    while (!(ADC0_INTFLAGS & ADC_RESRDY_bm))
        ;

    ADC0_INTFLAGS = ADC_RESRDY_bm; // clear ready flag

    return (uint8_t)ADC0_RESULT; // return val stored in 0-255
}

// intuition of "ADC - delay" linear model
// x axis as delay(t) & y axis as voltage, linear proportional
// voltage_min = 0, voltage_max = 255 --- for y axis
// delay_min = 250, delay_max = 2000 --- for x axis
// slope = (255-0)/(2000 - 250)
// for any voltage on this linear line can be illsutrate
// as "(adc_val - 0) = slope * (delay_time - 250)" --> delta_y = slope * delta_x
// rerange could get PLAYBACK_delay = 250 + (((uint32_t)adc_val * (2000 - 250)) / 255);
void update_playback_delay(void) {
    uint8_t adc_val = adc_read();

    PLAYBACK_delay = 250 + (((uint32_t)adc_val * (2000 - 250)) >> 8);

    half_delay = PLAYBACK_delay >> 1; // equivalent to divide by 2
}
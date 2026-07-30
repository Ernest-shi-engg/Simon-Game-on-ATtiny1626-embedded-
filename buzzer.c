#include "buzzer.h"
#include <avr/io.h>

int8_t octave = 0;

void initial_buzzer (void) {
    PORTB_OUTSET = PIN0_bm; // disable the buzzer at start, outset due to QUTy active low
    PORTB_DIRSET = PIN0_bm; // direction set portB pin0 as output

    TCA0_SINGLE_CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc; // prescalar = 1

    // set for PWM mode & choosing waveform output WG0
    TCA0_SINGLE_CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;
    
    // duty cycle = 0 set the PWM off at init
    TCA0_SINGLE_PER = 1;
    TCA0_SINGLE_CMP0 = 0;

    TCA0_SINGLE_CTRLA |= TCA_SINGLE_ENABLE_bm; // enable buzzer
}

// E_hi = 414 * 2 ^ ((-5)/12) = 310.15Hz, 3.3Mhz/freq - 1 = 10746
// C# = 414 * 2 ^ ((-8)/12) = 260.8Hz
// A = 414Hz
// E_lo = 414 * 2 ^ ((-17)/12) = 155.07Hz
void tone_select(uint8_t tone) {
    static const uint16_t per_val[4] = {10746, 12780, 8050, 21494}; // tones array

    uint16_t per = per_val[tone];

    if (octave > 0) {
        for (int8_t i = 0; i < octave; i++) {
            per >>= 1; // freq up, period down
        }
    } else if (octave < 0) {
        for (int8_t i = 0; i > octave; i--) {
            per <<= 1; // freq down, period up
        }
    }

    TCA0_SINGLE_PERBUF = per;
    TCA0_SINGLE_CMP0BUF = (per + 1) >> 1; // CMP0 shift 1 bit == divde by 2
}

void incre_octave(void) {
    if (octave < 2) {
        octave++;
    }
}

void decre_octave(void) {
    if (octave > -2) {
        octave--;
    }
}

void octave_reset(void) {
    octave = 0;
}

void stop_tone(void) {
    TCA0.SINGLE.CMP0BUF = 0;
}
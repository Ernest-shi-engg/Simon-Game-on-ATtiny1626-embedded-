#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "display.h"
volatile uint16_t elapsed_t = 0;
volatile uint8_t pb_debounced = 0xFF; // pull up resistor, not pressed = 1
volatile uint8_t pb_falling = 0x00;

// periodic interrupt every 1ms
ISR (TCB0_INT_vect) {
    elapsed_t++; // increment elapsed time for time counting
    TCB0_INTFLAGS = TCB_CAPT_bm; // clear flag
}

// periodic interrupt every 5ms
ISR(TCB1_INT_vect) {
    swap_disp_digit(); // swap RHS & LHS of display every 5ms

    // Confirm a press after 3 stable samples (15 ms), and require a longer
    // stable release to avoid counting bounce as a second press.
    static uint8_t press_count[4] = {0};
    static uint8_t release_count0 = 0x00;
    static uint8_t release_count1 = 0x00;

    // shift pa4-pa7 to LSB 4 bit by '>>4'
    uint8_t pb_sample = (PORTA_IN >> 4) & 0x0F; // read current button status
    uint8_t pb_curr = (pb_debounced >> 4) & 0x0F; // read current button after debounced
    uint8_t press = pb_curr & ~pb_sample;
    uint8_t release = ~pb_curr & pb_sample;

    uint8_t confirmed_press = 0x00;
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t mask = (1 << i);
        if (press & mask) {
            if (press_count[i] < 3) {
                press_count[i]++;
            }
            if (press_count[i] == 3) {
                confirmed_press |= mask;
            }
        } else {
            press_count[i] = 0;
        }
    }

    release_count1 = (release_count1 ^ release_count0) & release;
    release_count0 = ~release_count0 & release;
    uint8_t confirmed_release = release & ~(release_count1 | release_count0);

    uint8_t confirmed = confirmed_press | confirmed_release;
    
    // revised version moving falling edge detection from main() to ISR
    // using "|=" increment the falling edge ever since 5ms interrupt was triggered
    // confirm Hi at previous and Lo at current check falling edege
    uint8_t falling = confirmed_press;
    pb_falling |= (falling << 4); // move LSB 4 back to pa4-pa7 for main loop

    pb_debounced ^= (confirmed << 4); //update pb_debounced
    TCB1_INTFLAGS = TCB_CAPT_bm;
}

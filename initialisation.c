#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include "initialisation.h"

// define all the initial functions

void initial_timer (void) {
    // configure TCB0 for periodic interrupt
    TCB0_CTRLB = TCB_CNTMODE_INT_gc; 
    TCB0_CCMP = 3333; // 3.3Mhz * 1ms = 3333 clocks
    TCB0_INTCTRL = TCB_CAPT_bm; // interrupt enable
    TCB0_CTRLA = TCB_ENABLE_bm; // enable TCB0

    // configure TCB1 for debouncing & swap interrupt
    TCB1_CTRLB = TCB_CNTMODE_INT_gc;
    TCB1_CCMP = 16667; // 3.3MHz * 5ms = 16667 clocks
    TCB1_INTCTRL = TCB_CAPT_bm; // interrupt enable
    TCB1_CTRLA = TCB_ENABLE_bm; // enable TCB1
}

void initial_pb(void) {
    PORTA_DIRCLR = PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm; // button S1 - S4 as input, set as 0
    // enbale all the pull up resistors while buttons not in use
    PORTA_PIN4CTRL = PORT_PULLUPEN_bm; // S1
    PORTA_PIN5CTRL = PORT_PULLUPEN_bm; // S2
    PORTA_PIN6CTRL = PORT_PULLUPEN_bm; // S3
    PORTA_PIN7CTRL = PORT_PULLUPEN_bm; // S4
}
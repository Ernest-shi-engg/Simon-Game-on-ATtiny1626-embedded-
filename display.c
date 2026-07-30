#include "display.h"
#include "display_macros.h"
#include <stdint.h> 
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t digit_l = DISP_OFF; // left disp
volatile uint8_t digit_r = DISP_OFF; // right disp

// RHS for all numbers display
static const uint8_t num_seg[10] = {
0X08, //SEG_ZEROS 
0x6B, //SEG_ONE 
0x44, //SEG_TWO
0x41, //SEG_THREE 
0X23, //SEG_FOUR 
0X11, //SEG_FIVE 
0X10, //SEG_SIX 
0X4B, //SEG_SEVEN 
0X00, //SEG_EIGHT 
0X01 //SEG_NINE 
};


void initial_display (void) {
    // set terminal of ssd as output
    PORTMUX_SPIROUTEA = PORTMUX_SPI0_ALT1_gc; // SPI pins routing on PC0-3
    PORTC_DIRSET = PIN0_bm | PIN2_bm; // CLK & MOSI

    // DISP_LATCH
    PORTA_OUTSET = PIN1_bm; // set initial high (off)
    PORTA_DIRSET = PIN1_bm; // set DISP_LATCH as output
 
    // set slave master peripheral interaction 
    // MCU-master, display chip-slave in quty
    SPI0_CTRLA = SPI_MASTER_bm; // initialise SPI0 as master
    SPI0_CTRLB = SPI_SSD_bm; // disable auto slave selected
    SPI0_INTCTRL = SPI_IE_bm; // interrupt enable
    SPI0_CTRLA |= SPI_ENABLE_bm; // enable SPI; '|=' to ensure previous master_bm not overwrite
}

void display_write(uint8_t data) {
    SPI0_DATA = data;
}

void set_display_seg(uint8_t seg_l, uint8_t seg_r) {
    digit_l = seg_l;
    digit_r = seg_r;
}

void swap_disp_digit(void) {
    static uint8_t digit = 0;

    if (digit) {
        display_write(digit_l | DISP_LHS); // skipped when digit = 0
    } else {
        display_write(digit_r); // execute when digit = 0 and display RHS
    }

    digit = !digit; // inverse digit between 0 & 1 and continue the loop
}

void display_off(void) {
    set_display_seg(DISP_OFF, DISP_OFF);
}

void display_step(uint8_t step) {
    switch (step)
    {
    case 0:
        // S1: LHS E&F
        set_display_seg(DISP_BAR_LEFT, DISP_OFF);
        break;

    case 1:
        // S2: LHS B&C
        set_display_seg(DISP_BAR_RIGHT, DISP_OFF);
        break;

    case 2:
        // S3: RHS E&F
        set_display_seg(DISP_OFF, DISP_BAR_LEFT);
        break;

    case 3:
        // S4: RHS B&C
        set_display_seg(DISP_OFF, DISP_BAR_RIGHT);
        break;

    default:
        display_off();
        break;
    }
}

void display_success(void) {
    // "88" while success
    set_display_seg(DISP_ON, DISP_ON);
}

void display_fail(void) {
    // "--" while fail
    set_display_seg(DISP_DASH, DISP_DASH);
}

void display_score(uint16_t score) {
    uint16_t val = score;
    uint8_t tens = 0;
    uint8_t units = 0;

    // keep only last 2 decimal (>100)
    while (val >= 100) {
        val -= 100;
    }

    while (val >= 10) {
        val -= 10;
        tens++;
    }

    units = (uint8_t)val;

    if (score < 10) {
        set_display_seg(DISP_OFF, num_seg[units]);
    } else {
        set_display_seg(num_seg[tens], num_seg[units]);
    }

}

ISR(SPI0_INT_vect) {
    // creating rising edge, from 0 -> 1
    PORTA_OUTCLR = PIN1_bm; // low
    PORTA_OUTSET = PIN1_bm; // hi

    SPI0_INTFLAGS = SPI_IF_bm; 
}
#include "uart.h"
#include <stdint.h>
#include <avr/io.h>

void initial_uart(void) {
    //PB2 - TXD0, PB3 - RXD0
    PORTB_DIRSET = PIN2_bm; // TX output(transmit)
    PORTB_DIRCLR = PIN3_bm; // RX input(receive)

    USART0_BAUD = 1389; // 9600 BAUD @ 3.33Mhz

    //enable TX & RX
    USART0_CTRLB = USART_RXEN_bm | USART_TXEN_bm;   
}

uint8_t uart_available(void) {
    return (USART0_STATUS & USART_RXCIF_bm);
}

char uart_getc(void) {
    return USART0_RXDATAL;
}
void uart_putc(char c) {
    while(! (USART0_STATUS & USART_DREIF_bm))
        ;
    USART0_TXDATAL = c;
}

void uart_puts(const char *str) {
    while (*str) {
        uart_putc(*str);
        str++;
    }
}

void uart_put_uint16(uint16_t value) {
    static const uint16_t places[5] = {
        10000, 1000, 100, 10, 1
    };

    uint8_t started = 0;

    for (uint8_t i = 0; i < 5; i++) {
        uint8_t digit = 0;

        while (value >= places[i]) {
            value -= places[i];
            digit++;
        }

        if (digit > 0 || started || places[i] == 1) {
            uart_putc('0' + digit);
            started = 1;
        }
    }
}

uartcommand uart_get_command(uint8_t *input) {
    if(!uart_available()) {
        return 0;
    }

    char c = uart_getc();

    switch (c) {
        case '1':
        case 'q':
        *input = 0;
        return UART_GAME_COMMAND;

        case '2':
        case 'w':
        *input = 1;
        return UART_GAME_COMMAND;

        case '3':
        case 'e':
        *input = 2;
        return UART_GAME_COMMAND;

        case '4':
        case 'r':
        *input = 3;
        return UART_GAME_COMMAND;

        case ',':
        case 'k':
        return UART_INC_FREQ;

        case '.':
        case 'l':
        return UART_DEC_FREQ;

        case '0':
        case 'p':
        return UART_RESET;

        case '9':
        case 'o':
        return UART_SEED;

        default:
        return UART_NONE;
    }
}
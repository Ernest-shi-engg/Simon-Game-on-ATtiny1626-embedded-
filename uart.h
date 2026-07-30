#ifndef UART_H
#define UART_H

#include <stdint.h>

typedef enum {
    UART_NONE,
    UART_GAME_COMMAND,
    UART_INC_FREQ,
    UART_DEC_FREQ,
    UART_RESET,
    UART_SEED
} uartcommand;

void initial_uart(void);

uint8_t uart_available(void);
char uart_getc(void);

void uart_putc(char c);
void uart_puts(const char *str);
void uart_put_uint16(uint16_t val);

uartcommand uart_get_command(uint8_t *input);

#endif

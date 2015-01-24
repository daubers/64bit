/*
 * Copyright 2015 <b@Zi.iS>
 */

int uart_putchar(char c, FILE *stream);
char* next_line();

void uart_init(void);

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


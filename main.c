/*
 * 64bit IO
 *
 * © 2015 b@Zi.iS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"

int main(void) {

    uart_init();
    sei();

    stdout = &uart_output;

    DDRA |= _BV(0); //Output

    printf("64bit IO\n");

    while(1) {
        printf("FooBar\n");
        _delay_ms(200);
    }
}

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

#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "main.h"
#include "uart.h"

uint8_t last[8];
uint8_t cur[8];
uint8_t dir[8];
int8_t id = 40;

unsigned char spi(unsigned char data) {
    SPDR = data;
    while(!(SPSR & (1<<SPIF)));
    return SPDR;
}

int main(void) {

    OUTPUT(MOSI);
    OUTPUT(SCK);
    OUTPUT(RS485_DIR);
    OUTPUT(BLANK);
    OUTPUT(LATCH);

    HIGH(BLANK); //High till we have clocked data

    uart_init(42);

    SPCR |= _BV(SPE) | _BV(MSTR) | _BV(SPR0); //SPI, Master, fck/16

    sei();

    for(uint8_t i = 0; i<8; i++) {
        spi(0); //disable all inputs and outputs
    }
    HIGH(LATCH);
    _delay_ms(1); //>20ns
    LOW(LATCH);

    while(true) {
        uint8_t next;

        HIGH(LATCH); //Start outputing and testing inputs
        for(uint8_t i = 0; i<8; i++) {
            spi(cur[1]); //clock out outputs
            if(i == 1) { //>20ns needed for latch
                LOW(LATCH);
            }
            if(i == 6) { //detect >1µs after previous latch and >20ns before next
                HIGH(BLANK); //Sore inputs and stop outputing
            }
        }
        HIGH(LATCH); //Start driving just outputs, copy input states into shift register
        LOW(BLANK); //Start outputing

        for(uint8_t i = 0; i<8; i++) {
            if(i == 1) { //>20ns needed for latch
                LOW(LATCH);
            }
            next = spi(cur[i] | dir[i]) & dir[i]; // clock out both outputs and inputs, clock in intputs
            if(last[i] != next) {
                last[i] = next;
                uart_send(i, last[i]);
            }
        }

        uart_poll_clear_to_send();
    }
}

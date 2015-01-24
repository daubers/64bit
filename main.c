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
#include "main.h"
#include "uart.h"

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

    uart_init();
    stdout = &uart_output;

    SPCR |= _BV(SPE) | _BV(MSTR) | _BV(SPR0); //SPI, Master, fck/16

    sei();

    //Clear Outputs
    spi(0x00);
    spi(0x00);
    spi(0x00);
    spi(0x00);
    spi(0x00);
    spi(0x00);
    spi(0x00);
    spi(0x00);
    HIGH(LATCH);
    LOW(BLANK);
    _delay_ms(1); //LOD detection
    LOW(LATCH);

    while(1) {
        spi(0b10101010);
        spi(0b10101010);
        spi(0b10101010);
        spi(0b10101010);
        spi(0b10101010);
        spi(0b10101010);
        spi(0b10101010);
        spi(0b10101010);
        HIGH(LATCH);
        LOW(BLANK);
        _delay_ms(1);
        LOW(LATCH);
        _delay_ms(500);

        //READ: Note should be banked to avoid overload
        printf("%X%X%X%X\n",
            spi(0xFF) +
            (spi(0xFF) << 8),
            spi(0xFF) +
            (spi(0xFF) << 8),
            spi(0xFF) +
            (spi(0xFF) << 8),
            spi(0xFF) +
            (spi(0xFF) << 8)
        );
        HIGH(LATCH);
        _delay_ms(1);
        HIGH(BLANK);
        LOW(LATCH);
    }
}


/*
 * Binary UART messages
 *
 * Copyright 2015 <b@Zi.iS>
 */

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "main.h"

#include <util/setbaud.h>

uint8_t tx[UART_TX_BUFFER_SIZE];
uint8_t tx_start;
uint8_t tx_end = 2;
uint8_t tx_checksum = 0;
uint8_t tx_sending = 0;

uint8_t _id;

void uart_init(char id) {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0) | _BV(TXCIE0);   //Enable
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00) | _BV(UPM01); //8-bit, even parity

    _id = id;
}

void uart_send(char add, char data) {
    //Can't really check overflow, as outputing the error would just overflow more
    tx_checksum ^= add;
    tx[tx_end++] = add;
    tx_end %= UART_TX_BUFFER_SIZE;
    tx_checksum ^= data;
    tx[tx_end++] = data;
    tx_end %= UART_TX_BUFFER_SIZE;
    if(data == 0xFF) {
        tx[tx_end++] = data;
        tx_end %= UART_TX_BUFFER_SIZE;
    }
}

void error(char data) {
    uart_send(0x7F, data);
}

void reply() {
    HIGH(RS485_DIR);
    _delay_ms(10);
    tx_sending = tx_end;
    tx[tx_start] = 0x80 | (tx_sending + UART_TX_BUFFER_SIZE - tx_start) % UART_TX_BUFFER_SIZE;
    if (tx_checksum == 0xFF) {
        tx_checksum = 0;
    }
    tx[tx_start + 1] = tx_checksum;
    tx_end += 2;
    tx_end %= UART_TX_BUFFER_SIZE;
    tx_checksum = 0;
    UCSR0B |= _BV(UDRIE0);
}

ISR(USART0_RX_vect) {
    static uint8_t rx[3];
    static uint8_t rxidx = 0;
    static uint8_t checksum = 0;
    static bool skipff = FALSE;
    static bool packet = FALSE;

    if (UCSR0A & _BV(FE0)) {
        packet = FALSE;
        error('F');
        rx[rxidx] = UDR0;
        return;
    }
    if (UCSR0A & _BV(UPE0)) {
        packet = FALSE;
        error('P');
        rx[rxidx] = UDR0;
        return;
    }
    if (UCSR0A & _BV(DOR0)) {
        packet = FALSE;
        error('O');
        rx[rxidx] = UDR0;
        return;
    }
    rx[rxidx] = UDR0;
    if(!packet) {
        //Packets start with 0xFF, ID
        if(rx[(rxidx + 1) % 3] != 0xFF && rx[(rxidx + 2) % 3] == 0xFF && rx[rxidx] == _id) {
            packet = TRUE;
            checksum = rx[rxidx];
            skipff = FALSE;
            rxidx = 0;
            return;
        }
    } else {
        //all other 0xFF are escaped by repeating
        if(rx[rxidx] == 0xFF) {
            if(!skipff) {
                skipff = TRUE;
                return;
            } else {
                skipff = FALSE;
            }
        } else {
            if(skipff) {
                packet = FALSE;
                error('E');
                return;
            }
        }
        if(rxidx == 2) {
            if(checksum != rx[2]) {
                packet = FALSE;
                error('C');
                return;
            }
            if(rx[0] > 7) {
                dir[rx[0]-8] = rx[1];
            } else {
                cur[rx[0]] = rx[1];
            }
            packet = FALSE;
            reply();
        }
        checksum ^= rx[rxidx];
    }
    rxidx++;
}

ISR(USART0_UDRE_vect) {
    if (tx_start != tx_sending) {
        UDR0 = tx[tx_start++];
        tx_start %= UART_TX_BUFFER_SIZE;
    } else {
        UCSR0B &= ~_BV(UDRIE0);
    }
}

ISR(USART0_TX_vect) {
    _delay_ms(10);
    LOW(RS485_DIR);
}

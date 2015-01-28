/*
 * Binary UART messages
 *
 * Copyright 2015 <b@Zi.iS>
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "main.h"

#include <util/setbaud.h>

struct {
    int start;
    int end;
    union {
        uint8_t bytes[UART_TX_BUFFER_SIZE * 3];
        struct {
            uint8_t id:7;
            uint8_t head:1;

            uint8_t data7:1;
            uint8_t res:3;
            uint8_t add:3;
            uint8_t body:1;

            uint8_t data0123456:7;
            uint8_t body2:1;

        } msg[UART_TX_BUFFER_SIZE];
    } buffer;
} tx_buffer;

uint8_t _id;

union {
    uint8_t byte;
    struct {
        uint8_t msb:1;
        uint8_t res:2;
        uint8_t type:1;
        uint8_t add:3;
        uint8_t head:1;
    } msg;
} add;

#define SEEN_MY_ADDRESS 255
#define NOT_IN_MESSAGE 254
#define SEEN_OTHER_ADDRESS 253
#define SEEN_OTHER_BODY 252

void uart_init(char id) {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    //UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); //8-bit
    UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0) | _BV(TXCIE0);   //Enable

    for(int i=0; i<UART_TX_BUFFER_SIZE; i++) {
        tx_buffer.buffer.msg[i].head = 1;
        tx_buffer.buffer.msg[i].id = id;
    }
    _id = id | 0x80;
    add.byte = 254;
}

void uart_send(char address, char data) {
    tx_buffer.buffer.msg[tx_buffer.end/3].add = address & 0b111;
    tx_buffer.buffer.msg[tx_buffer.end/3].data7 = (data & 0x80) >> 7;
    tx_buffer.buffer.msg[tx_buffer.end/3].data0123456 = data & ~0x80;
    tx_buffer.end = (tx_buffer.end + 3) % (UART_TX_BUFFER_SIZE * 3);
}

void uart_poll_clear_to_send() {
    if((tx_buffer.start != tx_buffer.end) & !(UCSR0D & _BV(6))) {
        UCSR0B |= _BV(UDRIE0);
    }
}

ISR(USART0_RX_vect) {
    int b = UDR0;
    if(b & 0x80) {
        if(b == _id) {
            add.byte = SEEN_MY_ADDRESS;
        } else {
            add.byte = SEEN_OTHER_ADDRESS;
        }
    } else {
        if(add.byte == SEEN_MY_ADDRESS) {
            add.byte = b;
        } else if(add.byte < 128) {
            if(add.msg.type) {
                cur[add.msg.add] = b | add.msg.msb << 7;
            } else {
                dir[add.msg.add] = b | add.msg.msb << 7;
            }
            add.byte = NOT_IN_MESSAGE;
            UCSR0D |= _BV(6); //Clear RX Start so we can send
        } else if(add.byte == SEEN_OTHER_ADDRESS) {
            add.byte = SEEN_OTHER_BODY;
        } else if(add.byte == SEEN_OTHER_BODY) {
            add.byte = NOT_IN_MESSAGE;
            UCSR0D |= _BV(6); //Clear RX Start so we can send
        }
    }
}

ISR(USART0_UDRE_vect) {
    if (tx_buffer.start != tx_buffer.end) {
        HIGH(RS485_DIR);
        UDR0 = tx_buffer.buffer.bytes[tx_buffer.start];
        tx_buffer.start = (tx_buffer.start + 1) % (UART_TX_BUFFER_SIZE * 3);
    } else {
        UCSR0B &= ~_BV(UDRIE0);
    }
}

ISR(USART0_TX_vect) {
    LOW(RS485_DIR);
}

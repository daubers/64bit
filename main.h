#define UART_TX_BUFFER_SIZE 32

#define BLANK_port A
#define BLANK_pin 7

#define RS485_DIR_port A
#define RS485_DIR_pin 0

#define MOSI_port A
#define MOSI_pin 6

#define SCK_port A
#define SCK_pin 4

#define LATCH_port B
#define LATCH_pin 2

#define _hack(x, y) x ## y
#define __hack(x, y) _hack(x, y)

#define OUTPUT(x) __hack(DDR, x ## _port) |= _BV(x ## _pin)
#define HIGH(x) __hack(PORT, x ## _port) |= _BV(x ## _pin)
#define LOW(x) __hack(PORT, x ## _port) &= ~_BV(x ## _pin)
#define FALSE 0
#define TRUE 1

uint8_t cur[8];
uint8_t dir[8];

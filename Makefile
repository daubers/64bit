#lfuse = 0xEF;

CPU = attiny841

TOOLCHAIN = avr-
CC = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AS = $(TOOLCHAIN)as
LD = $(TOOLCHAIN)ld
OBJCP = $(TOOLCHAIN)objcopy
AR = $(TOOLCHAIN)ar

#CPPFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,--print-gc-sections
CPPFLAGS += -Os -DF_CPU=16000000
CPPFLAGS += -Wall -Werror
CPPFLAGS += -g
CPPFLAGS += -mmcu=$(CPU)
CPPFLAGS += -Wa,-aghlms=$@.lst
CFLAGS = -std=gnu11
CXXFLAGS = -std=gnu++11

LDFLAGS = -mmcu=$(CPU)
LDLIBS = -lm

all: main.hex

%.hex: %
	avr-size -C $<
	$(OBJCP) -O ihex -R .eeprom $< $@

clean:
	@rm main libs/*/*.o libs/*.o libs/*/*.lst libs/*.lst *.lst *.elf *.bin *.d *.map *.o 2> /dev/null || true

main: main.o uart.o

upload: main.hex
	avrdude -c avrispmkII -p $(CPU) -P usb -U flash:w:$+:i

run: upload
	miniterm.py /dev/ttyUSB0 38400

-include $(wildcard *.d)

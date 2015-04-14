#lfuse = 0xEF;

BAUD = 38400
#20000000
#38400
CPU = attiny841

TOOLCHAIN = avr-
CC = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AS = $(TOOLCHAIN)as
LD = $(TOOLCHAIN)ld
OBJCP = $(TOOLCHAIN)objcopy
AR = $(TOOLCHAIN)ar

#CPPFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,--print-gc-sections
CPPFLAGS += -MMD
CPPFLAGS += -Os -DF_CPU=16000000 -DBAUD=$(BAUD)
CPPFLAGS += -Wall -Werror
CPPFLAGS += -g
CPPFLAGS += -mmcu=$(CPU)
CPPFLAGS += -Wa,-aghlms=$@.lst
CFLAGS = -std=gnu11
CXXFLAGS = -std=gnu++11

LDFLAGS = -mmcu=$(CPU)
LDLIBS = -lm

DEPS := $(COBJS:.o=.d)

-include $(DEPS)

all: main.hex

%.hex: %
	avr-size -C $<
	$(OBJCP) -O ihex -R .eeprom $< $@

clean:
	@rm main libs/*/*.o libs/*.o libs/*/*.lst libs/*.lst *.lst *.elf *.bin *.d *.map *.o 2> /dev/null || true

main: main.o uart.o

upload: main.hex
	avrdude -B0.3 -b1000000 -c dragon_isp -p $(CPU) -P usb -U flash:w:$+:i && touch $@

run: upload
	miniterm.py --parity=E /dev/ttyUSB0 $(BAUD)

-include $(wildcard *.d)

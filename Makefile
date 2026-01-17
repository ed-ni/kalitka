MCU = attiny13a
F_CPU = 1200000UL

SRC_DIR = src
SRC = $(SRC_DIR)/main.cpp

CC = avr-gcc
OBJCOPY = avr-objcopy

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall
LDFLAGS = -mmcu=$(MCU)

TARGET = main

all: $(TARGET).hex

$(TARGET).elf: $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

flash: $(TARGET).hex
	avrdude -c avrisp -p t13 -P /dev/ttyUSB0 -b 19200 -U flash:w:$<

fuses:
	avrdude -c avrisp -p t13 -P /dev/ttyUSB0 -b 19200 \
		-U lfuse:w:0x6A:m -U hfuse:w:0xFF:m

clean:
	rm -f *.elf *.hex

CC = avr-gcc

F_CPU = 16000000UL

TARGET = main
ELF = $(TARGET).elf
HEX = $(TARGET).hex

CFLAGS = -Wall -Wextra -Werror -DF_CPU=$(F_CPU) -mmcu=atmega328p

all: hex flash

$(ELF): main.c
	$(CC) $(CFLAGS) main.c -o $(ELF)

hex: $(ELF)
	avr-objcopy -O ihex $(ELF) $(HEX)

flash: hex
	avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 -U flash:w:$(HEX)

clean:
	rm -f $(HEX) $(ELF)

CC = avr-gcc

F_CPU = 16000000UL

TARGET = main
BIN = $(TARGET).bin
HEX = $(TARGET).hex

CFLAGS = -Wall -Wextra -Werror -DF_CPU=$(F_CPU) -mmcu=atmega328p

all: hex flash

$(BIN): main.c
	$(CC) $(CFLAGS) main.c -o $(BIN)

hex: $(BIN)
	avr-objcopy -O ihex $(BIN) $(HEX)

flash: hex
	avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 -U flash:w:$(HEX)

clean:
	rm -f $(BIN) $(HEX)

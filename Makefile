CC=avr-gcc
FLAGS=-Os -mmcu=attiny84a -Wall -pedantic -fwrapv
OBJCOPY=avr-objcopy
SERIALPORT=/dev/ttyUSB0

all:
	$(CC) $(FLAGS) -c Timer/timer.c
	$(CC) $(FLAGS) -c UART/uart.c
	$(CC) $(FLAGS) -c Modbus/modbus.c
	$(CC) $(FLAGS) -c loki.c
	$(CC) $(FLAGS) -o loki.elf loki.o timer.o uart.o modbus.o
	$(OBJCOPY) -j .text -j .data -O ihex loki.elf loki.hex


program:
	avrdude -c avrisp -p t84 -P $(SERIALPORT) -b 19200 -U flash:w:loki.hex


clean:
	rm *.o
	rm *.elf
	rm *.hex

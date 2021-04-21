MCU=atmega88
CFLAGS=-Wall -gdwarf-2 -Os -std=gnu99 -DF_CPU=8000000 -fno-inline-small-functions -ffunction-sections -fdata-sections -Wl,--relax,--gc-sections -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000 -I ../simavr/simavr/sim -I ../simavr/simavr/sim/avr
CC=avr-gcc
OBJCOPY=avr-objcopy

TARGET=ledblink_again

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -j .text -j .data -j .eeprom -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).elf: $(TARGET).c
	$(CC) $(CFLAGS) -mmcu=$(MCU) $(TARGET).c -o $(TARGET).elf

clean:
	rm -f *.elf *.hex
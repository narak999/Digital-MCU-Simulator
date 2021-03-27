import subprocess
file_type = "cpp"
f = open("Makefile", "w")
input_file = ""
if file_type == "c":
        input_file = "ledblink"
        f.write("PORT=\\\\\\\\.\\\\GLOBALROOT\\\\Device\\\\USBSER000\n"
                "MCU=atmega328p\n"
                "CFLAGS=-c -Os -Wall -DF_CPU=16000000L\n"
                "CC=avr-gcc\n"
                "OBJCOPY=avr-objcopy\n\n"
                f"TARGET={input_file}\n\n"
                "$(TARGET).hex: $(TARGET).elf\n\t"
                "$(OBJCOPY) -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex\n\n"
                "$(TARGET).elf: $(TARGET).o\n\t"
                "$(CC) -mmcu=$(MCU) $(TARGET).o -o $(TARGET).elf\n\n"
                "$(TARGET).o: $(TARGET).c\n\t"
                "$(CC) $(CFLAGS) -mmcu=$(MCU) $(TARGET).c\n\n"
                "clean:\n\t"
                "rm -f *.o *.hex *.obj *.hex")
elif file_type == "cpp":
        input_file = "Cpp_Example"
        f.write("PORT=\\\\\\\\.\\\\GLOBALROOT\\\\Device\\\\USBSER000\n"
                "MCU=atmega328p\n"
                "CFLAGS=-c -Os -Wall -DF_CPU=16000000L\n"
                "CC=avr-g++\n"
                "OBJCOPY=avr-objcopy\n\n"
                f"TARGET={input_file}\n\n"
                "$(TARGET).hex: $(TARGET).elf\n\t"
                "$(OBJCOPY) -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex\n\n"
                "$(TARGET).elf: $(TARGET).o\n\t"
                "$(CC) -mmcu=$(MCU) $(TARGET).o -o $(TARGET).elf\n\n"
                "$(TARGET).o: $(TARGET).cpp\n\t"
                "$(CC) $(CFLAGS) -mmcu=$(MCU) $(TARGET).cpp\n\n"
                "clean:\n\t"
                "rm -f *.o *.hex *.obj *.hex")
f.close()

p = subprocess.Popen(['make'], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
result = p.communicate()[0].decode("utf-8")
print(result)
try:
        f = open(input_file + ".hex", "r")
        print(f.read())
except (OSError, IOError) as e:
        print(e)
finally:
        f.close()


# Digital-MCU-Simulator
This is the project for ECEN 403 and ECEN 404 class.

For my code (DMCUS_IDE.py), in order to successfully run it, the user will need to change the directories inside the code. The user will also need to install some Python packages including Pygments, Tkinter, Signal, Subprocess, and Webbrowser. One more thing that might prevent the code from running on the user's computer might be the location of the icon file cannot be located, although the icon file is attached with the code above. 

NOTE: This Python code is created and meant to run with Python 3.6. Using a different version of Python to compile and run the code might cause unexpected errors.

Compilation command for simavr: 

avr-gcc -Wall -gdwarf-2 -Os -std=gnu99 -DF_CPU=8000000 -fno-inline-small-functions -ffunction-sections -fdata-sections -Wl,--relax,--gc-sections -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000 -I ~/Desktop/simavr/simavr/sim -I ~/Desktop/simavr/simavr/sim/avr -mmcu=atmega88 ledblink.c -o ledblink.elf

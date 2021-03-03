#include <stdio.h>
#include "libMPSSE_i2c.h"

// GPIO Extender I2C Addresses
#define GPIO_EX_1_ADR 0100010
#define GPIO_EX_2_ADR 0100011

// internal registers in GPIO Extenders for each bank of 8 GPIO pins
#define GPIO_EX_BANK_A 0x09
#define GPIO_EX_BANK_B 0x19

static uint8 gpio1_a_val;
static uint8 gpio1_b_val;
static uint8 gpio2_a_val;
static uint8 gpio2_b_val;
static uint8 int_gpio_val;
static uint8 gpio_dir;
static FT_STATUS ftStatus;

int read_ADC() {
	// reads the voltage on the ADC from the I2C bus and returns it as an integer

}

void write_DAC(int value) {
	// writes a value to the DAC to output it. Double check input size on this

}

bool write_GPIO(int line, bool value) {
	// sets the value on a GPIO line to high (true) or low (false) voltage
	// returns true if successful, false if not
	//
	// layout of the lines is as follows:
	/* 0-15:  GPIO Extender 1 (P29-P44)
	 * 16-31: GPIO Extender 2 (P13-P28)
	 * 32-39: Interface Chip GPIO Ch A (P3, P4, P7-P11, P51) (GPIOHx)
	 * 40-41: Interface Chip GPIO Ch B (P5, P6) (GPIOL2 & GPIOL3) (L0 & L1 for I2C)
			* Looking at the libMPSSE programming guide, I don't think these pins are ever accessible as GPIO, despite not being used for I2C, so these will be disregarded
	 */

	// on onboard GPIO, direction is 0 for in, 1 for out


	if (line <= 15 && line >= 0) {
		// code for sending appropriate I2C commands to GPIO Extender 1
		// I2C Address: 010 0010

		// GPIO extenders are split into two banks of 8 lines, need an offset to set up value correctly
		int offset = 0;
		if (line > 7) {
			offset = 8;
		}

		// create output value
		// if line value is 1, use ORR, if line value is 0, use XOR
		if (line > 7) {
			if (value == 0) {
				gpio1_b_val = gpio1_b_val ^ (value << (line - offset));
			}
			else {
				gpio1_b_val = gpio1_b_val | (value << (line - offset));
			}
		}
		else {
			if (value == 0) {
				gpio1_a_val = gpio1_a_val ^ (value << (line - offset));
			}
			else {
				gpio1_a_val = gpio1_a_val | (value << (line - offset));
			}
		}

		// configure bank to write to requested pin

		// create I2C Byte Buffer
		// 2 bytes to send, one register byte and one data byte
		uint16 data_send = 0;
		if (line < 8) {
			data_send = data_send + gpio1_a_val;
			data_send = (data_send << 8) + GPIO_EX_BANK_A;
		}
		else {
			data_send = data_send + gpio1_b_val;
			data_send = (data_send << 8) + GPIO_EX_BANK_B;
		}

	}
	else if(line <= 31) {
		// code for sending appropriate I2C commands to GPIO Extender 2
		// I2C Address: 010 0011

	}
	else if(line <= 39) {

		// first create dir byte by ORR existing gpio_dir with 2^n, where n is the line (from 0 to 7) to be written to
		gpio_dir = gpio_dir | (1 << (line - 32));

		// create output value
		// if line value is 1, use ORR, if line value is 0, use XOR
		if (value == 0) {
			int_gpio_val = int_gpio_val ^ (value << (line - 32));
		}
		else {
			int_gpio_val = int_gpio_val | (value << (line - 32));
		}

		// TODO: find out what handles are here. Should be in d2xx files
		ftStatus = FT_WriteGPIO(int_GPIO_handle, gpio_dir, (uint8)value);
	}
	else {
		printf("Line value out of acceptable bounds. Must be between 0 and 39 inclusive\n");
		return 0;
	}

}

int main() {

	// sets the initial output values of each GPIO line to 0
	// will be changed in write_GPIO function to change the ouptut
	gpio1_val = 0;
	gpio2_val = 0;
	int_gpio_val = 0;
	gpio_dir = 0;

	ftStatus = 0;
	

}

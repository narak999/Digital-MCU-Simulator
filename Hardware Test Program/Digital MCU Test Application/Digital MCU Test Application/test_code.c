/*
Digital MCU Simulator
ECEN 404 Team 10, Fall 2020-Spring 2021
Jonathan Howard (and others)

This code tests the Physical I/O Board designed for the Digital MCU Simulator project. The physical I/O board is built around the FTDI FT2232HL chip for USB GPIO and I2C Communcations, accomplished with the libMPSSE library.

Several functions in this code, including
	* init_time(), start_time(), and stop_time()
	* write_bytes() and read_bytes()
are adapted from sample code provided with the libMPSSE library. 
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pch.h"
#ifdef _WIN32
#include <windows.h> 
#endif
#include "libMPSSE_i2c.h"

#define I2C_DEVICE_BUFFER_SIZE
#define CHANNEL_TO_OPEN 0 //should only need the first channel
#define MAX_WRITE_TRIALS 10

// GPIO Extender I2C Addresses
#define GPIO_EX_1_ADR 0b0100010
#define GPIO_EX_2_ADR 0b0100011

// internal registers in GPIO Extenders for each bank of 8 GPIO pins
#define GPIO_EX_BANK_A_VAL 0x09
#define GPIO_EX_BANK_B_VAL 0x19
#define GPIO_EX_BANK_A_DIR 0x01
#define GPIO_EX_BANK_B_DIR 0x11

// macros for easier reading and writing
#define READ 1
#define INPUT 1
#define WRITE 0
#define OUTPUT 0

// ADC and DAC Addresses
#define ADC_ADR 0b0100001
#define ADC_CONF_REG 0x02
#define DAC_ADR 0b1001100
#define DAC_CONF_REG 0x40

// val variables store the value of the 8 bits of that GPIO bank
// dir variables store the direction of the 8 bits of that GPIO bank
// *never* change a variable without reading from the GPIO bank first, slower but safer
static uint8 gpio1_a_val;
static uint8 gpio1_b_val;
static uint8 gpio2_a_val;
static uint8 gpio2_b_val;
static uint8 gpio1_a_dir;
static uint8 gpio1_b_dir;
static uint8 gpio2_a_dir;
static uint8 gpio2_b_dir;
static uint8 int_gpio_val;
static uint8 int_gpio_dir;
static FT_STATUS ftStatus;
static uint8 buffer[I2C_DEVICE_BUFFER_SIZE] = { 0 };
static uint32 timeWrite = 0;
static uint32 timeRead = 0;
#ifdef _WIN32
static LARGE_INTEGER llTimeStart = { 0 };
static LARGE_INTEGER llTimeEnd = { 0 };
static LARGE_INTEGER llFrequency = { 0 };
#endif

// FT2232HL Setup variables
static uint32 numDevices;
static uint32 deviceFlags;
static uint32 deviceType;
static uint32 deviceID;
static uint32 deviceLocID;
static char serial[16];
static char description[64];
static FT_HANDLE deviceHandle;

static uint32 numI2Cchannels;
static FT_HANDLE i2cChannel;
static ChannelConfig i2cConfig;


// Standard Functions, adapted from example program (included in the github under libMPSSE/Samples
// initializes timer
static void init_time() {
#ifdef _WIN32
	QueryPerformanceFrequency(&llFrequency);
#endif
}

// starts timer
static void start_time() {
#ifdef _WIN32
	QueryPerformanceCounter(&llTimeStart);
#endif

}

// stops timer
static uint32 stop_time() {
	uint32 dwTemp = 0;
#ifdef _WIN32
	QueryPerformanceCounter(&llTimeEnd);
	dwTemp = (uint32)((llTimeEnd.QuadPart - llTimeStart.QuadPart) * 1000 / (float)llFrequency.QuadPart);
#endif
	return dwTemp;
}

// Writes to I2C device
// sample code writes to a 24LC024H EEPROM, will be used for several devices
static FT_STATUS write_bytes(uint8 deviceAddress, uint8 registerAddress, uint8* data, uint32 numBytes) {
	
	FT_STATUS status;
	uint32 bytesToTransfer = 0;
	uint32 bytesTransferred = 0;
	uint32 options = 0;
	uint32 trials = 0;

#if FAST_TRANSFER
	options = I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT | I2C_TRANSFR_OPTIONS_FAST_TRANSFER_BYTES;
#else
	options = I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT;
#endif

	buffer[bytesToTransfer++] = registerAddress;
	memcpy(buffer + bytesToTransfer, data, numBytes);
	bytesToTransfer += numBytes;

	// timing write procedure
	start_time();
	status = I2C_DeviceWrite(i2cChannel, deviceAddress, bytesToTransfer, buffer, &bytesTransferred, options);
	timeWrite = stop_time();

	// if information isn't written, try until MAX_WRITE_TRIALS attempts have been made
	while (status != FT_OK && trials < MAX_WRITE_TRIALS) {
		start_time();
		status = I2C_DeviceWrite(i2cChannel, deviceAddress, bytesToTransfer, buffer, &bytesTransferred, options);
		timeWrite = stop_time();
		trials++;
	}

	return status;

}

// reads from I2C device
static FT_STATUS read_bytes(uint8 slaveAddress, uint8 registerAddress, uint8* data, uint32 numBytes) {
	
	FT_STATUS status = FT_OK;
	uint32 bytesToTransfer = 0;
	uint32 bytesTransferred = 0;
	uint32 options = 0;
	uint32 trials = 0;

#if FAST_TRANSFER
	options = I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT | I2C_TRANSFR_OPTIONS_FAST_TRANSFER_BYTES;
#else
	options = I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT;
#endif
	status = I2C_DeviceRead(i2cChannel, slaveAddress, bytesToTransfer, buffer, &bytesTransferred, options);
	trials = 0;
	while (status != FT_OK && trials < MAX_WRITE_TRIALS) {
		status = I2C_DeviceRead(i2cChannel, slaveAddress, bytesToTransfer, buffer, &bytesTransferred, options);
		trials++;
	}
	return status;
}

uint16 read_ADC(int adc_pin) {
	// reads the voltage on the ADC from the I2C bus and returns it as an integer

	/* ADC Channel-Pin Mapping
		
		Channel		Pin
		1			 2
		2			47
		3			12
		4			48
		5			45
		6			49
		7			46
		8			50

		(in code, for simplicity channels are 0-indexed instead of 1-indexed, but the above mapping holds)
	
		Never rely on auto component numbering, ever
	*/

	int channel = -1;
	switch (adc_pin) {
	case 2:
		channel = 0;
		break;
	case 47:
		channel = 1;
		break;
	case 12:
		channel = 2;
		break;
	case 48:
		channel = 3;
		break;
	case 45:
		channel = 4;
		break;
	case 49:
		channel = 5;
		break;
	case 46:
		channel = 6;
		break;
	case 50:
		channel = 7;
		break;
	default:
		channel = -1;
		break;
	}
	if (channel == -1) {
		printf("Specified pin is not an ADC Input. Please consult references and PCB Silkscreen");
		return -1;
	}
	else {
		/* Reading from the ADC:
			Register address is 
			"1 [channel in binary, 0-7] 0000 
		
		*/

		// register bitmask
		uint8 adc_register = ((0b1000 | channel) << 4);
		// need to reserve 2 bytes of space, but send the data location as a pointer to 1 byte of space, kind of hacky tbh
		uint16 value = 0;
		void* val_ptr = (void*)&value;
		ftStatus = read_bytes(ADC_ADR, adc_register, (uint8*)val_ptr, 2);
		// this might need to be swapped, should return MSB first then LSB. TBD
		return value;

	}

}

void write_DAC(uint16 value) {
	// writes a value to the DAC to output it. Double check input size on this
	// use the "Write DAC and input registers" command, = 0011xxxx
	uint16 write_value = value << 4; // DAC reads from the 12 MSB of the two bytes, and the bottom 4 bits are don't cares. This positions the value as expected.
	// hacky stuff, see ADC write code
	void* val_ptr = (void*)&write_value;
	// just as with ADC, might need to swap order of bytes in write_value. Endianness tbd
	ftStatus = write_bytes(DAC_ADR, 0x30, (uint8*)val_ptr, 2);
}

bool write_GPIO(int line, bool value) {
	// sets the value on a GPIO line to high (true) or low (false) voltage
	// returns true if successful, false if not
	//
	// layout of the lines is as follows:
	/*
		0-15: GPIO Extender 1 (P29-P44)
	   16-31: GPIO Extender 2 (P13-P28)
	   32-39: Interface Chip GPIO Ch A (P3, P4, P7-P11, P51) (GPIOHx)
	 */

	// on onboard GPIO, direction is 0 for in, 1 for out


	if (line <= 15 && line >= 0) {
		// code for sending appropriate I2C commands to GPIO Extender 1
		// I2C Address: 010 0010

		// GPIO extenders are split into two banks of 8 lines, need an offset to set up value correctly
		int offset = 0;
		if (line > 7)
			offset = 8;

		// read from GPIO values
		if (line > 7) {
			read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_B_VAL, &gpio1_b_val, 1);
		}
		else {
			read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_A_VAL, &gpio1_a_val, 1);
		}

		// create output value
		// if line value is 1, use ORR, if line value is 0, use XOR
		// creates a bitmask of the desired value and the existing value on the GPIO line
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

		// create I2C Byte Buffer
		uint8 data_send = 0;
		if (line > 7) {
			write_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_B_VAL, &gpio1_b_val, 1);
			return 1;
		}
		else {
			write_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_A_VAL, &gpio1_a_val, 1);
			return 1;
		}

	}
	else if(line <= 31) {
		// code for sending appropriate I2C commands to GPIO Extender 2
		// I2C Address: 010 0011

		// GPIO extenders are split into two banks of 8 lines, need an offset to set up value correctly
		int line2 = line - 16; // offsets line to reduce the amount of code changes
		int offset = 0;
		if (line > 7)
			offset = 8;

		// read from GPIO values
		if (line2 > 7) {
			read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_B_VAL, &gpio2_b_val, 1);
		}
		else {
			read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_A_VAL, &gpio2_a_val, 1);
		}

		// create output value
		// if line value is 1, use ORR, if line value is 0, use XOR
		// creates a bitmask of the desired value and the existing value on the GPIO line
		if (line2 > 7) {
			if (value == 0) {
				gpio2_b_val = gpio2_b_val ^ (value << (line2 - offset));
			}
			else {
				gpio2_b_val = gpio2_b_val | (value << (line2 - offset));
			}
		}
		else {
			if (value == 0) {
				gpio2_a_val = gpio2_a_val ^ (value << (line2 - offset));
			}
			else {
				gpio2_a_val = gpio2_a_val | (value << (line2 - offset));
			}
		}
		// create I2C Byte Buffer
		uint8 data_send = 0;
		if (line2 > 7) {
			write_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_B_VAL, &gpio2_b_val, 1);
			return 1;
		}
		else {
			write_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_A_VAL, &gpio2_a_val, 1);
			return 1;
		}
	}
	else if(line <= 39) {

		// create output value
		// if line value is 1, use ORR, if line value is 0, use XOR
		if (value == 0) {
			int_gpio_val = int_gpio_val ^ (value << (line - 32));
		}
		else {
			int_gpio_val = int_gpio_val | (value << (line - 32));
		}

		// TODO: find out what handles are here. Should be in d2xx files
		ftStatus = FT_WriteGPIO(deviceHandle, int_gpio_dir, (uint8)value);
		return 1;
	}
	else {
		printf("Line value out of acceptable bounds. Must be between 0 and 39 inclusive\n");
		return 0;
	}

}

bool read_GPIO(int line) {

	/*
	line numbers:
		0-15: GPIO Extender 1 (P29-P44)
	   16-31: GPIO Extender 2 (P13-P28)
	   32-39: Interface Chip GPIO Ch A (P3, P4, P7-P11, P51) (GPIOHx)

	   order of operations:
		* Read DIR from GPIO bank
		* Change DIR to be able to read from the pin
		* Write
	 */
	uint8 adr = 0;
	if (line < 0) {
		printf("Line must be a positive value");
		return 0;
	}
	int line2 = line;
	if (line >= 16 && line <= 31) {
		// offsets line for compressed code
		line2 = line - 16;
	}
	int	offset = 0;
	if (line2 <= 7) {
		offset = 0;
	}
	else if (line2 <= 15) {
		offset = 8;
	}

	uint8 bitmask = 1 << (line2 - offset);

	if (line <= 15) {
		// read from GPIO values
		if (line2 > 7) {
			read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_B_VAL, &gpio1_b_val, 1);
			return (gpio1_b_val & bitmask) >> (line2 - offset);
		}
		else {
			read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_A_VAL, &gpio1_a_val, 1);
			return (gpio1_a_val & bitmask) >> (line2 - offset);
		}
	}
	else if (line <= 31) {
		// read from GPIO values
		if (line2 > 7) {
			read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_B_VAL, &gpio2_b_val, 1);
			return (gpio2_b_val & bitmask) >> (line2 - offset);
		}
		else {
			read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_A_VAL, &gpio2_a_val, 1);
			return (gpio2_a_val & bitmask) >> (line2 - offset);
		}
	}
	return 0;
	
}

static FT_STATUS set_pin_dir(int pin, bool dir) {
	/* pin directions:
	
		* To read from the pin, dir = READ
		* To write from the pin, dir = WRITE
	
	*/
	if (pin < 32) {
		// if pin is on the GPIO Expanders
		int	offset = 0;
		int pin2 = pin;
		if (pin >= 16 && pin <= 31) {
		// offsets pin for compressed code
			pin2 = pin - 16;
		}
		if (pin2 <= 7) {
			offset = 0;
		}
		else if (pin2 <= 15) {
			offset = 8;
		}

		if (pin <= 15) {
			// if pin <= 15, work with GPIO Expander 1
			if (pin2 > 7) {
				// if dir is READ, use ORR, if dir is WRITE, use XOR
				// creates a bitmask of the desired value and the existing value on the GPIO line
				read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_B_DIR, &gpio1_b_dir, 1);
				if (dir == WRITE) {
					gpio1_b_val = gpio1_b_val ^ (dir << (pin2 - offset));
				}
				else {
					gpio1_b_val = gpio1_b_val | (dir << (pin2 - offset));
				}
				write_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_B_DIR, &gpio1_b_dir, 1);
			}
			else {
				read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_A_DIR, &gpio1_a_dir, 1);
				if (dir == WRITE) {
					gpio1_a_val = gpio1_a_val ^ (dir << (pin2 - offset));
				}
				else {
					gpio1_a_val = gpio1_a_val | (dir << (pin2 - offset));
				}
				write_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_A_DIR, &gpio1_a_dir, 1);
			}
		}
		else {
			// otherwise, work with GPIO Expander 2
			if (pin2 > 7) {
				// if dir is READ, use ORR, if dir is WRITE, use XOR
				// creates a bitmask of the desired value and the existing value on the GPIO line
				read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_B_DIR, &gpio2_b_dir, 1);
				if (dir == WRITE) {
					gpio2_b_val = gpio2_b_val ^ (dir << (pin2 - offset));
				}
				else {
					gpio2_b_val = gpio2_b_val | (dir << (pin2 - offset));
				}
				write_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_B_DIR, &gpio2_b_dir, 1);
			}
			else {
				read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_A_DIR, &gpio2_a_dir, 1);
				if (dir == WRITE) {
					gpio2_a_val = gpio2_a_val ^ (dir << (pin2 - offset));
				}
				else {
					gpio2_a_val = gpio2_a_val | (dir << (pin2 - offset));
				}
				write_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_A_DIR, &gpio2_a_dir, 1);
			}
		}
	}
	else if (pin <= 39) {
		// if pin is on internal GPIO
	}
	else {
		printf("Cannot write on pin %d, out of range.", pin);
	}
}

static start_library() {
	Init_libMPSSE();
}

static void clean_library() {
	Cleanup_libMPSSE();
}

int main() {

	// initializes variables for GPIO Expansion
	// will be changed in write_GPIO function to change the ouptut
	gpio1_a_val = 0;
	gpio1_b_val = 0;
	gpio2_a_val = 0;
	gpio2_b_val = 0;
	int_gpio_val = 0;
	gpio1_a_dir = 0;
	gpio1_b_dir = 0;
	gpio2_a_dir = 0;
	gpio2_b_dir = 0;
	int_gpio_dir = 0x55; // internal GPIO pins will alternate in/out in/out. 1 is an output, 0 is an input
	i2cConfig.ClockRate = I2C_CLOCK_STANDARD_MODE;
	i2cConfig.LatencyTimer = 250;
	i2cConfig.Options = 0;

	ftStatus = 0;

	/*
											Interface Configuration
	-----------------------------------------------------------------------------------------------------------
	Configures the FT2232HL accordingly
	
	*/
	ftStatus = FT_CreateDeviceInfoList(&numDevices);
	ftStatus = FT_GetDeviceInfoDetail(0, &deviceFlags, &deviceType, &deviceID, &deviceLocID, &serial, &description, &deviceHandle);

	// naively assume only one device is connected for now, will flesh out later
	// finally found a step-by-step guide for this device setup from FTDI. Awful documentation
	
	ftStatus = FT_Open(0, &deviceHandle);
	/*
	ftStatus = FT_ResetDevice(deviceHandle);
	ftStatus = FT_SetUSBParameters(deviceHandle, 4096, 4096);	// input and output transfers will be 4 kiB large
	ftStatus = FT_SetChars(deviceHandle, 0, 0, 0, 0);			// disables event and error characters, documentation is kind of unclear beyond "Most applications disable these"
	ftStatus = FT_SetTimeouts(deviceHandle, 1000, 1000);		// sets a 1 second timeout on both reading and writing to the device
	ftStatus = FT_SetLatencyTimer(deviceHandle, 50);			// Device will wait 50 ms before sending an incomplete USB packet back to the host computer. Will adjust based on latencies of host components.
	ftStatus = FT_SetFlowControl(deviceHandle, FT_FLOW_RTS_CTS, 0x10, 0x13); // honestly not quite sure what this does, setup guide says I need it. Will do more research in the future
	ftStatus = FT_SetBitMode(deviceHandle, 0, 0);
	ftStatus = FT_SetBitMode(deviceHandle, 0, 0x2);	// 0x2 sets this device as an MPSSE device, for I2C and GPIO support, instead of the default serial interface. Pin directions will be set up later
	*/

	// according to the libMPSSE manual initializing the I2C channel should do all of the above automatically.
	ftStatus = I2C_GetNumChannels(&numI2Cchannels);
	ftStatus = I2C_OpenChannel(0, i2cChannel);
	ftStatus = I2C_InitChannel(i2cChannel, &i2cConfig);

	// configuring the MPSSE
	// First, the start up guide suggests a test communication is sent with a deliberately bad command, and checking for the "bad command" return code.
	// based on the list of available commands, 0xF0 should be a bad command.
	uint8 testCommand = 0xf0;
	uint8 returnFromTest = 0;
	uint32 bytesWritten = 0;
	uint32 bytesRead = 0;
	ftStatus = FT_Write(deviceHandle, (void*)&testCommand, 1, &bytesWritten);
	// if the write was successful, read back
	if (ftStatus == FT_OK) {
		ftStatus = FT_Read(deviceHandle, (void*)&returnFromTest, 1, &bytesRead);
		if (returnFromTest == 0xFA) {
			printf("Test communciation successful!\n");
		}
		else {
			printf("Test communication failed, please send help\n");
		}
	}


	
	/*
											ADC Configuration
	-----------------------------------------------------------------------------------------------------------
	Configures the AD7998 ADC for functionality
	Per the Datasheet,
		D11-D4 control the 8th to 1st ADC Channels, respectively
		D3 controls the I2C Filter. Datasheet appears to suggest default to 1, for an active filter
		D2 Enables the built in Alert function,
		D1 controls how the alert/busy function is used
		D0 controls the polarity of the alert function

		Since the Alert pin is NC on the PCB, D2:D0 will be set to 0.
		All 8 pins are exposed on the PCB for reading, so all 8 pins will be set to read
		D15-D12 are 0, as 2 bytes need to be written but only 12 control bits are needed

		control is 0000 1111 1111 1000 = 0x0FF8
		configuration register location is 00000010 = 0x02

	*/

	uint16 adc_config = 0x0FF8;
	void* adc_config_ptr = (void*)&adc_config;
	// writes config to ADC
	write_bytes(ADC_ADR, ADC_CONF_REG, (uint8*)adc_config_ptr, 2);

	/*
											DAC Configuration
	-----------------------------------------------------------------------------------------------------------
	Configures the AD56914 DAC to function as intended
	5 control bits, need to write 2 bytes, control bits are 5 MSB in high byte
	from MSB to LSB of control bits, we have
	Reset	- Resets DAC, good practice to reset on initialization
	PD1		- PD1 and PD0 are power down modes, not necessary so will keep 0 for normal operation
	PD0
	REF		- 0 enables Internal Reference, 1 disables Internal Reference. DAC is configured on the board to use the internal reference, so this stays 0
	Gain	- Output gain, keeping 0 to V_Ref swing for 0 to 2.5V.

	Config data is therefore 10000 000 0000 0000 = 0x8000

	config register is 0100 0000 =  0x40
	*/
	uint16 dac_config = 0x8000;
	void* dac_config_ptr = (void*)&dac_config;
	write_bytes(DAC_ADR, DAC_CONF_REG, (uint8*)dac_config_ptr, 2);

	/*
											GPIO Configuration
	-----------------------------------------------------------------------------------------------------------
	Configures the MCP2017 I/O Expanders to function as intended
	*/

		
	// Set up default configuration values in IOCON (configuration) register
	// Due to default value, this should be at address 0A. After this configuration is sent, it will be at 05.
	uint8 gpio_config_adr = 0x0A;
	uint8 gpio_config_data = 0x60;
	// I2C line to send configuration data, shouldn't need to touch after this
	ftStatus = write_bytes(GPIO_EX_1_ADR, gpio_config_adr, &gpio_config_data, 1);
	ftStatus = write_bytes(GPIO_EX_2_ADR, gpio_config_adr, &gpio_config_data, 1);
	
	// reads starting directions from GPIO Extenders
	ftStatus = read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_A_DIR, &gpio1_a_dir, 1);
	ftStatus = read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_B_DIR, &gpio1_b_dir, 1);
	ftStatus = read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_A_DIR, &gpio1_a_dir, 1);
	ftStatus = read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_B_DIR, &gpio1_b_dir, 1);

	// read the values from GPIO Expanders
	ftStatus = read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_A_VAL, &gpio1_a_val, 1);
	ftStatus = read_bytes(GPIO_EX_1_ADR, GPIO_EX_BANK_B_VAL, &gpio1_b_val, 1);
	ftStatus = read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_A_VAL, &gpio1_a_val, 1);
	ftStatus = read_bytes(GPIO_EX_2_ADR, GPIO_EX_BANK_B_VAL, &gpio1_b_val, 1);

}

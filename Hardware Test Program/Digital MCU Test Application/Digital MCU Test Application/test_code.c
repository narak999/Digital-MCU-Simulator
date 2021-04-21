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
//#include "pch.h"
#ifdef _WIN32
#include <windows.h> 
#endif
#include <unistd.h>
#include "ftd2xx.h"

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
static unsigned char gpio1_a_val;
static unsigned char gpio1_b_val;
static unsigned char gpio2_a_val;
static unsigned char gpio2_b_val;
static unsigned char gpio1_a_dir;
static unsigned char gpio1_b_dir;
static unsigned char gpio2_a_dir;
static unsigned char gpio2_b_dir;
static unsigned char int_gpio_val;
static unsigned char int_gpio_dir;
static FT_STATUS ftStatus;
static unsigned char buffer[I2C_DEVICE_BUFFER_SIZE] = { 0 };
static unsigned long int timeWrite = 0;
static unsigned long int timeRead = 0;
#ifdef _WIN32
static LARGE_INTEGER llTimeStart = { 0 };
static LARGE_INTEGER llTimeEnd = { 0 };
static LARGE_INTEGER llFrequency = { 0 };
#endif

// FT2232HL Setup variables
static unsigned int numDevices = 0;
static unsigned int deviceFlags = 0;
static unsigned int deviceType = 0;
static unsigned int deviceID = 0;
static unsigned int deviceLocID = 0;
static char serial[16];
static char description[64];

static unsigned long int numI2Cchannels = 0;
//static ChannelConfig i2cConfig;

FT_HANDLE deviceHandle;
//FT_HANDLE i2cChanne l;


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
static unsigned long int stop_time() {
	unsigned long int dwTemp = 0;
#ifdef _WIN32
	QueryPerformanceCounter(&llTimeEnd);
	dwTemp = (unsigned long int)((llTimeEnd.QuadPart - llTimeStart.QuadPart) * 1000 / (float)llFrequency.QuadPart);
#endif
	return dwTemp;
}

// Writes to I2C device
// sample code writes to a 24LC024H EEPROM, will be used for several devices
static FT_STATUS write_bytes(unsigned char deviceAddress, unsigned char registerAddress, unsigned char* data, unsigned long int numBytes) {
	
	FT_STATUS status;
	unsigned long int bytesToTransfer = 0;
	unsigned long int bytesTransferred = 0;
	unsigned long int options = 0;
	unsigned long int trials = 0;

	/*
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
	status = I2C_DeviceWrite(deviceHandle, deviceAddress, bytesToTransfer, buffer, &bytesTransferred, options);
	timeWrite = stop_time();

	// if information isn't written, try until MAX_WRITE_TRIALS attempts have been made
	while (status != FT_OK && trials < MAX_WRITE_TRIALS) {
		start_time();
		status = I2C_DeviceWrite(deviceHandle, deviceAddress, bytesToTransfer, buffer, &bytesTransferred, options);
		timeWrite = stop_time();
		trials++;
	}

	*/
	// above code uses libMPSSE_I2C functions, which gave me issues communicating with the devices. I'm going to reimplement them using ftd2xx commands, namely FT_Read and FT_Write. It'll be more complex but shouldn't be that difficult an aspect. 
	
	/* Order of operations
	 * MSB First
	 * pull SDA down before falling edge of clock (channel 18) -- MPSSE command should be 0x12 for Rising edge, with length = 0x00 and data = 0x00
	 * send the following byte on rising clock edge
	 * 	[deviceAddress << 1] ## 1 (device address needs to be 7 MSB, LSB is 1 for Write, 0 for Read ==> 0x10, 0x01, 0x00, deviceAddress << 1 + 1;
	 * wait for ACK bit
	 * send registerAddress byte ==> 0x10, 0x01, 0x00, registerAddress
	 * wait for ACK bit
	 * send data, one byte at a time, with an ACK wait between each byte ==> 0x10, 0x01, 0x00, data byte 1, ACK, 0x010, 0x01, 0x00, data byte 2, etc
	 * stop write by pulling SDA high after rising clock edge -- MPSSE command should be 0x13, length = 0x00 and data = 0x01
	 */

	// pull SDA down
	// format = 0x[command][length][data] = 0x120080 - 80 at the end because the MSB is sent first, so the data byte is 1000 0000
	unsigned long int startData = 0x120080;
	void * startDataPtr = (void*)&startData;
	
	unsigned int bytesWritten = 0;
	// 3 is number of bytes to write
	ftStatus = FT_Write(deviceHandle, startDataPtr, 3, &bytesWritten);

	// sending device address
	//unsigned long int 

	return status;

}

// reads from I2C device
static FT_STATUS read_bytes(unsigned char slaveAddress, unsigned char registerAddress, unsigned char* data, unsigned long int numBytes) {
	
	FT_STATUS status = FT_OK;
	unsigned long int bytesToTransfer = 0;
	unsigned long int bytesTransferred = 0;
	unsigned long int options = 0;
	unsigned long int trials = 0;

//#if FAST_TRANSFER
	/*
	options = I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT | I2C_TRANSFR_OPTIONS_FAST_TRANSFER_BYTES;
#else
	options = I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT;
#endif
	//status = I2C_DeviceRead(deviceHandle, slaveAddress, bytesToTransfer, buffer, &bytesTransferred, options);
	trials = 0;
	while (status != FT_OK && trials < MAX_WRITE_TRIALS) {
		//status = I2C_DeviceRead(deviceHandle, slaveAddress, bytesToTransfer, buffer, &bytesTransferred, options);
		trials++;
	}
	*/
	return status;
}

unsigned int read_ADC(int adc_pin) {
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
		printf("Specified pin is not an ADC Input. Please consult references and PCB Silkscreen\n");
		return -1;
	}
	else {
		/* Reading from the ADC:
			Register address is 
			"1 [channel in binary, 0-7] 0000 
		
		*/

		// register bitmask
		unsigned char adc_register = ((0b1000 | channel) << 4);
		// need to reserve 2 bytes of space, but send the data location as a pointer to 1 byte of space, kind of hacky tbh
		unsigned int value = 0;
		void* val_ptr = (void*)&value;
		ftStatus = read_bytes(ADC_ADR, adc_register, (unsigned char*)val_ptr, 2);
		// this might need to be swapped, should return MSB first then LSB. TBD
		return value;

	}

}

void write_DAC(unsigned int value) {
	// writes a value to the DAC to output it. Double check input size on this
	// use the "Write DAC and input registers" command, = 0011xxxx
	unsigned int write_value = value << 4; // DAC reads from the 12 MSB of the two bytes, and the bottom 4 bits are don't cares. This positions the value as expected.
	// hacky stuff, see ADC write code
	void* val_ptr = (void*)&write_value;
	// just as with ADC, might need to swap order of bytes in write_value. Endianness tbd
	ftStatus = write_bytes(DAC_ADR, 0x30, (unsigned char*)val_ptr, 2);
}

unsigned char write_GPIO(int line, unsigned char value) {
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
		unsigned char data_send = 0;
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
		unsigned char data_send = 0;
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
		//ftStatus = FT_WriteGPIO(deviceHandle, int_gpio_dir, (unsigned char)value);
		return 1;
	}
	else {
		printf("Line value out of acceptable bounds. Must be between 0 and 39 inclusive\n");
		return 0;
	}

}

unsigned char read_GPIO(int line) {

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
	unsigned char adr = 0;
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

	unsigned char bitmask = 1 << (line2 - offset);

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

static FT_STATUS set_pin_dir(int pin, unsigned char dir) {
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
		// MPSSE Channel A, 
	}
	else {
		printf("Cannot write on pin %d, out of range.", pin);
	}
}
/*

static void start_library() {
	Init_libMPSSE();
}

static void clean_library() {
	Cleanup_libMPSSE();
}
*/

int main() {

	deviceHandle = malloc(sizeof(int));

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
	/*i2cConfig.ClockRate = I2C_CLOCK_STANDARD_MODE;
	i2cConfig.LatencyTimer = 250;
	i2cConfig.Options = 0;*/

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
	printf("ftStatus = %d after opening\n", ftStatus);
	if (&deviceHandle == NULL) {
		printf("Hey nerd the device handle is null\n");
	}
	printf("Device Handle = %p\n", deviceHandle);
	// gets device handle correctly, need to figure out what the deal is
	ftStatus |= FT_ResetDevice(deviceHandle);
	ftStatus |= FT_SetUSBParameters(deviceHandle, 4096, 4096);	// input and output transfers will be 4 kiB large
	ftStatus |= FT_SetChars(deviceHandle, 0, 0, 0, 0);			// disables event and error characters, documentation is kind of unclear beyond "Most applications disable these"
	ftStatus |= FT_SetTimeouts(deviceHandle, 1000, 1000);		// sets a 1 second timeout on both reading and writing to the device
	ftStatus |= FT_SetLatencyTimer(deviceHandle, 50);			// Device will wait 50 ms before sending an incomplete USB packet back to the host computer. Will adjust based on latencies of host components.
	ftStatus |= FT_SetFlowControl(deviceHandle, FT_FLOW_RTS_CTS, 0x10, 0x13); // honestly not quite sure what this does, setup guide says I need it. Will do more research in the future
	ftStatus |= FT_SetBitMode(deviceHandle, 0, 0);
	ftStatus |= FT_SetBitMode(deviceHandle, 0, 0x2);	// 0x2 sets this device as an MPSSE device, for I2C and GPIO support, instead of the default serial interface. Pin directions will be set up later

	printf("ftStatus = %d\n", ftStatus);
	// configuring the MPSSE
	// First, the start up guide suggests a test communication is sent with a deliberately bad command, and checking for the "bad command" return code.
	// based on the list of available commands, 0xF0 should be a bad command.
	unsigned char testCommand = 0xf0;
	unsigned int returnFromTest = 0;
	unsigned int bytesWritten = 0;
	unsigned int bytesRead = 0;
	ftStatus = FT_Write(deviceHandle, (void*)&testCommand, 1, &bytesWritten);
	// if the write was successful, read back
	if (ftStatus == FT_OK) {
		printf("Device successfully written to!\n");
		ftStatus = FT_Read(deviceHandle, (void*)&returnFromTest, 5, &bytesRead);
		if(ftStatus == FT_OK) {
			printf("Read occurred successfully\n");
		}
		else {
			printf("something went wrong with the read\n");
		}
		printf("returnFromTest = %x\n", returnFromTest);
		printf("bytesRead = %d\n", bytesRead);
	}

	// next, configure the MPSSE module


	// according to the libMPSSE manual initializing the I2C channel should do all of the above automatically.
	/*ftStatus = I2C_GetNumChannels(&numI2Cchannels);
	printf("num channels = %lu\n", numI2Cchannels);*/

	// test code, check channel info
	/*
	FT_DEVICE_LIST_INFO_NODE devList;
	if (numI2Cchannels > 0) {
		for (int i = 0; i < numI2Cchannels; i++) {
			ftStatus = I2C_GetChannelInfo(i, &devList);
			printf("Information on channel number %d:\n", i);
			printf("	Flags = 0x%x\n", devList.Flags);
			printf("	Type = 0x%x\n", devList.Type);
			printf("	ID = 0x%x\n", devList.ID);
			printf("	LocID = 0x%x\n", devList.LocId);
			printf("	SerialNumber = 0x%s\n", devList.SerialNumber);
			printf("	Description = 0x%s\n", devList.Description);
			printf("	ftHandle = 0x%p\n", devList.ftHandle);
		}
	}
	*/

	// set up onboard GPIO default pin directions
	// low four pins will be inputs by default, upper four pins will be outputs by default
	// direction = 0xF0
	// initial state = 0x00 // low outputs by default
	// command = 0x82
	unsigned long int dirCommand = 0xF00082;
	void * dirCmdPtr = (void*)&dirCommand;
	unsigned int bytesSent = 0;
	FT_Write(deviceHandle, dirCmdPtr, 3, &bytesSent);


	// send a test write to the GPIO Pins
	// command: same command with same direction and different values
	// sets pin 4/7 to high
	dirCommand = 0xF01082;
	dirCmdPtr = (void*)&dirCommand;
	bytesSent = 0;
	FT_Write(deviceHandle, dirCmdPtr, 3, &bytesSent);
	printf("\n\n\nCheck for output on the GPIO pin\n");
	printf("Press any key to continue...\n");
	getchar();
	
	// second test write to the GPIO Pins
	// command: same command with same direction and different values
	// sets pin 4/7 to low
	dirCommand = 0xF00082;
	dirCmdPtr = (void*)&dirCommand;
	bytesSent = 0;
	FT_Write(deviceHandle, dirCmdPtr, 3, &bytesSent);
	printf("\n\n\nCheck for output on the GPIO pin\n");
	printf("Press any key to continue...\n");
	getchar();
	
	//sends a test loop for GPIO high/low pulses with a period of 2 seconds for 10 full cycles
	for (int i = 0; i < 10; i++) {
		dirCommand = 0xF01082;
		dirCmdPtr = (void*)&dirCommand;
		bytesSent = 0;
		FT_Write(deviceHandle, dirCmdPtr, 3, &bytesSent);
		printf("Running output cycle %d...\n", i);

		sleep(1);

		dirCommand = 0xF00082;
		dirCmdPtr = (void*)&dirCommand;
		bytesSent = 0;
		FT_Write(deviceHandle, dirCmdPtr, 3, &bytesSent);

		sleep(1);

	}

	dirCommand = 0xF01082;
	dirCmdPtr = (void*)&dirCommand;
	bytesSent = 0;
	FT_Write(deviceHandle, dirCmdPtr, 3, &bytesSent);
	printf("Testing GPIO input...\n");

	dirCommand = 0x83;
	dirCmdPtr = (void*)&dirCommand;
	bytesSent = 0;
	ftStatus = FT_Write(deviceHandle, dirCmdPtr, 1, &bytesSent);
	printf("ftStatus = %d\n", ftStatus);

	//sleep(1); // making sure that the command processes, don't know the latency of the connection

	unsigned int returnVal = 0;
	void* returnPtr = (void *)&returnPtr;
	unsigned int bytesReceived = 0;
	
	ftStatus = FT_Read(deviceHandle, returnPtr, 3, &bytesReceived);
	printf("ftStatus = %d\n", ftStatus);
	printf("Return Value: 0x%x\n", returnVal);
	printf("bytes received: 0x%x\n", bytesReceived);
	
	dirCommand = 0x81;
	dirCmdPtr = (void*)&dirCommand;
	bytesSent = 0;
	ftStatus = FT_Write(deviceHandle, dirCmdPtr, 1, &bytesSent);
	printf("ftStatus = %d\n", ftStatus);

	//sleep(1); // making sure that the command processes, don't know the latency of the connection

	returnVal = 0;
	returnPtr = (void *)&returnPtr;
	bytesReceived = 0;
	
	ftStatus = FT_Read(deviceHandle, returnPtr, 3, &bytesReceived);
	printf("ftStatus = %d\n", ftStatus);
	printf("Return Value: 0x%x\n", returnVal);
	printf("bytes received: 0x%x\n", bytesReceived);
	getchar();





	/*
	int channelOpenAttempts = 0;
	while(channelOpenAttempts < 10) {
		ftStatus = I2C_OpenChannel(0, deviceHandle);
		if (ftStatus == FT_OK)
			break;
		channelOpenAttempts = channelOpenAttempts + 1;
	}
	if(ftStatus == FT_OK) {
		printf("Channel opened OK\n");
		printf("Channel Handle = %p\n", deviceHandle);
	}
	int channelConfigAttempts = 0;
	while(channelConfigAttempts < 10) {
		ftStatus = I2C_InitChannel(deviceHandle, &i2cConfig);
		if (ftStatus == FT_OK)
			break;
		channelConfigAttempts = channelConfigAttempts + 1;
	}
	if(ftStatus == FT_OK) 
		printf("Channel configured OK\n");
	*/


	
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

	unsigned int adc_config = 0x0FF8;
	void* adc_config_ptr = (void*)&adc_config;
	// writes config to ADC
	//write_bytes(ADC_ADR, ADC_CONF_REG, (unsigned char*)adc_config_ptr, 2);
	

	/*
											DAC Configuration
	-----------------------------------------------------------------------------------------------------------
	Configures the AD5691 DAC to function as intended
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
	unsigned int dac_config = 0x8000;
	void* dac_config_ptr = (void*)&dac_config;
	//write_bytes(DAC_ADR, DAC_CONF_REG, (unsigned char*)dac_config_ptr, 2);
	if(ftStatus != FT_OK) {
		printf("ftStatus is not ok, something went wrong (before)\n");
		printf("ftStatus = %x\n", ftStatus);
	}
	//write_DAC(2048);
	if(ftStatus != FT_OK) {
		printf("ftStatus is not ok, something went wrong (after) \n");
		printf("ftStatus = %x\n", ftStatus);
	}

	/*
											GPIO Configuration
	-----------------------------------------------------------------------------------------------------------
	Configures the MCP2017 I/O Expanders to function as intended
	*/

	/*	
	// Set up default configuration values in IOCON (configuration) register
	// Due to default value, this should be at address 0A. After this configuration is sent, it will be at 05.
	unsigned char gpio_config_adr = 0x0A;
	unsigned char gpio_config_data = 0x60;
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
	*/

	// close the channel
	ftStatus = FT_Close(deviceHandle);
	


}

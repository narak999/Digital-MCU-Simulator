// Digital MCU Simulator - Hardware Validation Program
// Written by: Jonathan Howard
//
// This program is intended to validate the function of the breakout board which accompanies the software portion. The board consists of 3 functions across 5 components which will be tested:
//	Two I2C GPIO Expanders
//	1 I2C ADC
//	1 I2C DAC
//	GPIO native to the USB controller
//
// The functionality of all 5 components will be tested briefly in this program as a proof of concept. A GPIO pin on each of the three sources will be activated, held on for a time, then deactivated, 
// then the DAC will produce a set voltage, and finally the ADC will read a voltage produced by an external power source.
//
// This code is heavily based on the programmer's guide put together by FTDI, the company which produces the USB Interface. Additionally, to run it needs two DLLs: the D2XX dll and the libMPSSE for I2C. 


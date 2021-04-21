#ifndef F_CPU
#define F_CPU 8000000
#endif


#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include "avr_mcu_section.h"
AVR_MCU(F_CPU, "atmega88");

const struct avr_mmcu_vcd_trace_t _mytrace[]  _MMCU_ = {
	{ AVR_MCU_VCD_SYMBOL("PORTB"), .what = (void*)&PORTB, },	
};

static int uart_putchar(char c, FILE *stream) {
	if (c == '\n')
		uart_putchar('\r', stream);
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

int
main ()
{
	// DDRB |= _BV(DDB0); 
	DDRB = 0xFF;

	stdout = &mystdout;
	printf("Initialized successfully\n");
	 
	uint16_t i;
	i = 0;
	PORTB = 0x01;
	printf("defined i successfully\n");
	while (i < 10)
	{
		PORTB = PORTB << 1;
		_delay_ms(1000);
		PORTB = PORTB << 1;
		_delay_ms(1000);
		PORTB = PORTB << 1;
		_delay_ms(1000);
		PORTB = PORTB << 1;
		_delay_ms(1000);
		PORTB = PORTB << 1;
		_delay_ms(1000);
		PORTB = PORTB << 1;
		_delay_ms(1000);
		PORTB = PORTB << 1;
		_delay_ms(1000);
		
		PORTB = PORTB >> 1;
		_delay_ms(1000);
		PORTB = PORTB >> 1;
		_delay_ms(1000);
		PORTB = PORTB >> 1;
		_delay_ms(1000);
		PORTB = PORTB >> 1;
		_delay_ms(1000);
		PORTB = PORTB >> 1;
		_delay_ms(1000);
		PORTB = PORTB >> 1;
		_delay_ms(1000);
		PORTB = PORTB >> 1;
		_delay_ms(1000);


		i++;
	}

	sleep_cpu();
}

#include <avr/io.h>

#define F_CPU 16000000UL // Crystal Freq.
#define TIMER_CLOCK 2    // Toggling Freq.

int main()
{
    DDRD = _BV(PD5);                                // set OC1A pin as output, required for output
    TCCR1A = _BV(COM1A0);                           // enable toggle OC1A output on compare
    TCCR1B = _BV(CS10) | _BV(CS11) | _BV(WGM12);    // prescaler=64, clear timer/counter on compareA match
    OCR1A = ((F_CPU / 64) / (2 * TIMER_CLOCK)) - 1; // preset timer1 high/low byte

    /*
     * Initialisation done, LED will now flash without executing any code!
     */
    while(1) // Infinite loop
    {
    }

    return 0;
}
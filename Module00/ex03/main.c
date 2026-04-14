#include <avr/io.h>
#include <util/delay.h>

int main()
{
	DDRB |= (1 << PB0);

	DDRD &= ~(1 << PD2);

	// button PD2: pressed 0; released 1;
	uint8_t last_state = (PIND & (1 << PD2));

	while (1)
	{
		// Simple debounce
		_delay_ms(20);

		// button PD2: pressed 0; released 1;
		// unint8_t: 8 bits unsigned integer type
		uint8_t current = (PIND & (1 << PD2));

		// Last state: pressed mode 0; realsed mode 1;
		if (last_state && !current)
		{
			// Debounce
			_delay_ms(200);

			// ^= Bitwise XOR : 11 -> 0; 00 -> 0; 10 -> 1;
			// if PB0 = 0 (off) ^= 1 --> 1 (on);
			// if PB0 = 1 (on) ^= 1 --> 0 (off);
			PORTB ^= (1 << PB0);
		}

		// Record current mode
		last_state = current;
	}
	return (0);
}

// Chapiter 13 I/O-Ports -- 13.2.1 Configuring the Pin -- Page59

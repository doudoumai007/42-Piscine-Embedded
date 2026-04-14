#include <avr/io.h>
#include <util/delay.h>

int main()
{
	// LEDs
	DDRB |= (1 << PB0);
	DDRB |= (1 << PB1);
	DDRB |= (1 << PB2);
	DDRB |= (1 << PB4);

	// Buttons
	DDRD &= ~(1 << PD2);
	DDRD &= ~(1 << PD4);

	// button: pressed 0; released 1;
	uint8_t SW1_state = (PIND & (1 << PD2));
	uint8_t SW2_state = (PIND & (1 << PD4));

	// 8 bits unsigned integer type
	// 0-255
	uint8_t	value = 0;

	while (1)
	{
		_delay_ms(20);

		// button: pressed 0; released 1;
		uint8_t SW1_current = (PIND & (1 << PD2));
		uint8_t SW2_current = (PIND & (1 << PD4));

		// Last state: pressed mode 0; realsed mode 1;
		if (SW1_state && !SW1_current)
		{
			_delay_ms(200);
			value++;
		}
		else if (SW2_state && !SW2_current)
		{
			_delay_ms(200);
			value--;
		}

		// Record current mode
		SW1_state = SW1_current;
		SW2_state = SW2_current;

		// Mask 4 high bits, keep 4 low bits
		// 0x0F == 15 == 0000 1111
		value &= 0x0F;

		// D1 (1st bit)
		if (value & (1 << 0))
			PORTB |= (1 << PB0);
		else
			PORTB &= ~(1 << PB0);

		//D2 (2nd bit)
		if (value & (1 << 1))
			PORTB |= (1 << PB1);
		else
			PORTB &= ~(1 << PB1);

		//D3 (3rd bit)
		if (value & (1 << 2))
			PORTB |= (1 << PB2);
		else
			PORTB &= ~(1 << PB2);

		//D4 (4th bit)
		if (value & (1 << 3))
			PORTB |= (1 << PB4);
		else
			PORTB &= ~(1 << PB4);

	}
	return (0);
}

// Chapiter 13 I/O-Ports -- 13.2.1 Configuring the Pin -- Page59

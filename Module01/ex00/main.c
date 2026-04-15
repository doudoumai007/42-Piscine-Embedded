#include <avr/io.h>


void	delay(unsigned int ms)
{
	volatile	unsigned long i;

	while (ms--)
	{
		for (i = 0; i < 8000; ++i)
			__asm__("nop");
	}
}

int	main()
{
	// Set D2 as 1 output
	DDRB |= (1 << PB1);

	while (1)
	{
		PORTB ^= (1 << PB1);

		delay(500);
	}
	return (0);
}
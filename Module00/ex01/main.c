#include <avr/io.h>

int main()
{
	// Data Direction Register pin 1 ouput; 0 input
	// |= bitwise OR : 10 -> 1; 11 -> 1; 00 -> 0;
	// Put PB0 as output
	DDRB |= (1 << PB0);

	// Level: HIGH 1; LOW 0
	PORTB |= (1 << PB0);

	while (1)
	{

	}
	return (0);
}

// Chapter 13 I/O-Ports -- 13.2.1 Configuring the Pin -- Page59
#include <avr/io.h>

int main()
{
	DDRB |= (1 << PB0);

	// HOW TO SET BOTTONS
	//1. (1 << PD2) : only put PD2 to 1
	//2. ~ : bitwise NOT, put PD2 to 0, the others to 1
	//3. Bitwise AND operation rules:
	//	1 & x → keeps the original value
	//	0 & x → becomes 0
	// 👉PD2 becomes 0, the others dont' change
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);

	while (1)
	{
		// Botton : pressed->low release->high
		// 1. (1 << PD2) : only put PD2 to 1
		// 2. & : only keep the PD2 value， the others become 0
		// 3. check if PD2 is 0 or 1 :
		// 4. 0 -> pressed; 1 -> released
		if (!(PIND & (1 << PD2)))
			PORTB |= (1 << PB0);
		else
			PORTB &= ~(1 << PB0);
	}
	return (0);
}
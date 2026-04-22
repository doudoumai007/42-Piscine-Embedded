#include <avr/io.h>

void	delay(unsigned int ms)
{
	// volatile : put the variable the RAM
	// Avoid optimisering the value in the variable when loop
	volatile	unsigned int i;

	while (ms--)
	{
		// TOTAL：19～20 cycles/loop (details👇); 16M cycles/s --> 16000 cycles/ms
		// --> 800 loops/ms
		for (i = 0; i < 800; ++i)
		// __asm__ inline assembly
		// nop : No operation but still consume one CPU cycle
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
}

/* In main.s, compile info shows: // avr-gcc -mmcu=atmega328p -S main.c
-- P625
.L4:
	nop // 1 cycle

	ldd r24,Y+1 // 2 cycles (read variable int i --> r24, which is 16-bit)
	ldd r25,Y+2 // 2 cycles
	adiw r24,1 // 2 cycles (operation "add immediate to word" to r24)
	std Y+2,r25 // 2 cycles (store variable int i, which is 16-bit)
	std Y+1,r24 // 2 cycles
.L3:
	ldd r24,Y+1 // 2 cycles
	ldd r25,Y+2 // 2 cycles
	cpi r24,32 // 1 cycle
	ldi r18,3 // 1 cycle
	cpc r25,r18 // 1 cycle
	brlo .L4 // 1~2 cycle
*/
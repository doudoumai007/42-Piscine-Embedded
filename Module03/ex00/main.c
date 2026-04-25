#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BLUE PD3
#define RED PD5
#define GREEN PD6


// Set Interrupt Service Routine
__attribute__((signal))
void TIMER1_COMPA_vect(void)
{
	static uint8_t color = 0;

	switch (color)
	{
		case 0:
			PORTD = (1 << RED);
			break ;
		case 1:
			PORTD = (1 << GREEN);
			break ;
		case 2:
			PORTD = (1 << BLUE);
			break ;
	}

	++color;
	if (color == 3)
		color = 0;
}

void	auto_interrupt()
{
	// CTC (Clear Timer on Compare Match) mode + interrpution
	// 🟢 Set CTC mode 4: WGM13: 0; WGM12:1; WGM11:0; WGM10:0
	TCCR1B |= (1 << WGM12);

	// 🟢 Set prescaler 1024
	TCCR1B |= (1 << CS12) | (1 << CS10);

	// 🟢 Set TOP: OCR1A = frequence * 1s - 1
	OCR1A = (F_CPU / 1024)  - 1;

	// 🟢 Set Timer/Counter1 Interrupt Mask --P145
	// Output Compare A Match Interrupt Enable
	TIMSK1 |= (1 << OCIE1A);
}

int	main()
{
	// 🟢 Enable global interrpution --P20
	// SREG: Status Register
	SREG |= (1 << 7);

	// Set LEDs as output
	DDRD |= (1 << BLUE) | (1 << RED) | (1 << GREEN);

	// Set interrupt timer
	auto_interrupt();

	while (1);
}
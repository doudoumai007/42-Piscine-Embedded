#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t duty_cycle = 0;
volatile uint8_t up = 1; // Going up from

// __attribute__ ((signal, used, externally_visible))
// void	TIMER0_COMPA_vect()
// __attribute__ ((signal, used, externally_visible))
// void    __vector_16(void)
__attribute__((interrupt(0x00E))) // --P66
void f(void)
{
	if (up == 1)
	{
		OCR1A = duty_cycle;
		if (duty_cycle == 255)
			up = 0;
		else
			++duty_cycle;
	}
	else
	{
		OCR1A = duty_cycle;
		if (duty_cycle == 0)
			up = 1;
		else
			--duty_cycle;
	}
}

void	init_timer0()
{
	// Timer0 --for CTC working mode
	// 🟢 Enable Timer/Counter1 Output Compare A Match interrupt
	TIMSK0 |= (1 << OCIE0A);

	// 🟢 Set CTC mode 4 + prescaer
	TCCR0B = (1 << CS02) | (1 << CS00);
	TCCR0A = (1 << WGM01);

	// 🟢 Set TOP: OCR0A = how many ticks in 1/512s
	OCR0A = (F_CPU / 1024) / 512;
}

void	init_timer1()
{
	// Timer1 --for Fast PWM + TOP:ICR1 (mode 14)
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12);

	// 🟢 Compare Output Mode for PWM
	TCCR1A |= (1 << COM1A1);

	// Set Timer1‘s prescaler = 1024
	TCCR1B |= (1 << CS12) | (1 << CS10);

	// Set ICR1 (TOP)
	ICR1 = 255;
}

int	main()
{
	// LED D2(OC1A)
	DDRB |= (1 << PB1);

	// 🟢 Enable global interrpution --P20
	// SREG: Status Register
	SREG |= (1 << 7);

	init_timer0();
	init_timer1();

	while (1);
}

// https://gcc.gnu.org/onlinedocs/gcc/AVR-Attributes.html

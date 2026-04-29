#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t button_pressed = 0;

// __attribute__ ((signal, used, externally_visible))
// void	INT0_vect()
// __attribute__((signal, used, externally_visible))
// void    __vector_1(void)
__attribute__((interrupt (0x001))) // INT0 vector --P66
void f1(void)
{
	// Disable INT0 to avoid multiple triggers
	EIMSK &= ~(1 << INT0);

	// Debounce
	// Reset Timer1
	TCNT1 = 0;

	// 🟢 Set CTC mode 4 + prescaer
	TCCR1B = (1 << CS12) | (1 << CS10) | (1 << WGM12);
}


// __attribute__ ((signal, used, externally_visible))
// void	TIMER1_COMPA_vect()
// __attribute__((signal, used, externally_visible))
// void    __vector_11(void)
__attribute__ ((interrupt (0x00B))) // TIMER1 COMPA vector --P66
void f2(void)
{
	// 🟢 Stop CTC mode
    TCCR1B = 0;
	// 🟢 Reset Timer1
	TCNT1 = 0;
    
    if (!button_pressed)
	{
		// Switch turn on / off
		PORTB ^= (1 << PB0);

		// Set INT0 to trigger on rising edge -- P80
		EICRA = (1 << ISC00) | (1 << ISC01);
		button_pressed = 1;
	}
	else
	{
		// When detect release of button, reset button
		button_pressed = 0;

		// 🟢 Switch INT0 back to falling edge
		EICRA = (1 << ISC01);

	}
	// 🟢 Clear INT0 interrupt flag
	EIFR |= (1 << INTF0);
	// 🟢 Re-enabe INT0
	EIMSK |= (1 << INT0);
}

int	main()
{

	// Led output
	DDRB |= (1 << PB0);

	// Button input, level high for start
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);

	// 🟢 Enable global interrpution --P20
	// SREG: Status Register
	SREG |= (1 << 7);

	// 🟢 Enable external interrupt, falling edge of INT0
	// PD2 -> INT0 --P98 ( The PD2 pin can serve as an external interrupt source)
	// Interrupt 0 Sense Control --P80
	// The falling edge of INT0 generates an interrupt request: ISC01:1; ISC00:0
	EICRA = (1 << ISC01);

	// 🟢 Enable external interrupt Request 0
	EIMSK = (1 << INT0);

	// 🟢 Enable Timer/Counter1 Output Compare A Match interrup
	TIMSK1 |= (1 << OCIE1A);

	// 🟢 Set TOP: OCR1A = precaler * 20ms - 1
	OCR1A = (F_CPU / 1024) / 50 - 1;

	while (1);
}

// https://gcc.gnu.org/onlinedocs/gcc/AVR-Attributes.html

#include <avr/io.h>
#include <util/delay.h>

int	main()
{
	// duty cycle 10% 1Hz: turn on 100ms, turn off 900ms 

	// In Timer1 there are 2 Output Compare channels: A B (OC1A / OC1B)
	// OC1A is served on PB1
	// The PB1 pin can serve as an external output for the Timer/Counter1 Compare Match A.
	// The PB1 pin has to be configured as an output (DDB1 set (one)) to serve this function. -- P92
	DDRB |= (1 << PB1);

	// Set the registers to control the modes Fast PWM + Top:ICR1 --> Mode 14 --P142
	// TCCR1A : WGM11:1 WGM10:0;
	// TCCR1B : WGM13:1 WGM12:1
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12);

	// COM1A1:1 COM1A0:0 --P140
	// Clear OC1A/OC1B on Compare Match, set at BOTTOM (non-inverting mode)
	// --> Turn off at the TOP and turn on at the BOTTOM
	TCCR1A |= (1 << COM1A1);

	// Set Timer1‘s prescaler = 1024
	TCCR1B |= (1 << CS12) | (1 << CS10);

	// Set ICR1 = F_CPU / prescaler (TOP)
	ICR1 = F_CPU/1024;

	// Output Compare Register : set high level time
	// 10% duty cycle -> ICR1*0.1
	// uint32_t to avoid overflow
	OCR1A = (uint32_t)ICR1 * 10 / 100;

	// Buttons
	DDRD &= ~(1 << PD2);
	DDRD &= ~(1 << PD4);

	// button: pressed 0; released 1;
	uint8_t SW1_state = (PIND & (1 << PD2));
	uint8_t SW2_state = (PIND & (1 << PD4));

	uint8_t duty = 10;

	while (1)
	{
		// button: pressed 0; released 1;
		uint8_t SW1_current = (PIND & (1 << PD2));
		uint8_t SW2_current = (PIND & (1 << PD4));

		// Debounce & get button status again
		_delay_ms(20);
		SW1_current = (PIND & (1 << PD2));
		SW2_current = (PIND & (1 << PD4));
	
		// Last state: pressed mode 0; realsed mode 1;
		if (SW1_state && !SW1_current)
		{
			if (duty <= 90)
				duty += 10;
			OCR1A = (uint32_t)ICR1 * duty / 100;
		}
		else if (SW2_state && !SW2_current)
		{
			if (duty >= 20)
				duty -= 10;
			OCR1A = (uint32_t)ICR1 * duty / 100;
		}

		// Record current mode
		SW1_state = SW1_current;
		SW2_state = SW2_current;
	}
}
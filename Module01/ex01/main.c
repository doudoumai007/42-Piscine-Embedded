#include <avr/io.h>

int	main()
{
	// In Timer1 there are 2 Output Compare channels: A B (OC1A / OC1B)
	// OC1A is served on PB1
	// The PB1 pin can serve as an external output for the Timer/Counter1 Compare Match A.
	// The PB1 pin has to be configured as an output (DDB1 set (one)) to serve this function. -- P92
	DDRB |= (1 << PB1);

	// 🟢 Set Timer1 CTC mode (16 mode totally) --P141
	// CTC mode: Clear Timer on Compare Match Mode --P131
	// To put CTC mode, need to put WGM12 1 --P141
	// WGM : Waveform Generation Mode
	// WGM10 WGM11 -> TCCR1A; WGM12 WGM13 -> TCCR1B(Timer/Counter control Register B)
	TCCR1B |= (1 << WGM12);

	// 🟢 Toggle OC1A(PB1) every time when it matchhes
	// non-PWM : COM1A1 0; COM1A0 1;  --P140
	// COM1A1:0: Compare Output Mode for Channel A -> TCCR1A
	TCCR1A |= (1 << COM1A0);

	// 🟢 Set Timer1‘s prescaler = 1024
	// Why 1024?
	// Timer1（16-bit）max count 2^16 = 65536 ticks
	// if we use 1024 as prescaler : 16MHz / 1024 = 15625 Hz
	// We can use timer1 to do around : 65536/15625 ≈ 4.19s
	// Clock Select Bit Description: CS12 1; CS11 0; CS10 1 -> TCCR1B; --P143
	TCCR1B |= (1 << CS12) | (1 << CS10);

	// 🟢 Output Compare Register : set target value
	// Timer frequency = F_CPU/prescaler
	// OCR1A = frequency * time
	// In CTC mode, the counter TCNT1's maximum value (TOP) is defined by OCR1A --P131
	OCR1A = (F_CPU/1024)*0.5;

	while (1)
	{

	}
}
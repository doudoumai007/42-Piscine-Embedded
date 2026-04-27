#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BLUE PD3
#define RED PD5
#define GREEN PD6

void set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	// OC0A -> PD6; OC0B -> PD5; OC2B -> PD3 --P97
	// Change the duty cycle by wheel --P103
	// because OCR set the he switching threshold between high and low when compare with the timer
	// when TCNT0 < OCR0A -> OC0A high;
	// when TCNT0 >= OCR0A -> OC0A low
	// for example if we set OCR0A as 10, 10/255 time level is high, 245/255 time level is low
	OCR0A = g;
	OCR0B = r;
	OCR2B = b;
}

void wheel(uint8_t pos)
{
	pos = 255 - pos;
	if (pos < 85)
	{
		set_rgb(255 - pos * 3, 0, pos * 3);
	}
	else if (pos < 170)
	{
		pos = pos - 85;
		set_rgb(0, pos * 3, 255 - pos * 3);
	}
	else
	{
		pos = pos - 170;
		set_rgb(pos * 3, 255 - pos * 3, 0);
	}
}

void	init_rgb()
{
	// 🟢 Set Fast PWM mode TOP 0xFF 
	// OC0A -> PD6; OC0B -> PD5; OC2B -> PD3 --P97
	// Fast PWM TOP 0xFF for Timer0: Mode3 WGM02:0 WGM01:1 WGM00:1  --P115
	// Fast PWM TOP 0xFF for Timer2: Mode3 WGM22:0 WGM21:1 WGM20:1  --P164
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	TCCR2A |= (1 << WGM21) | (1 << WGM20);

	// 🟢 Set Compare Output Mode, Fast PWM Mode
	// Clear OC0A on Compare Match, set OC0A at BOTTOM, (non-inverting mode). --P113
	// Clear OC0B on Compare Match, set OC0B at BOTTOM, (non-inverting mode) --P114
	// Clear OC2B on Compare Match, set OC2B at BOTTOM, (non-inverting mode). --P163
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	TCCR2A |= (1 << COM2B1);

	// 🟢 No prescaler because in Fast PWM Mode:
	// frequency = F_CPU/256 = 62500 < 65536, so no overfloat
	TCCR0B |= (1 << CS00); // --P117
	TCCR2B |= (1 << CS20); // --P165

	// Initializer duty cycle --P103
	OCR0A = 0;
	OCR0B = 0;
	OCR2B = 0;
}

int	main()
{
	// Set LEDs as output
	DDRD |= (1 << BLUE) | (1 << RED) | (1 << GREEN);

	// Initializer LEDs mode
	init_rgb();

	uint8_t pos = 0;
	while (1)
	{
		wheel(pos);
		// loop of 256 -> 256*50ms = 12.8s/loop
		_delay_ms(50);
		++pos;
	}
}
#include "led.h"



void set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	//PCINT19/OC2B/INT1) PD3 = blue
	//(PCINT21/OC0B/T1) PD5 = red
	//(PCINT22/OC0A/AIN0) PD6 = green

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

void init_rgb()
{
	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6);
	OCR0A = 0;
	OCR0B = 0;
	OCR2B = 0;


	TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);//(p115), fast pwm with top 0xFF and activate OC0A/B
	TCCR0B = (1 << CS00);//no prescale

	TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);//(p115), fast pwm with top 0xFF and activate OC2B1
	TCCR2B = (1 << CS20);

}

int main()
{
	uint8_t pos = 0;

	init_rgb();

	while(1)
	{
		_delay_ms(50);
		wheel(pos ++);		
	}
}

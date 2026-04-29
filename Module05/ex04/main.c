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

void	uart_tx(char c)
{
	// UDREn: Data Register Empty
	// it indicates whether the transmit buffer is ready to receive new data
	// 1: the transmit buffer is empty； 0: still has data to transmit --P187
	while (!(UCSR0A & (1 << UDRE0)));

	// UDR: Data Register
	// 2 roles: transmit & receive
	UDR0 = c;
}

void	uart_init()
{
	// USART Initialization --P184

	// 🟢 Set Baude Rate in the register
	// ATmega328P has only one UART -> USART0
	// For precision raison we choose Asynchronous Double Speed mode --P182
	// add UL to avoid overfloat (baude rate saved as int -> 16bit)
	uint16_t ubrr = F_CPU / (8UL * UART_BAUDERATE) - 1;
	// 8 high bits
	UBRR0H = (ubrr >> 8);
	// 8 low bits
	UBRR0L = ubrr;

	// Set Double Speed asynchronous mode --P182
	// Control and status register
	UCSR0A |= (1 << U2X0);

	// 🟢 Enable the Transmitter
	UCSR0B = (1 << TXEN0);

	// 🟢 Set Frame Format : 8N1 -> Data bits 8 + Parity bit none + Stop bits 1 --P183
	// We can send Frames with 5 to 8 bits, USART Character Size is controled by 3 bits
	// bit 0 UCSZ00; bit 1 UCSZ01; size 9 -> UCSZ02
	// 8-bit size -> UCSZn2:0; UCSZn1:1; UCSZn0:1 --P203 (Table 20-11)
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

void	adc_init()
{
	// 🟢 Enable ADC (Analog-to-Digital Converter)
	// ADCSRA (ADC Control and Status Register A)
	ADCSRA |= (1 << ADEN);

	// 🟢 Set prescaler 128 have a frequence 50 kHz - 200 kHz
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	// ADMUX (ADC Multiplexer Selection Register)
	// Voltage Reference Selections for ADC : --P257
	// 🟢 Enable AVCC with external capacitor at AREF pin
	ADMUX |= (1 << REFS0);

	// 🟢 Enable left adjust the result (ADLAR = 1) -> 8-bit resolution--P257
	// ADCL and ADCH (（ADC Data Register Low/ HIGH） --P259
	ADMUX |= (1 << ADLAR);
}

uint8_t	read_adc()
{
	// Enable ADSC (Adc Start Convertion) -- -P258
	ADCSRA |= (1 << ADSC);

	// Wait until the conversion is complete, ADSC will turn 0 --P258
	while (ADCSRA & (1 << ADSC));

	//Return 8-bit
	return (ADCH);
}

void	print_hex(uint8_t value)
{
	const char hex[17] = "0123456789abcdef";
	// High 4-bit bitwise right 4 -> High 4-bit
	uart_tx(hex[value >> 4]);
	// bitwise AND 0000 1111 -> High 4-bit become 0000 -> Low 4-bit
	uart_tx(hex[value & 0x0f]);
	uart_tx('\n');
	uart_tx('\r');
}

void	leds(uint8_t value)
{
	if (value < 64)
		PORTB = 0;
	else if (value >= 64 && value < 128)
		PORTB =  (1 << PB0);
	else if (value >= 128 && value < 192)
		PORTB = (1 << PB0) | (1 << PB1);
	else if (value >= 192 && value < 255)
		PORTB = (1 << PB0) | (1 << PB1) | (1 << PB2);
	else
		PORTB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);
}

int	main()
{
	// Set LED RGB as output
	DDRD |= (1 << BLUE) | (1 << RED) | (1 << GREEN);

	//Set Leds
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);

	// Initializer LEDs mode
	init_rgb();

    adc_init();
	uart_init();

    while (1)
	{
		uint8_t value = read_adc();
		wheel(value);
		leds(value);
		print_hex(value);
		_delay_ms(20);
	}
}
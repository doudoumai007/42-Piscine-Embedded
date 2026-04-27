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

	OCR0A = 0;
	OCR0B = 0;
	OCR2B = 0;
}

void	uart_tx(unsigned char c)
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

	// Set Double Speed asynchronous mode --P182-/*-
	// Control and status register
	UCSR0A |= (1 << U2X0);

	// 🟢 Enable the Transmitter and Receiver --P188
	// 🟢 Enable Receive Complete Interrupt --P191
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);

	// 🟢 Set Frame Format : 8N1 -> Data bits 8 + Parity bit none + Stop bits 1 --P183
	// We can send Frames with 5 to 8 bits, USART Character Size is controled by 3 bits
	// bit 0 UCSZ00; bit 1 UCSZ01; size 9 -> UCSZ02
	// 8-bit size -> UCSZn2:0; UCSZn1:1; UCSZn0:1 --P203 (Table 20-11)
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

volatile	char buffer[7];
volatile	uint8_t i = 0;
void	parse_buffer()
{
	if (buffer[0] != '#')
		return ;

	for (uint8_t i = 1; i < 7; ++i)
	{
		if (!((buffer[i] >= 'A' && buffer[i] <= 'F') || (buffer[i] >= '0' && buffer[i] <= '9')))
			return ;
	}

	uint8_t value[7];
	value[0] = 0;

	for (uint8_t i = 1; i < 7; ++i)
	{
		if (buffer[i] >= 'A' && buffer[i] <= 'F')
			value[i] = buffer[i] - 55;
		else if (buffer[i] >= '0' && buffer[i] <= '9')
			value[i] = buffer[i] - 48;
	}

	uint8_t r = value[1] * 16 + value[2];
	uint8_t g = value[3] * 16 + value[4];
	uint8_t b = value[5] * 16 + value[6];

	set_rgb(r, g, b);
}

// (signal) put interrupt service routine
__attribute__((signal))
void USART_RX_vect(void)
{

	unsigned char c = UDR0;
	uart_tx(c);

	if (c != '\r')
	{
		if (i < 7)
			buffer[i] = c;
		i++;
	}
	else
	{
		if (i == 7)
		{
			buffer[i] = '\0';
			parse_buffer();
		}
		i = 0;
	}
}

int	main()
{
	// Set LEDs as output
	DDRD |= (1 << BLUE) | (1 << RED) | (1 << GREEN);

	// Initializer LEDs mode
	init_rgb();

	// Set interrupt timer
	uart_init();

	// 🟢 Enable global interrpution --P20
	// SREG: Status Register
	SREG |= (1 << 7);

	while (1);
}
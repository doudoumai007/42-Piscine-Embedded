#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

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

	// 🟢 Enable right adjust the result (ADLAR = 0) -> 10-bit resolution--P257
	// ADCL and ADCH (（ADC Data Register Low/ HIGH） --P259
}

uint16_t	read_adc(uint8_t channel)
{
	// Enable channel 0/1/2 --P258
	// (ADMUX & 0xF0) -> keep High 4-bit
	// (channel & 0x0F) -> choose channel in Low 4-bit
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	
	// Enable ADSC (Adc Start Convertion) -- -P258
	ADCSRA |= (1 << ADSC);

	// Wait until the conversion is complete, ADSC will turn 0 --P258
	while (ADCSRA & (1 << ADSC));

	// Read 8-low-bit and 8-high-bit
	uint8_t low = ADCL;
	uint8_t high = ADCH;

	uint16_t value = (uint16_t)(high << 8) | low;

	//Return 16-bit
	return (value);
}

void	print_decimal(uint16_t value)
{
	if (value == 0)
	{
		uart_tx('0');
		return ;
	}
	// Max 1023 + '\0'
	char buffer[5];
	uint8_t i = 0;
	while (value > 0)
	{
		buffer[i++] = (value % 10) + '0';
		value /= 10;
	}
	while (i--)
		uart_tx(buffer[i]);
}

int	main()
{
    adc_init();
	uart_init();

    while (1)
	{
		//RV1 -> PC0/ADC0 --P94
		uint16_t value = read_adc(0);
		print_decimal(value);
		uart_tx(',');
		uart_tx(' ');
		//LDR -> PC1/ADC1
		value = read_adc(1);
		print_decimal(value);
		uart_tx(',');
		uart_tx(' ');
		//NTC -> PC2/ADC2
		value = read_adc(2);
		print_decimal(value);
		uart_tx('\n');
		uart_tx('\r');
		_delay_ms(20);
	}
}
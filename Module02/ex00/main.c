#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void	uart_tx(char c)
{

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
	UCSR0A |= (1 << U20N);

	// 🟢 Enable the Transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);

	// 🟢 Set Frame Format
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);



}

int	main()
{
	uart_init();

	while (1)
	{
		uart_tx('Z');
		_delay_ms(1000);
	}
}

// screen /dev/ttyUSB0 115200

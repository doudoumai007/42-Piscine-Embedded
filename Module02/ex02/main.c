#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Blocking, polling-based UART
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

char uart_rx(void)
{
	// RXCn: USART Receive Complete
	// 1: there are unread data in the receive buffer；
	// 0: the receive buffer is empty --P187
	while (!(UCSR0A & (1 << RXC0)));

	return (UDR0);
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
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);

	// 🟢 Set Frame Format : 8N1 -> Data bits 8 + Parity bit none + Stop bits 1 --P183
	// We can send Frames with 5 to 8 bits, USART Character Size is controled by 3 bits
	// bit 0 UCSZ00; bit 1 UCSZ01; size 9 -> UCSZ02
	// 8-bit size -> UCSZn2:0; UCSZn1:1; UCSZn0:1 --P203 (Table 20-11)
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

int	main()
{
	uart_init();

	unsigned char	c;

	while (1)
	{
		c = uart_rx();
		uart_tx(c);
	}
}

// screen /dev/ttyUSB0 115200
// cat /dev/ttyUSB0
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

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

unsigned char	toggle_case(unsigned char c)
{
	if (c >= 'A' && c <= 'Z')
		c += 32;
	else if (c >= 'a' && c <= 'z')
		c -= 32;
	return (c);
}

// // Interrupt Service Routine
// ISR(USART_RX_vect)
// {
// 	unsigned char c = UDR0;
// 	c = toggle_case(c);
// 	uart_tx(c);
// }

// (signal) put interrupt service routine
__attribute__((signal))
void USART_RX_vect(void)
// __attribute__ ((interrupt (0x012))) // USART, RX --P66 ?
// void f2(void)
{
	// Interrupt-driven receive, UDR0 has already accomplished reception
	unsigned char c = UDR0;
	c = toggle_case(c);
	uart_tx(c);
}


int	main()
{
	uart_init();

	// 🟢 Enable global interrpution --P20
	// SREG: Status Register
	SREG |= (1 << 7);

	while (1);
}

// screen /dev/ttyUSB0 115200
// cat /dev/ttyUSB0
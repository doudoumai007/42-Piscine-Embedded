#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void uart_printstr(const char* str)
{
	// UDREn: Data Register Empty
	// it indicates whether the transmit buffer is ready to receive new data
	// 1: the transmit buffer is empty； 0: still has data to transmit --P187
	if (!str[0])
		return ;

	uint8_t	i = 0;
	while (str[i])
	{
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = str[i];
		++i;
	}
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

	// 🟢 Enable the Transmitter
	UCSR0B = (1 << TXEN0);

	// 🟢 Set Frame Format : 8N1 -> Data bits 8 + Parity bit none + Stop bits 1 --P183
	// We can send Frames with 5 to 8 bits, USART Character Size is controled by 3 bits
	// bit 0 UCSZ00; bit 1 UCSZ01; size 9 -> UCSZ02
	// 8-bit size -> UCSZn2:0; UCSZn1:1; UCSZn0:1 --P203 (Table 20-11)
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

void	auto_interrupt()
{
	// CTC (Clear Timer on Compare Match) mode + interrpution
	// 🟢 Set CTC mode 4: WGM13: 0; WGM12:1; WGM11:0; WGM10:0
	TCCR1B |= (1 << WGM12);

	// 🟢 Set prescaler 1024
	TCCR1B |= (1 << CS12) | (1 << CS10);

	// 🟢 Set TOP: OCR1A = precaler * 2s - 1
	OCR1A = (F_CPU / 1024) * 2 - 1;

	// 🟢 Set Timer/Counter1 Interrupt Mask --P145
	// Output Compare A Match Interrupt Enable
	TIMSK1 = (1 << OCIE1A);
}

// // Interrupt Service Routine
// ISR(TIMER1_COMPA_vect)
// {
// 	uart_printstr("Hello World!\r\n");
// }

// Set VectorNo.12 --P74 
// (0x0016 (bytes)-> but 0x000B (word) in real， because in vector use word index = 2 bytes)
// Link the fonciton to the interrupt's address
// __attribute__((interrupt(0x000B)))
// Set Interrupt Service Routine
__attribute__((signal))
void TIMER1_COMPA_vect(void)
{
	uart_printstr("Hello World!\r\n");
}

int	main()
{
	uart_init();

	// 🟢 Enable global interrpution --P20
	// SREG: Status Register
	SREG |= (1 << 7);

	auto_interrupt();

	while (1);
}

// screen /dev/ttyUSB0 115200
// cat /dev/ttyUSB0
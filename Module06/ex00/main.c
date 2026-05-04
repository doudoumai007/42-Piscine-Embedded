#include <avr/io.h>
#include <util/twi.h>

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

void	print_hex(uint8_t value)
{
	const char hex[17] = "0123456789abcdef";
	// High 4-bit bitwise right 4 -> High 4-bit
	uart_tx(hex[value >> 4]);
	// bitwise AND 0000 1111 -> High 4-bit become 0000 -> Low 4-bit
	uart_tx(hex[value & 0x0f]);
}

void	print_status(uint8_t status)
{
	uart_printstr("0x");
	print_hex(status);
	uart_printstr("\r\n");
}

void	i2c_write(uint8_t data)
{
	TWDR = data;
	
	// Send the address or the commande
	// Set TWSTA(start) condition and clear TWINT(interrupt) Flag --P226
	TWCR = (1 << TWEN) | (1 << TWINT);

	// Wait until communication finishes
	while (!(TWCR & (1 << TWINT)));

	// Read TWSP status (5-high-bit -> status) --P240
	uint8_t status = TWSR & 0xF8; // TWSR & (1111 1000)
	print_status(status);
}

void	i2c_start(void)
{
	// Set TWSTA(start) condition and clear TWINT(interrupt) Flag --P226
	TWCR = (1 << TWEN) | (1 << TWSTA) | (1 << TWINT);

	// Wait until communication finishes
	while (!(TWCR & (1 << TWINT)));

	// Read TWSP status (5-high-bit -> status) --P240
	uint8_t status = TWSR & 0xF8; // TWSR & (1111 1000)
	print_status(status);

	// Check TW if START was sent
	if (status == 0x08)
	// Enter Master Transmitter Mode --P227
	// 0x38:address -- first 7 bit; last bit: 0(read); 1(write);
		i2c_write(0x38 << 1);
}


void	i2c_stop(void)
{
	// Set TWSTO(STOP) condition --P227
	TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);
}


void	i2c_init(void)
{
	// 🟢 Set prescaler = 1 -> TWPS1:0;TWPS0:0 --P241
	// TWSR (Status Register)
	TWSR = 0;

	// 🟢 Set SCL 100kHz
	// SCL = F_CPU / (16 + 2 * TWBR * prescaler) -> TWBR = 72 --P222
	// TWBR (Control Register)
	TWBR = 72;

	// 🟢 Enable TWI --P226
	TWCR = (1 << TWEN);
}

int	main()
{
	uart_init();

	// i2c: 2-wire Serial Interface -> TWI in avr
	// SDA（Serial Data Line）& SCL（Serial Clock Line）
	// SCL: control the timing of the communication by providing the clock signal
	// SDA: transmit the actual data (address, data, and ACK)
	// P223 -> Interfacing the Application to the TWI in a Typical Transmission
	i2c_init();
	i2c_start();
	i2c_stop();
}
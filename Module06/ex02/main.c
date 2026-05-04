#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <stdlib.h> //dtostrf

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

void	print_hex_value(char c)
{
	const char hex[17] = "0123456789ABCDEF";
	// High 4-bit bitwise right 4 -> High 4-bit
	char high = (hex[(c >> 4) & 0X0f]);
	// bitwise AND 0000 1111 -> High 4-bit become 0000 -> Low 4-bit
	char low = hex[c & 0x0f];
	uart_tx(high);
	uart_tx(low);
}

void	i2c_write(unsigned char data)
{
	TWDR = data;
	
	// Send the address or the commande
	// Enable TWI and clear TWINT(interrupt) Flag --P226
	TWCR = (1 << TWEN) | (1 << TWINT);

	// Wait until communication finishes
	while (!(TWCR & (1 << TWINT)));
}

void	i2c_start(void)
{
	// Set TWSTA(start) condition and clear TWINT(interrupt) Flag --P226
	TWCR = (1 << TWEN) | (1 << TWSTA) | (1 << TWINT);

	// Wait until communication finishes
	while (!(TWCR & (1 << TWINT)));
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

uint8_t	i2c_read_nack()
{
	// Send the address or the commande
	// Enable TWI and clear TWINT(interrupt) Flag --P226
	TWCR = (1 << TWEN) | (1 << TWINT);

	// Wait until communication finishes
	while (!(TWCR & (1 << TWINT)));

	return (TWDR);
}

uint8_t	i2c_read_ack()
{
	// Send the address or the commande
	// Enable TWI and clear TWINT(interrupt) Flag --P226
	TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);

	// Wait until communication finishes
	while (!(TWCR & (1 << TWINT)));

	return (TWDR);
}

// AHT20 Datasheet --P12
void	power_on_check()
{
	_delay_ms(100);
	i2c_start();
	// SLA + R
	// 0x38:address -- first 7 bit; last bit: 0(read); 1(write);
	i2c_write(0x38 << 1 | 1);
	uint8_t status = i2c_read_nack();
	if ((status & 0x18) != 0X18)
	{
		i2c_start();
		// SLA + W
		i2c_write(0x38 << 1);
		i2c_write(0x1B);
		i2c_write(0x1C);
		i2c_write(0x1E);
		i2c_stop();
	}
}

uint8_t	idx = 0;
uint8_t count = 0;
float hum_arr[3];
float temp_arr[3];

void	update_data(float hum, float temp)
{
	hum_arr[idx] = hum;
	temp_arr[idx] = temp;
	// Built a loop of 0 1 2
	// Replace the oldest one with the lastest one
	idx = (idx + 1) % 3;
	++count;
}

float	average_data(float* arr)
{
	float sum = 0;
	for (uint8_t i = 0; i < 3; ++i)
		sum += arr[i];
	float average = sum / 3.0;
	return (average);
}

void	i2c_read()
{
	i2c_start();
	// Trigger Measurement --P12 Dadasheet AHT20
	// command(0xAC) followed by required parameters (0x33, 0x00)
	// SLA + W
	i2c_write(0X38 << 1);
	i2c_write(0xAC); 
	i2c_write(0x33);
	i2c_write(0x00);
	i2c_stop();

	// Wait 80ms for completing the Measurement
	_delay_ms(80);

	// Check Measurement complished
	uint8_t status;
	do
	{
		i2c_start();
		// SLA + R
		i2c_write((0X38 << 1) | 1);
		status = i2c_read_nack();
		i2c_stop();
		_delay_ms(10);
	}
	while (status & 0x80); // 7-bit should be 1 to realse

	// Read 7 byte in the register of the sensor
	uint8_t	buffer[7] = {0};
	i2c_start();
	// SLA + R
	i2c_write((0X38 << 1) | 1);
	for (uint8_t i = 0; i < 7; ++i)
	{
		// First 6 times: read and continue
		if (i < 6)
			buffer[i] = i2c_read_ack();
		else
			buffer[i] = i2c_read_nack();
	} 
	i2c_stop();

	// Analyze Data
	// byte0: status
	// byte1: humidity high
	// byte2: humidity mid
	// byte3: humidity low + temperature high
	// byte4: temperature mid
	// byte5: temperature low
	// Huminity 20-bit
	// Temperature 20-bit
	uint32_t raw_h = ((uint32_t)buffer[1] << 12) | ((uint32_t)buffer[2] << 4) | ((uint32_t)buffer[3] >> 4);
	uint32_t raw_t = (((uint32_t)buffer[3] & 0x0f) << 16) | ((uint32_t)buffer[4] << 8) | (uint32_t)buffer[5];

	// Relative Humidity Conversion & Temperature Conversion --P13
	// Choose 0.1 because the accuracy is ±0.3°C & ±2%RH --P2
	float hum = (float)raw_h / 1048576.0 * 100.0;
	float temp = (float)raw_t / 1048576.0 * 200.0 - 50;

	// Update data and get average 
	update_data(hum, temp);
	// Ignore the first 2 times
	if (count < 3)
		return ;
	float average_h = average_data(hum_arr);
	float average_t = average_data(temp_arr);

	char h_str[15];
	char t_str[15];
	// char *dtostrf(double val, signed char width, unsigned char prec, char *s);
	dtostrf(average_h, 4, 1, h_str);
	dtostrf(average_t, 4, 1, t_str);

	// Print data
	uart_printstr("Temperature: ");
	uart_printstr(t_str);
	uart_printstr(".C, Humidity: ");
	uart_printstr(h_str);
	uart_printstr("%\r\n");
}

int	main()
{
	uart_init();

	// i2c: 2-wire Serial Interface -> TWI in avr
	// SDL（Serial Data Line）& SCL（Serial Clock Line）
	// SCL: control the timing of the communication by providing the clock signal
	// SDA: transmit the actual data (address, data, and ACK)
	// P223 -> Interfacing the Application to the TWI in a Typical Transmission
	i2c_init();
	power_on_check();
	while (1)
	{
		i2c_read();
		// data collection cycle should be greater than 1 second/time. --P7
		_delay_ms(2000);
	}
}
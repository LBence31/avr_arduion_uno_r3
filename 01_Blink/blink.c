#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

// DDRx  - configure PORT as input/output
// PORTx - write data to the port pins
// PINx  - read data from the port pins

#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

void uart_init(unsigned int ubrr)
{
    UBRR0H = (ubrr >> 8);
    UBRR0L = (ubrr);
    UCSR0B = (1 << TXEN0);                  // Enable TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data
}

void uart_send(const char *s)
{
    while (*s)
    {
        while (!(UCSR0A & (1 << UDRE0)))
            ; // Wait until buffer empty
        UDR0 = *s++;
    }
}

int main(void)
{
    uart_init(MYUBRR);

    DDRB |= (1 << PB5); // pin 13 output
    while (1)
    {
        PORTB ^= (1 << PB5);
        uint8_t val = (PINB & (1 << PB5)) ? 1 : 0;
        char buf[8];
        sprintf(buf, "PB5=%d\r\n", val);
        uart_send(buf);
        _delay_ms(1500);
    }
}

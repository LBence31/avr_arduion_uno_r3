#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD 9600

// UCSR0A for flags and half double speed mode
// USCR0B for enabling TX/RX, interrupts, data size bits
// USCR0C for frame format (parity, stop bits, data bits, mode)
// UBRR0H Baud Rate High byte
// UBDR0L Baur Rate Low byte
// UDR0 where data is written and read
// UDR0 where data is written and read

void uart_init(const uint16_t *myubr)
{
    UCSR0A = 0x00;                          // empty A register
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);   // enable RX and TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, no parity, 1 stop

    // set low and high bits for rate
    UBRR0H = (uint8_t)(*myubr >> 8);
    UBRR0L = (uint8_t)(*myubr);
}

void uart_send(char *s)
{
    while (*s)
    {
        while (!(UCSR0A & (1 << UDRE0)))
            ; // Wait until buffer empty
        UDR0 = *s++;
    }
}

uint16_t analog_read()
{
    ADCSRA |= (1 << ADSC); // set start conversation flag to 1 -> start listening
    while (ADCSRA & (1 << ADSC))
        ;       // wait until complete
    return ADC; // return 10-bit result
}

int main(void)
{
    int16_t baselineTemp = 27;

    // 16 as we'll be in normal speed mode
    // 8 if in UCSR0A U2X0 is enabled for double speed mode
    const uint16_t myubr = (F_CPU / (16UL * BAUD)) - 1;
    uart_init(&myubr);

    DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4);     // output
    PORTD &= ~(1 << PD2) & ~(1 << PD3) & ~(1 << PD4); // set outputs as LOW

    // configure analog input
    DDRC &= ~(1 << PC0);   // input
    DIDR0 |= (1 << ADC0D); // DIDR0 - Digital Input Disable Register, 0 is for ADC channels, 1 is for analog comparator
    ADMUX = (1 << REFS0);  // AVcc as ref, general Voltage referecne, 0 on all others -> right aligned(10bit), ADC0
    ADCSRA = (1 << ADEN)   // enable ADC
             | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128

    while (1)
    {
        char buf[26];
        uint16_t sensor_val = analog_read();
        uint32_t voltage_mv = ((uint32_t)sensor_val * 5000) / 1023;
        uint16_t volts = voltage_mv / 1000;
        uint16_t hundredths = (voltage_mv % 1000) / 10;
        int16_t temperature = (voltage_mv - 500) / 10;
        sprintf(buf, "Sensor Value: %u\n\r", sensor_val);
        uart_send(buf);
        sprintf(buf, "Voltage Value: %u.%02u\n\r", volts, hundredths);
        uart_send(buf);
        sprintf(buf, "Temperature: %d\n\r\n\r", temperature);
        uart_send(buf);

        if (temperature < baselineTemp + 2)
        {
            PORTD &= ~(1 << PD2) & ~(1 << PD3) & ~(1 << PD4);
        }
        else if (temperature >= baselineTemp + 2 && temperature < baselineTemp + 4)
        {

            PORTD &= ~(1 << PD3) & ~(1 << PD4);
            PORTD |= (1 << PD2);
        }
        else if (temperature >= baselineTemp + 4 && temperature < baselineTemp + 6)
        {

            PORTD &= ~(1 << PD4);
            PORTD |= (1 << PD2) | (1 << PD3);
        }
        else if (temperature >= baselineTemp + 6)
        {

            PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4);
        }

        _delay_ms(1500);
    }
}

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD 9600

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

void analog_input_init()
{
    DIDR0 |= (1 << ADC0D) | (1 << ADC1D) |
             (1 << ADC2D); // DIDR0 - Digital Input Disable Register, 0 is for ADC channels, 1 is for analog comparator
    ADMUX = (1 << REFS0);  // AVcc as ref, general Voltage referecne, 0 on all others -> right aligned(10bit), ADC0
    ADCSRA = (1 << ADEN)   // enable ADC
             | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128
}

uint16_t analog_read_from(const uint8_t channel)
{
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // keep refs and set new channel
    ADCSRA |= (1 << ADSC);                     // set start conversation flag to 1 -> start listening
    while (ADCSRA & (1 << ADSC))
        ;       // wait until complete
    return ADC; // return 10-bit result
}

void pwm_init()
{
    // port 9, 10 - timer 1
    // WGM10 and WGM12 set to 1 -> fast pwn, 8-bit
    // when fast pwn: COM1A1 and COM1B1 -> enable PWM for 9 and 10 pins, non-inverting mode
    // CS11 -> prescaler 8
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
    TCCR1B = (1 << WGM12) | (1 << CS11);

    // port 11 - timer 2
    // WGM20 and WGM21 set to 1 -> fast pwn (note: it can only be 8-bit)
    // when fast pwn: COM2A1 -> enable PWM for pin 11, non-inverting mode
    // CS21 -> prescaler 8
    TCCR2A = (1 << COM2A1) | (1 << WGM20) | (1 << WGM21);
    TCCR2B = (1 << CS11);
}

int main(void)
{
    _delay_ms(2000);                                 // wait for USB connection to stabilize
    DDRC &= ~(1 << PC0) & ~(1 << PC1) & ~(1 << PC2); // analog inputs
    DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3);    // pwm outputs

    const uint16_t myubr = (F_CPU / (16UL * BAUD)) - 1;
    uart_init(&myubr);
    analog_input_init();
    pwm_init();

    while (1)
    {
        uint16_t redSensorValue = analog_read_from(ADC0D);
        _delay_ms(5);
        uint16_t greenSensorValue = analog_read_from(ADC1D);
        _delay_ms(5);
        uint16_t blueSensorValue = analog_read_from(ADC2D);

        uint8_t redRGB = redSensorValue / 4;
        uint8_t greenRGB = greenSensorValue / 4;
        uint8_t blueRGB = blueSensorValue / 4;

        // OCR1A - pin9 | OCR1B - pin10 | OCR2A - pin11
        OCR1A = redRGB;
        OCR1B = greenRGB;
        OCR2A = blueRGB;

        char buf[62];
        sprintf(buf, "Raw Sensor Values \t red: %u \t green: %u \t blue: %u \n\r", redSensorValue, greenSensorValue,
                blueSensorValue);
        uart_send(buf);
        sprintf(buf, "Mapped Sensor Values \t red: %u \t green: %u \t blue: %u \n\r\n\r", redRGB, greenRGB, blueRGB);
        uart_send(buf);
        // _delay_ms(1500);
    }
}

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

int main(void)
{
    // to set pin as output you OR it with 1, meaning my pins will be set to 1
    // but leave the others not specified as it was
    // DDRD |= (1 << PD3);
    // DDRD |= (1 << PD4);
    // DDRD |= (1 << PD5);
    DDRD |= (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD3);

    // to set a pin as input you have to negate and AND to set yours to 0 and leave the others as they were
    // eg. at this point DDRD probably looks like this: DDRD = 0b00111000, (1 << PD2) = 0b00000100
    // ~(1 << PD2) = 0b11111011
    // 0b00111000 & 0b11111011 = 0b00111000
    DDRD &= ~(1 << PD2);

    uint8_t starting = 1;

    while (1)
    {
        // Low is 1, High is 0
        uint8_t switchState = (PIND & (1 << PD2) ? 1 : 0);

        if (switchState)
        {
            if (starting)
            {
                // flash all 3 LEDs for 3 times
                for (uint8_t i = 3; i > 0; i--)
                {
                    PORTD |= (1 << PD3) | (1 << PD4) | (1 << PD5);
                    _delay_ms(700);
                    PORTD &= ~(1 << PD3) & ~(1 << PD4) & ~(1 << PD5);
                    _delay_ms(700);
                }

                // Light first RED light -> light both RED ligths -> light GREEN ligth and turn off RED lights
                PORTD |= (1 << PD5);
                PORTD &= ~(1 << PD4);

                _delay_ms(1000);

                PORTD |= (1 << PD4);

                _delay_ms(1000);

                PORTD &= ~(1 << PD4) & ~(1 << PD5);
                PORTD |= (1 << PD3);
                starting = 0;
            }
        }
        else
        {
            starting = 1;
            PORTD &= ~(1 << PD3) & ~(1 << PD4);
            PORTD |= (1 << PD5);

            _delay_ms(250);
            PORTD |= (1 << PD4);
            PORTD &= ~(1 << PD5);
            _delay_ms(250);
        }
    }
}

/*
 * ucnc.cpp
 *
 * Created: 16.01.2021 20:55:30
 * Author : AlrHa
 */ 
#define F_CPU	20000000UL
#define BAUD	19200
#define CUBRR		((F_CPU / 16 / BAUD) - 1)

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define IN_SRCLR	(PINC0)
#define IN_SRCLK	(PINC1)
#define IN_RCLK		(PINC2)
#define IN_OE		(PINC3)
#define IN_SER		(PINC4)

#define AUX_BTN		(PINC5)
#define RESET		(PINC6)

#define	OUT_SER		(PIND2)
#define	OUT_OE		(PIND3)
#define	OUT_RCLK	(PIND4)
#define	OUT_SRCLK	(PIND5)
#define	OUT_SRCLR	(PIND6)


#define SSEL_A		(PINB0)
#define SSEL_B		(PINB1)
#define SSEL_C		(PINB2)
#define ASIN		(PINE2)

//inverted select signal
#define ASA	((uint8_t)0x04) //011 -> 100 -> 0x04
#define ASB	((uint8_t)0x07) //000 -> 111 -> 0x07
#define ASC	((uint8_t)0x02) //101 -> 010 -> 0x02
#define ASD	((uint8_t)0x06) //001 -> 110 -> 0x06
#define ASE	((uint8_t)0x00) //111 -> 000 -> 0x00
#define ASF	((uint8_t)0x05) //010 -> 101 -> 0x05
#define ASG	((uint8_t)0x01) //110 -> 001 -> 0x01
#define ASH	((uint8_t)0x03) //100 -> 011 -> 0x03

const uint8_t ASI_MAP[] PROGMEM= {ASA, ASB, ASC, ASD, ASE, ASF, ASG, ASH};
	
void init_uart(){
	UBRR0H = (uint8_t)(CUBRR >> 8);
	UBRR0L = (uint8_t)(CUBRR);
	UCSR0B =  (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void setup(){
	
	//make PINC0-4 outputs
	DDRC |= ((1 << IN_SRCLR) | (1 << IN_SRCLK) | (1 << IN_RCLK) | (1 << IN_OE) | (1 << IN_SER));
	PORTC = 0x00;	
	//disable input clear
	PORTC |= (1 << IN_SRCLR);
	//enable input indicators
	PORTC &= ~(1 << IN_OE);
	
	//make PIND2-6 outputs
	DDRD |= ((1 << OUT_SER) | (1 << OUT_OE) | (1 << OUT_RCLK) | (1 << OUT_SRCLK) | (1 << OUT_SRCLR));
	//disable output clear
	PORTD |= (1 << OUT_SRCLR);
	//enable outputs
	PORTD &= ~(1 << OUT_OE);
	
	//make PB0-PB2 outputs
	DDRB |= (1 << SSEL_A) |(1 << SSEL_B) |(1 << SSEL_C);
	
	
	
	init_uart();
}

void set_input_indicators(uint8_t indicators){
	
	for (uint8_t i = 0; i < 8; i++)
	{
		//shift value into register
		if(((indicators >> i) & 0x01)){
			PORTC |= (1 << IN_SER);
		}else{
			PORTC &= ~(1 << IN_SER);
		}
		//clock register
		PORTC |= (1 << IN_SRCLK);
		PORTC &= ~(1 << IN_SRCLK);
	}	
	
	//clock output register
	PORTC |= (1 << IN_RCLK);
	PORTC &= ~(1 << IN_RCLK);	
}

void set_outputs(uint8_t outputs){
	for (uint8_t i = 0; i < 8; i++)
	{
		//shift value into register
		if(((outputs >> i) & 0x01)){
			PORTD |= (1 << OUT_SER);
			}else{
			PORTD &= ~(1 << OUT_SER);
		}
		//clock register
		PORTD |= (1 << OUT_SRCLK);
		PORTD &= ~(1 << OUT_SRCLK);
	}
		
	//clock output register
	PORTD |= (1 << OUT_RCLK);
	PORTD &= ~(1 << OUT_RCLK);
}

void uart_out(const char* data) {
	while(*data){
		UDR0 = *(data++);
		_delay_ms(10);
	}
}

int main(void)
{
    setup();
	
	

	
	
	

	
	uint8_t value = 0;

	
	
	//select internal 1.1v reference and ADC6
	ADMUX |= (1 << REFS0) | (1 << MUX2) | (1 << MUX1);
	//enable ADC and set prescaler to 128
	ADCSRA |= (1 << ADEN) | (1 << ADPS2)| (1 << ADPS1)| (1 << ADPS0);
	
	while (1) 
    {
		for (uint8_t s = 0; s < 8; s++)
		{
			PORTB = (uint8_t)((PORTB & (uint8_t)~0x07) | pgm_read_byte(&ASI_MAP[s]));
			_delay_ms(5);
					

			ADCSRA |= (1 << ADSC);
			_delay_ms(5);

			uint16_t av = ADCL;
			av |= (uint16_t)(ADCH << 8);
			av = (uint16_t)((100.0 / 1024.0) * av);

			char buffer[10];
			
			utoa(s, buffer, 10);
			uart_out(buffer);
			
			uart_out(": ");
			
			utoa(av, buffer, 10);
			uart_out(buffer);
			
			uart_out(" ");
			
		//set_input_indicators(value);
		//set_outputs(~value);
		//value++;
		//_delay_ms(100);
		
		}
		
		
		uart_out("\r\n");
	}
}


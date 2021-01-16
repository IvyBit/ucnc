/*
 * ucnc.cpp
 *
 * Created: 16.01.2021 20:55:30
 * Author : AlrHa
 */ 
#define F_CPU	20000000UL
#define BAUD	19200
#define CUBRR		((F_CPU / 16 / BAUD) - 1)

#include <avr/io.h>
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
		_delay_ms(100);
	}
}

int main(void)
{
    setup();
	
	UBRR0H = (uint8_t)(CUBRR >> 8);
	UBRR0L = (uint8_t)(CUBRR);
	UCSR0B =  (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	

	
	uint8_t value = 0;
	
	while (1) 
    {
			uart_out("Hello World!\r\n");
			
		set_input_indicators(value);
		set_outputs(~value);
		value++;
		_delay_ms(250);
	}
}


/*
 * ucnc.cpp
 *
 * Created: 16.01.2021 20:55:30
 * Author : AlrHa
 */ 
#define AVR

#define F_CPU	18432000UL
#define BAUD	115200UL
#define CUBRR		((F_CPU / 16 / BAUD) - 1)

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>


#include "errors.h"
#include "avrtypes.h"
#include "opcodes.h"
#include "expression.h"
#include "compiler.h"
#include "terminal.h"


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
#define ASA	((uint8_t)(~0x03 & 0x07)) //011 -> 100 -> 0x04
#define ASB	((uint8_t)(~0x00 & 0x07)) //000 -> 111 -> 0x07
#define ASC	((uint8_t)(~0x05 & 0x07)) //101 -> 010 -> 0x02
#define ASD	((uint8_t)(~0x01 & 0x07)) //001 -> 110 -> 0x06
#define ASE	((uint8_t)(~0x07 & 0x07)) //111 -> 000 -> 0x00
#define ASF	((uint8_t)(~0x02 & 0x07)) //010 -> 101 -> 0x05
#define ASG	((uint8_t)(~0x06 & 0x07)) //110 -> 001 -> 0x01
#define ASH	((uint8_t)(~0x04 & 0x07)) //100 -> 011 -> 0x03

const uint8_t ASI_MAP[8] PROGMEM= { ASA, ASB, ASC, ASD, ASE, ASF, ASG, ASH };
	
void setup_uart(){
	UBRR0H = (uint8_t)(CUBRR >> 8);
	UBRR0L = (uint8_t)(CUBRR);
	UCSR0B =  (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void setup_adc(){
	//select 5v reference and ADC6
	ADMUX |= (1 << REFS0) | (1 << MUX2) | (1 << MUX1);
	//enable ADC, enable interrupt, set prescaler to 128
	ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADPS2)| (1 << ADPS1)| (1 << ADPS0);
	
	PORTB = (uint8_t)((PORTB & (uint8_t)~0x07) | pgm_read_byte(&ASI_MAP[0]));
	//PORTB = (uint8_t)((PORTB & (uint8_t)~0x07) | ASA);
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

	setup_uart();
	
	setup_adc();	
	
	sei();
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



void err::on_error(const Error& ec) {
	while (true)
	{
		//BLINK OUTPUTS
		set_input_indicators(0xFF);
		_delay_ms(500);
		set_input_indicators(0x00);	
			
		//UART OUT ERROR		
	}
}

struct Input{
	uint8_t		threshold	= 50;	
	uint8_t		hysteresis  = 5;
	bool		inverted	= false;
	op::OpCode	state		= op::OpCode::FALSE;
	
	op::OpCode update(uint8_t value){		
		if(value > threshold + hysteresis){
			if(inverted){
				state = op::OpCode::FALSE;				
			}
			else {
				state = op::OpCode::TRUE;	
			}
		}else if(value < threshold - hysteresis){
			if(inverted){
				state = op::OpCode::TRUE;
			}
			else {
				state = op::OpCode::FALSE;
			}
		}	
		
		return state;	
	}
};


volatile uint8_t	adc_values[16]	= {0};
volatile uint8_t*	adc_value_write	= nullptr;
volatile uint8_t*	adc_value_read	= nullptr;
volatile bool adc_busy = false;

ISR(ADC_vect){
	
	static avr_size_t	adc_max	  = 1;
	static double		adc_mul   = 1;
	static uint8_t		adc_index = 0;
	
	uint8_t current_index = adc_index;
	adc_index++;
	
	if(adc_index > 7) adc_index = 0;
	
	PORTB = (uint8_t)((PORTB & (uint8_t)~0x07) | pgm_read_byte(&ASI_MAP[adc_index]));
	
	avr_size_t av = 0;
	
	av |= ADCL;
	av |= (uint16_t)(ADCH << 8);
	
	if(adc_max < av){
		adc_max = av;
		adc_mul = (100.0 / av);
	}
		
	adc_value_write[current_index] = (uint8_t)(adc_mul * av);
	
	if(adc_index < 7){
		ADCSRA |= (1 << ADSC);
	}else{
		adc_busy = false;
	}
}

void adc_busy_wait(){
	volatile avr_size_t n = 0;
	while(adc_busy){n++;}
}

void adc_start_conversion(){	
	
	adc_busy = true;
	
	if(adc_value_read != adc_values){
		adc_value_read = &adc_values[0];
		adc_value_write = &adc_values[8];
	} else{
		adc_value_read = &adc_values[8];
		adc_value_write = &adc_values[0];
	}
	
	ADCSRA |= (1 << ADSC);
}

Input inputs[] = 
{
	{50, 5, false, op::OpCode::FALSE},//A
	{50, 5, false, op::OpCode::FALSE},//B
	{50, 5, false, op::OpCode::FALSE},//C
	{50, 5, false, op::OpCode::FALSE},//D
	{50, 5, false, op::OpCode::FALSE},//E
	{50, 5, false, op::OpCode::FALSE},//F
	{40, 5, true,  op::OpCode::FALSE},//G
	{40, 5, true,  op::OpCode::FALSE} //H
};





ctr::Array<op::OpCode, 1024> expression_buffer;
ctr::Array<op::OpCode, 256> stack_buffer;
ex::Expression expressions[8];
comp::ExpressionCompiler compiler;

//char expression_string[200];

ex::ExpressionData data;

int main(void)
{
    setup();	
	
	int g = sizeof(ctr::Array<op::OpCode, 10>);
	
	//char x = expression_string[0];
	
	comp::CompilerResult cr;

	
	bool result = true;
	{
		avr_size_t offset = 0;
		expressions[0].set_buffer(&expression_buffer[offset]);
		result &= compiler.compile_expression("(~$A & $B) | ($A & ~$B)", expressions[0], stack_buffer, cr);
		offset += expressions[0].length();
		
		expressions[1].set_buffer(&expression_buffer[offset]);
		result &= compiler.compile_expression("$B", expressions[1], stack_buffer, cr);
		offset += expressions[1].length();
		
		expressions[2].set_buffer(&expression_buffer[offset]);
		result &= compiler.compile_expression("$C", expressions[2], stack_buffer, cr);
		offset += expressions[2].length();
		
		expressions[3].set_buffer(&expression_buffer[offset]);
		result &= compiler.compile_expression("$D", expressions[3], stack_buffer, cr);
		offset += expressions[3].length();
		
		expressions[4].set_buffer(&expression_buffer[offset]);
		result &= compiler.compile_expression("$E", expressions[4], stack_buffer, cr);
		offset += expressions[4].length();
		
		expressions[5].set_buffer(&expression_buffer[offset]);
		result &= compiler.compile_expression("$F", expressions[5], stack_buffer, cr);
		offset += expressions[5].length();
		
		expressions[6].set_buffer(&expression_buffer[offset]);
		result &= compiler.compile_expression("$G", expressions[6], stack_buffer, cr);
		offset += expressions[6].length();
		
		expressions[7].set_buffer(&expression_buffer[offset]);
		result &= compiler.compile_expression("$H", expressions[7], stack_buffer, cr);
		offset += expressions[7].length();
		
	}

	if (!result) {	
		while (1)
		{
			set_input_indicators(0xFF);
			_delay_ms(200);
			set_input_indicators(0x00);
		}
	}

	
	for (avr_size_t n = 0; n < 100; n++)
	{
		adc_start_conversion();
		adc_busy_wait();
	}
	
	
	
	char buffer[6] = {0};
	while(1){
		
		adc_busy_wait();
		adc_start_conversion();
		
		//if(adc_value_read == adc_values){
			//term::write_string("A:\t");
		//} else{
			//term::write_string("B:\t");
		//}
		//
		//for (avr_size_t i = 0; i < 120; i++)
		//{
			//if(i < adc_value_read[7])
				//term::write_char('X');			
			//else
				//term::write_char(' ');			
		//}
		//
		//term::write_string("\r\n");
		//continue;
		
		for (uint8_t i = 0; i < 8; i++)
		{
		
			
			//term::write_char('[');
			//term::write_char((char)(65 + i));
			//term::write_string("] : ");
			
			if(inputs[i].update(adc_value_read[i]) == op::OpCode::TRUE){
				//term::write_string("TRUE");
			}else{
				//term::write_string("FALSE");
			}
			
			data.set_at(i, inputs[i].state);
					
			//term::write_string("-> (");
			
			utoa((avr_size_t)(adc_value_read[i]), buffer, 10);
			//term::write_string(buffer);
			//term::write_char(')');
	
			
	
			//term::write_string("\t");		
		}
		
		
		set_input_indicators(data.data());
		uint8_t output = 0;
		for (uint8_t i = 0; i < 8; i++)
		{
			op::OpCode exp_result = expressions[i].eval(data, stack_buffer);
			if(exp_result == op::OpCode::TRUE){
				output |= (1 << (7 - i));
			}else{
				
			}
		}		
		set_outputs(output);
		
		//term::write_string("\r\n");		
	}
	

	//str::StringBuffer<100> s;
	//while (true)
	//{
		//
		//term::write_string("\r\n");
		//term::write_string("::[?] : ");
		//term::read_line(s);
		//
		//term::write_line(s);
		//
		//comp::CompilerResult cr;
		//ex::Expression<100> expr;
		//ex::ExpressionData dat;
		//
		//bool result = false;
		//{
			//result = comp::compile_expression(s, expr, cr);
		//}
//
		//if (result) {
			//term::write_line("START!");
			//for (avr_size_t n = 0; n < 10000; n++)
			//{			
				//dat.a(op::OpCode::TRUE);
				//op::OpCode result1 = expr.eval(dat);	
			//}
			//term::write_line("DONE!");
		//}
	//}

	
		
	

		
	
}


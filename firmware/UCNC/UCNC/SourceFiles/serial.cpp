/*
 * serial.cpp
 *
 * Created: 06.06.2021 10:52:47
 *  Author: alrha
 */

#include "definitions.h"
#include "serial.h"
#include "io.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

namespace serial{
	#define CUBRR	((F_CPU / 16 / BAUD) - 1)

	void setup(){
		UBRR0H = (uint8_t)(CUBRR >> 8);
		UBRR0L = (uint8_t)(CUBRR);
		UCSR0B =  (1 << TXEN0) | (1 << RXEN0);
		UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	}

	void write(char c){
		while ( !( UCSR0A & (1<<UDRE0)) );
		UDR0 = c;
	}

	void write(const char* cs){
		for(const char* tmp = cs; *tmp != STR_END; tmp++){
			write(*tmp);
		}
	}

	void write_line(const char* cs){
		write(cs);
		write_line();
	}

	void write_line(){
		write_P(PSTR("\r\n"));
	}

	void write_line(uint8_t lines){
		for(; lines > 0; lines--){
			write_line();
		}
	}

	void write_P(const char* src){
		for (char c; (c = pgm_read_byte(src)) != STR_END; src++)
		{
			write(c);
		}
	}


	void write_uint8_t(uint8_t value){
		char buffer[4];
		utoa(value, buffer, 10);
		write(buffer);
	}

	void write_uint16_t(uint16_t value){
		char buffer[7];
		utoa(value, buffer, 10);
		write(buffer);
	}

	char read(){
		while ( !(UCSR0A & (1 << RXC0)) );
		return (char)UDR0;
	}

	uint16_t read_line(char* cs){

		uint16_t cc = 0;

		for(;;){
			char c = read();
			switch(c){
				case STR_NL: case STR_CR:
					return cc;
				case STR_CANCEL :
					return SER_RESET;
				default:{
					cs[cc++] = c;
				}
			}
		}
		return cc;
	}


	void enable_serial_reset(){
		UCSR0B |= (1 << RXCIE0);
	}

	void disable_serial_reset(){
		UCSR0B &= ~(1 << RXCIE0);
	}

	ISR(USART0_RX_vect)
	{
		static uint8_t mode_step = 0;
		char data = UDR0;
		if(data == STR_CANCEL ){
			cli();
			io::reset_mcu();
		} else {

			if(mode_step == 0 && data == '$'){
				mode_step = 1;
			} else if(mode_step == 1 && data == 'C'){
				mode_step = 2;
			} else if(mode_step == 2 && (data == STR_CR || data == STR_NL)){
				io::set_boot_cfg_flag();
				io::reset_mcu();
			} else {
				mode_step = 0;
			}
		}
	}


};
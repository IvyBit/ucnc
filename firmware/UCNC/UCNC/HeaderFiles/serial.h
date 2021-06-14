/*
 * serial.h
 *
 * Created: 06.06.2021 10:49:16
 *  Author: alrha
 */
#include "definitions.h"
#include "stringbuffer.h"
#include <stdint-gcc.h>
#ifndef SERIAL_H_
#define SERIAL_H_

namespace serial{


		void setup();

		void write(char c);
		void write(const char* cs);
		void write_line();
		void write_line(uint8_t lines);
		void write_P(const char* cs);

		void write_uint8_t(uint8_t value);
		void write_uint16_t(uint16_t value);

		char read();
		uint16_t read_line(char* cs);

		template<uint16_t size>
		uint16_t read_line(str::stringbuffer<size> &buffer){

			uint16_t cc = 0;

			for(;;){
				char c = read();
				switch(c){
					case STR_CR: case STR_NL:
						return cc;
					case STR_CANCEL :
						return SER_RESET;
					default:{
						if(buffer.append(c)){
							cc++;
						} else {
							return SER_OVERF;
						}
					}
				}
			}

			return cc;
		}

		void enable_serial_reset();
		void disable_serial_reset();

};
#endif /* SERIAL_H_ */
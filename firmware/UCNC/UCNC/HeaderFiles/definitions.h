/*
 * defines.h
 *
 * Created: 06.06.2021 00:46:16
 *  Author: alrha
 */
#ifndef _DEFINES_H_
#define _DEFINES_H_

	#define F_CPU 16000000UL
	#define BAUD 38400UL
	#define ADC_LIMIT 222


	#define STR_END			'\0'
	#define STR_CR			'\r'
	#define STR_NL			'\n'
	#define STR_ESC			'\x1B'
	#define STR_CANCEL		'\x18'

	#define SER_RESET 0xFFFF
	#define SER_OVERF 0xFFFF -1


	#define STR_NO_MATCH -1

	#define EEPROM_SIZE 1024
	#define INPUT_BUFFER_SIZE 225



	#define DEFAULT_SYS_SERIAL false
	#define DEFAULT_SYS_BUZZER true

	#define DEFAULT_IN_THRESHOLD 70
	#define DEFAULT_IN_HYSTERESIS 10
	#define DEFAULT_IN_INVERTED false

	#define DEFAULT_OUT_ACTIVE true


#endif
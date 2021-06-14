/*
 * io.h
 *
 * Created: 05.06.2021 23:44:13
 *  Author: alrha
 */


#include <stdint-gcc.h>

#ifndef IO_H_
#define IO_H_


namespace io{


	void setup();

	void set_indicators(uint16_t indicators);
	void clear_indicators();
	void set_outputs(uint8_t outputs);
	void clear_outputs();



	bool adc_is_busy();
	void adc_busy_wait();
	void adc_start_conversion();
	void adc_update(uint8_t (&buffer)[8]);

	/*BUTTON*/
	bool get_aux_pushed();
	void enable_btn_reset_async();
	void disable_btn_reset_async();
	/*BUTTON*/


	/*BUZZER*/
	void buzzer_on_for(uint16_t milliseconds);
	void buzzer_on_for_async(uint16_t milliseconds);
	void buzzer_on();
	void buzzer_off();
	/*BUZZER*/




	/*LED*/
	void led_on_for(uint16_t milliseconds);
	void led_on_for_async(uint16_t milliseconds);
	void led_on();
	void led_off();
	/*LED*/



	void disable_watchdog();
	void reset_mcu()  __attribute__((noreturn)) ;


	void set_boot_cfg_flag();
	void clear_boot_cfg_flag();
	bool boot_cfg_flag_set();
};

#endif /* IO_H_ */
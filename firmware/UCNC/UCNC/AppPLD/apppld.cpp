/*
 * apppld.cpp
 *
 * Created: 05.06.2021 17:39:18
 *  Author: alrha
 */
 #if defined(NDEBUG) || defined(DEBUGPLD)
#include "apppld.h"
#include "io.h"
#include "config.h"
#include "protocol.h"
#include "parser.h"
#include "expression.h"
#include "opcodes.h"
#include "serial.h"

#include <util\delay.h>
namespace apps{



	apppld::apppld(){}

	void apppld::halt(){
		io::enable_btn_reset_async();
		serial::enable_serial_reset();

		while(1){
			io::led_on();
			io::buzzer_on_for(150);
			_delay_ms(150);
			io::led_off();
			_delay_ms(500);
		}
	}

	bool apppld::on_entry(){
		prot::write_welcome_pld();
		io::buzzer_on_for(500);
		io::set_indicators(0xFF00);

		//CHECK AND LOAD CONFIGURATION
		if(config::check_config()){

			//LOAD SYSTEM CONFIGURATION
			config::read_system_config(sys_config);


			//LOAD INPUT CONFIGURATION
			for (uint8_t i = 0; i < 8; i++){
				config::input_config ic;
				config::read_input_config(ic, i);
				apppld::io_translators[i].threshold = ic.threshold;
				apppld::io_translators[i].hysteresis = ic.hysteresis;
				apppld::io_translators[i].inverted = ic.inverted;
			}
		}

		//CHECK AND LOAD CONFIGURATION


		//COMPILE EXPRESSIONS AND BUILD TRUTH TABLE
		for (uint8_t t = 0; t < 8; t++){

			io::set_indicators((t + 1) << 8 | 0x00);

			char exp_buffer[EXPRESSION_SIZE];
			opcodes::op_code op_buffer[EXPRESSION_SIZE];
			expr::expression exp_target;

			exp_target.data = (&op_buffer[0]);

			config::read_expression(exp_buffer, t);
			parser::compiler_result result;
			parser::compile_expression(exp_buffer, exp_target, result);

			if(result.status == parser::compiler_status::OK){

				for (uint16_t r = 0; r < 256; r++) {
					io::set_indicators((t + 1) << 8 | r);

					expr::expression_data data = (uint8_t)r;

					switch (expr::eval(exp_target, data)){
						case opcodes::op_code::TRUE		: truth_table[r] |= (0x80 >> t);	break;
						case opcodes::op_code::FALSE	: truth_table[r] &= ~(0x80 >> t);	break;
						default: halt();
					}
				}

			}
			else {
				halt();
			}
		}
		//COMPILE EXPRESSIONS AND BUILD TRUTH TABLE

		io::enable_btn_reset_async();
		serial::enable_serial_reset();
		io::set_indicators(0xFFFF);
		return true;
	}


	bool apppld::on_run(){

		uint8_t current_values[8]	= {0};
		uint8_t last_values[8]		= {0xFF};
		uint8_t output_index		= 0;
		uint8_t last_output_index	= 0xFF;
		bool	values_changed		= false;

		io::adc_start_conversion();

		while(1){

			io::adc_busy_wait();
			io::adc_update(current_values);
			io::adc_start_conversion();

			values_changed = false;

			for (uint8_t i = 0; i < 8; i++){
				if(current_values[i] != last_values[i]){
					last_values[i] = current_values[i];
					values_changed = true;

					if(translation::translate(io_translators[i], current_values[i])){
						output_index |= (1 << (7 - i));
					} else {
						output_index &= ~(1 << (7 - i));
					}
				}
			}

			if(output_index != last_output_index){
				last_output_index = output_index;

				io::set_outputs(truth_table[output_index]);
				io::set_indicators((uint16_t)(output_index << 8) | truth_table[output_index]);
				if(sys_config.buzzer){
					io::buzzer_on_for_async(100);
					io::led_on_for_async(100);
				}
			}


			if(values_changed && sys_config.serial){
				for (uint8_t i = 0; i < 8; i++){
					serial::write('A' + i);
					serial::write(':');
					serial::write_uint8_t(current_values[i]);
					serial::write(':');
					if(io_translators[i].last_state){
						serial::write('1');
					} else {
						serial::write('0');
					}
					serial::write(' ');
				}
				serial::write_line();
			}
		}
		return true;
	}


	bool apppld::on_exit(){
		return true;
	}


	apppld::~apppld(){ }
};
#endif
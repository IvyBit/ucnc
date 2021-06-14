/*
 * config.cpp
 *
 * Created: 07.06.2021 18:07:21
 *  Author: alrha
 */
 #include "config.h"
 #include "stdint.h"
 #include <avr/eeprom.h>

 namespace config{

	uint8_t get_checksum(uint8_t* src, uint16_t size){
		uint8_t checksum = 0;
		for(uint8_t i = 0; i < size; i++){
			checksum = (checksum << 1) || (checksum >> 7);
			checksum += src[i];
		}
		return checksum;
	}


	bool read_system_config(system_config &config){

		uint16_t offset = 0;
		system_config buffer;

		eeprom_read_block((void*)&buffer, (void*)offset, sizeof(system_config));

		uint8_t checksum = get_checksum((uint8_t*)&buffer, sizeof(system_config) - 1);

		if(checksum == buffer.checksum){
			config = buffer;
			return true;
		} else {
			return false;
		}
	 }

	bool read_input_config(input_config &config, uint8_t index){

		uint16_t offset = sizeof(system_config) + (index * sizeof(input_config));

		input_config buffer;

		eeprom_read_block((void*)&buffer, (void*)offset, sizeof(input_config));

		uint8_t checksum = get_checksum((uint8_t*)&buffer, sizeof(input_config) - 1);

		if(checksum == buffer.checksum){
			config = buffer;
			return true;
		} else {
			return false;
		}
	}

	bool read_output_config(output_config &config, uint8_t index){

		uint16_t offset = sizeof(system_config) + (8 * sizeof(input_config)) + (index * sizeof(output_config));

		output_config buffer;

		eeprom_read_block((void*)&buffer, (void*)offset, sizeof(output_config));

		uint8_t checksum = get_checksum((uint8_t*)&buffer, sizeof(output_config) - 1);

		if(checksum == buffer.checksum){
			config = buffer;
			return true;
		} else {
			return false;
		}
	}

	bool read_expression(char* exp_target, uint8_t index){
		//[checksum][expression][0][......] [checksum...
		uint16_t offset = (EEPROM_SIZE - ((8 - index) * EXPRESSION_OFFSET));

		uint8_t checksum = eeprom_read_byte((uint8_t*)offset);

		char* exp = exp_target;

		for (uint16_t i = offset + 1; i < offset + EXPRESSION_OFFSET; i++) {
			char chr = eeprom_read_byte((uint8_t*)i);

			*(exp++) = chr;

			if(chr == STR_END){
				break;
			}
		}

		return exp_target[0] != STR_END && checksum == get_checksum((uint8_t*)exp_target, str::length(exp_target));

	}



	void write_system_config(system_config &config){

		uint16_t offset = 0;

		config.checksum = get_checksum((uint8_t*)&config, sizeof(system_config) - 1);

		eeprom_update_block((void*)&config, (void*)offset, sizeof(system_config));
	}

	void write_input_config(input_config &config, uint8_t index){

		uint16_t offset = sizeof(system_config) + (index * sizeof(input_config));

		config.checksum  = get_checksum((uint8_t*)&config, sizeof(input_config) - 1);

		eeprom_update_block((void*)&config, (void*)offset, sizeof(input_config));
	}

	void write_output_config(output_config &config, uint8_t index){

		uint16_t offset = sizeof(system_config) + (8 * sizeof(input_config)) + (index * sizeof(output_config));

		config.checksum = get_checksum((uint8_t*)&config, sizeof(output_config) - 1);

		eeprom_update_block((void*)&config, (void*)offset, sizeof(output_config));
	}

	void write_expression(const char* exp, uint8_t index){
		//[checksum][expression][0][......] [checksum...
		uint16_t offset = (EEPROM_SIZE - ((8 - index) * EXPRESSION_OFFSET));

		const uint8_t* src = (uint8_t*)exp;
		uint16_t i = offset + 1;

		for (; i < offset + EXPRESSION_SIZE + 1; i++){
			if(*src != STR_END){
				eeprom_update_byte((uint8_t*)(i), *src++);
			} else {
				break;
			}
		}

		eeprom_update_byte((uint8_t*)(i), STR_END);

		uint8_t checksum = get_checksum((uint8_t*)exp, str::length(exp));

		eeprom_update_byte((uint8_t*)(offset), checksum);
	}



	void reset_system_config(){
		system_config cfg;
		write_system_config(cfg);
	}

	void reset_input_config(){
		for (uint8_t i = 0; i < 8; i++){
			input_config cfg;
			write_input_config(cfg, i);
		}
	}

	void reset_output_config(){
		for (uint8_t i = 0; i < 8; i++){
			output_config cfg;
			write_output_config(cfg, i);
		}
	}

	void reset_expressions(){
		for (uint8_t i = 0; i < 8; i++) {
			str::stringbuffer<EXPRESSION_SIZE> buffer;
			buffer.append('$');
			buffer.append((char)('A' + i));
			write_expression(buffer, i);
		}
	}

	void reset_config(){

		reset_system_config();

		reset_input_config();

		reset_output_config();

		reset_expressions();
	}



	bool check_config(){

		//system config
		{
			system_config cfg;
			if(!read_system_config(cfg)){
				return false;
			}
		}

		//input config
		for (uint8_t i = 0; i < 8; i++){
			input_config cfg;
			if(!read_input_config(cfg, i)){
				return false;
			}
		}

		//output config
		for (uint8_t i = 0; i < 8; i++){
			output_config cfg;
			if(!read_output_config(cfg, i)){
				return false;
			}
			char buffer[EXPRESSION_SIZE];
			if(!read_expression(buffer, i)){
				return false;
			}
		}

		return true;
	}
 }
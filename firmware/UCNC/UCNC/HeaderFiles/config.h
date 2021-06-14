/*
 * config.h
 *
 * Created: 07.06.2021 17:31:34
 *  Author: alrha
 */

 #include "definitions.h"
 #include "stdint.h"
 #include "stringbuffer.h"
#ifndef CONFIG_H_
#define CONFIG_H_
namespace config{

	struct system_config{
		bool	serial		= DEFAULT_SYS_SERIAL;
		bool	buzzer		= DEFAULT_SYS_BUZZER;
		uint8_t	checksum	= 0;
	};//3

	struct input_config {
		uint8_t		threshold	= DEFAULT_IN_THRESHOLD;
		uint8_t		hysteresis  = DEFAULT_IN_HYSTERESIS;
		bool		inverted	= DEFAULT_IN_INVERTED;
		uint8_t		checksum	= 0;
	}; //32

	struct output_config {
		bool	 active			= DEFAULT_OUT_ACTIVE;
		uint8_t	 checksum		= 0;
	};//32

	#define CONFIG_SIZE (sizeof(config::system_config) + (8 * sizeof(config::input_config) + (8 * sizeof(config::output_config))))
	#define EXPRESSION_SPACE (EEPROM_SIZE - CONFIG_SIZE)
	#define EXPRESSION_OFFSET ((uint16_t)(EXPRESSION_SPACE / 8))
	#define EXPRESSION_SIZE ((uint16_t)(EXPRESSION_OFFSET - 1))

	bool read_system_config(system_config &config);

	bool read_input_config(input_config &config, uint8_t index);

	bool read_output_config(output_config &config, uint8_t index);


	void write_system_config(system_config &config);

	void write_input_config(input_config &config, uint8_t index);

	void write_output_config(output_config &config, uint8_t index);





	bool read_expression(char* exp_target, uint8_t index);

	void write_expression(const char* exp, uint8_t index);

	void reset_expressions();




	void reset_system_config();

	void reset_input_config();

	void reset_output_config();

	void reset_config();


	bool check_config();

};
#endif /* CONFIG_H_ */
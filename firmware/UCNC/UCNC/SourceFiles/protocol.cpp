

#include "config.h"
#include "serial.h"
#include "io.h"
#include "protocol.h"
#include "stringsfunctions.h"
#include "parser.h"

#include <avr/pgmspace.h>
#include <stdlib.h>

namespace prot{

	const char CMDP_RESET[] PROGMEM			= "$RST=";
	const char CMDP_RESTORE[] PROGMEM		= "\t- Restore ";
	const char CMDP_AND_REBOOT[] PROGMEM	= "defaults and reboot";
	const char CMDP_INPUT[] PROGMEM			= "\t- Input ";
	const char CMDP_OUTPUT[] PROGMEM		= "\t- Output ";
	const char CMDP_THRESHOLD[] PROGMEM		= " threshold\t(";
	const char CMDP_HYSTERESIS[] PROGMEM	= " hysteresis\t(";
	const char CMDP_INVERT[] PROGMEM		= " inverted\t(";
	const char CMDP_ACTIVE[] PROGMEM		= " active\t(";
	const char CMDP_EXPRESSION[] PROGMEM	= " expression\t(";
	const char CMDP_1_0[] PROGMEM			= "1/0)";
	const char CMDP_0_100[] PROGMEM			= "0-100)";
	const char CMDP_A_B[] PROGMEM			= "$A & $B | ...) #comment";
	const char CMDP_SPACE[] PROGMEM			= " = ";


	const char CMDE_SYSTEM_MSG[] PROGMEM	= "'$' ";
	const char CMDE_NOT_SUPPORTED_MSG[] PROGMEM	= " not supported.";

	const char CMDE_OK_MSG[] PROGMEM		= "OK";
	const char CMDE_ERROR_MSG[] PROGMEM		= "error: ";
	const char CMDE_INVALID_MSG[] PROGMEM	= "Command";
	const char CMDE_TARGET_MSG[] PROGMEM	= "Target";
	const char CMDE_OPTION_MSG[] PROGMEM	= "Option";
	const char CMDE_VALUE_MSG[] PROGMEM		= "Value";
	const char CMDE_LENGTH_MSG[] PROGMEM	= "Characters per line exceeded.";
	const char CMDE_CHECKSUM_MSG[] PROGMEM	= "EEPROM read failed. Defaults restored.";
	const char CMDE_NEAR_MSG[] PROGMEM		= " near ";

	#define CMD_SYSTEM_SERIAL 100
	#define CMD_SYSTEM_BUZZER 101


	struct cmd_type{
		uint8_t target = 0;
		uint8_t option = 0;
		bool edit = false;
		bool valid = false;
	};

	void write_welcome_cfg(){
		serial::write_P(PSTR("UCNC 1.4 ['$' help; '$P' run] : CONFIG"));
		serial::write_line();
	}

	void write_welcome_pld(){
		serial::write_P(PSTR("UCNC 1.4 ['$C' config] : RUN"));
		serial::write_line();
	}

	void write_status(uint16_t error){

		if(error == CMDE_OK){
			serial::write_P(CMDE_OK_MSG);
		}
		else {
			serial::write_P(CMDE_ERROR_MSG);
			serial::write_uint16_t(error);
			serial::write_P(PSTR(" - "));
			switch (error) {																															break;
				case CMDE_INVALID		: serial::write_P(CMDE_SYSTEM_MSG);	serial::write_P(CMDE_INVALID_MSG);	serial::write_P(CMDE_NOT_SUPPORTED_MSG);	break;
				case CMDE_TARGET		: serial::write_P(CMDE_SYSTEM_MSG);	serial::write_P(CMDE_TARGET_MSG);	serial::write_P(CMDE_NOT_SUPPORTED_MSG);	break;
				case CMDE_OPTION		: serial::write_P(CMDE_SYSTEM_MSG);	serial::write_P(CMDE_OPTION_MSG);	serial::write_P(CMDE_NOT_SUPPORTED_MSG);	break;
				case CMDE_VALUE			: serial::write_P(CMDE_SYSTEM_MSG);	serial::write_P(CMDE_VALUE_MSG);	serial::write_P(CMDE_NOT_SUPPORTED_MSG);	break;
				case CMDE_LENGTH		: 									serial::write_P(CMDE_LENGTH_MSG);												break;
				case CMDE_CHECKSUM		: 									serial::write_P(CMDE_CHECKSUM_MSG);												break;
				case CMDE_EXPRESSION	: return;
				default: break;
			}
		}
		serial::write_line();
	}

	void write_setting(uint16_t setting, uint16_t value){
		serial::write('$'); serial::write_uint16_t(setting); serial::write_P(CMDP_SPACE); serial::write_uint16_t(value); serial::write_line();
	}

	void write_settings(){
		/*
			$100	- Serial Output 	(1/0)
			$101	- Buzzer 			(1/0)
		*/
		{
			config::system_config cfg;
			config::read_system_config(cfg);
			write_setting(CMD_SYSTEM_SERIAL, cfg.serial);
			write_setting(CMD_SYSTEM_BUZZER, cfg.buzzer);
		}

		/*
			$2N0=100
			$2N1=100
			$2N2=100
		*/
		uint16_t cmd = 200;
		for(uint8_t i = 0; i < 8; i++){
			config::input_config cfg;
			config::read_input_config(cfg, i);
			write_setting(cmd++, cfg.threshold);
			write_setting(cmd++, cfg.hysteresis);
			write_setting(cmd,	 cfg.inverted);
			cmd += 8;
		}
		/*
			$3N0=100
			$2N1=100
		*/
		cmd = 300;
		for(uint8_t i = 0; i < 8; i++){
			config::output_config cfg;
			config::read_output_config(cfg, i);
			write_setting(cmd++, cfg.active);
			char buffer[EXPRESSION_SIZE];
			config::read_expression(buffer, i);
			serial::write('$'); serial::write_uint16_t(cmd); serial::write_P(CMDP_SPACE); serial::write(buffer); serial::write_line();
			cmd += 9;
		}
		write_status(CMDE_OK);
	}

	void write_status(){
		if(!io::adc_is_busy()){
			io::adc_start_conversion();
		}

		io::adc_busy_wait();
		io::adc_start_conversion();

		uint8_t buffer[8];
		io::adc_update(buffer);

		for (uint8_t i = 0; i < 8; i++){
			serial::write_uint8_t(buffer[i]); serial::write(";");
		}
		serial::write_line();
	}

	void write_value(){}



	//I am really sorry but I paid a ton of money for a super ultra wide display and I'M GONNA USE ALL OF IT!
	void write_help(){
		/*
			#ctrl-x	- Reset

			$RST=$	- Restore defaults and reboot
			$RST=S	- Restore system defaults and reboot
			$RST=I	- Restore input defaults and reboot
			$RST=O	- Restore output defaults and reboot
			$RST=E	- Restore expression defaults and reboot

			$$ 	- View settings
			$S	- View status

			$100	- Serial Output 	(1/0)
			$101	- Buzzer 			(1/0)
		*/
		serial::write_P(PSTR("# Ctrl-x\t: Reset"));																	serial::write_line(2);		

		serial::write_P(PSTR("# Input"));																			serial::write_line();
		serial::write_P(PSTR("# $2N0 - Threshold\t: 0-100% Input(51%) >= 50% -> TRUE"));							serial::write_line();
		serial::write_P(PSTR("# $2N1 - Hysteresis\t: 0-100% Input(40%) <= (50% +/- 10%) -> FALSE"));				serial::write_line();
		serial::write_P(PSTR("# $2N2 - Inverted\t: Invert input/operand $[A-H]"));									serial::write_line(2);

		serial::write_P(PSTR("# Supported Sensors"));																serial::write_line();
		serial::write_P(PSTR("# NC/NO Switches connected to 12V and SENSE or SENSE and GND"));						serial::write_line();
		serial::write_P(PSTR("# Board powered NPN/PNP NO/NC proximity sensors"));									serial::write_line(2);

		serial::write_P(PSTR("# Example Switch NC"));																serial::write_line();
		serial::write_P(PSTR("# S-GND: $2N0=25 $2N1=5 $2N2 = 0"));													serial::write_line();
		serial::write_P(PSTR("# 12V-S: $2N0=75 $2N1=5 $2N2 = 1"));													serial::write_line(2);

		serial::write_P(PSTR("# Example Switch NO"));																serial::write_line();
		serial::write_P(PSTR("# S-GND: $2N0=25 $2N1=5 $2N2 = 1"));													serial::write_line();
		serial::write_P(PSTR("# 12V-S: $2N0=75 $2N1=5 $2N2 = 0"));													serial::write_line(2);

		serial::write_P(PSTR("# Example Sensor NPN NC"));															serial::write_line();
		serial::write_P(PSTR("# 12V-S-GND: $2N0=35 $2N1=5 $2N2 = 0"));												serial::write_line(2);

		serial::write_P(PSTR("# Example Sensor NPN NO"));															serial::write_line();
		serial::write_P(PSTR("# 12V-S-GND: $2N0=35 $2N1=5 $2N2 = 1"));												serial::write_line(2);

		serial::write_P(PSTR("# Example Sensor PNP NC"));															serial::write_line();
		serial::write_P(PSTR("# 12V-S-GND: $2N0=65 $2N1=5 $2N2 = 1"));												serial::write_line(2);

		serial::write_P(PSTR("# Example Sensor PNP NO"));															serial::write_line();
		serial::write_P(PSTR("# 12V-S-GND: $2N0=65 $2N1=5 $2N2 = 0"));												serial::write_line(3);


		serial::write_P(PSTR("# Output"));																			serial::write_line();
		serial::write_P(PSTR("# Active low open collector"));														serial::write_line();
		serial::write_P(PSTR("# $3N0 - Active\t\t: Enable disable output"));										serial::write_line();
		serial::write_P(PSTR("# $3N1 - Expression\t: Output trigger condition"));									serial::write_line(2);

		serial::write_P(PSTR("# Expression"));																		serial::write_line();
		serial::write_P(PSTR("# Comment\t: # text"));																serial::write_line();
		serial::write_P(PSTR("# Operands\t: $A $B $C $D $E $F $G $H"));												serial::write_line();
		serial::write_P(PSTR("# Operators\t: not('~') and('&') or('|') eq('=')"));									serial::write_line(2);

		serial::write_P(PSTR("# Example\t: XOR -> ($A & ~$B) | (~$A & $B) # (A or not B) or (not A and B)"));		serial::write_line();
		serial::write_P(PSTR("# Example\t: Multiple inputs mapped to one output -> 301 = $A | $B | $C"));			serial::write_line(3);


		serial::write_P(CMDP_RESET); serial::write('$'); serial::write_P(CMDP_RESTORE);											serial::write_P(CMDP_AND_REBOOT);	serial::write_line();
		serial::write_P(CMDP_RESET); serial::write('S'); serial::write_P(CMDP_RESTORE); serial::write_P(PSTR("system "));		serial::write_P(CMDP_AND_REBOOT);	serial::write_line();
		serial::write_P(CMDP_RESET); serial::write('I'); serial::write_P(CMDP_RESTORE); serial::write_P(PSTR("input "));		serial::write_P(CMDP_AND_REBOOT);	serial::write_line();
		serial::write_P(CMDP_RESET); serial::write('O'); serial::write_P(CMDP_RESTORE); serial::write_P(PSTR("output "));		serial::write_P(CMDP_AND_REBOOT);	serial::write_line();
		serial::write_P(CMDP_RESET); serial::write('E'); serial::write_P(CMDP_RESTORE); serial::write_P(PSTR("expression "));	serial::write_P(CMDP_AND_REBOOT);	serial::write_line(2);

		serial::write_P(PSTR("$$	- View settings"));									serial::write_line();
		serial::write_P(PSTR("$S	- View input status"));								serial::write_line(2);

		serial::write_P(PSTR("$100	- Serial Output 	(1/0)"));						serial::write_line();
		serial::write_P(PSTR("$101	- Buzzer 		(1/0)"));							serial::write_line(2);
		/*
			$2N0	- Input # threshold 	(0-100)
			$2N1	- Input # hysteresis 	(0-100)
			$2N2	- Input # inverted 		(1/0)
		*/
		uint16_t cmd = 200;
		for(uint8_t i = 0; i < 8; i++){
			serial::write_line(2);
			serial::write('$'); serial::write_uint16_t(cmd++);	serial::write_P(CMDP_INPUT); serial::write((char)('A' + i)); serial::write_P(CMDP_THRESHOLD);	serial::write_P(CMDP_0_100);	serial::write_line();
			serial::write('$'); serial::write_uint16_t(cmd++);	serial::write_P(CMDP_INPUT); serial::write((char)('A' + i)); serial::write_P(CMDP_HYSTERESIS);	serial::write_P(CMDP_0_100);	serial::write_line();
			serial::write('$'); serial::write_uint16_t(cmd);	serial::write_P(CMDP_INPUT); serial::write((char)('A' + i)); serial::write_P(CMDP_INVERT);		serial::write_P(CMDP_1_0);
			cmd += 8;
		}
		serial::write_line();
		/*
			$3N0	- Output # active 		(1/0)
			$3N1	- Output # expression 	("$A | $B ...")
		*/
		cmd = 300;
		for(uint8_t i = 0; i < 8; i++){
			serial::write_line(2);
			serial::write('$'); serial::write_uint16_t(cmd++);	serial::write_P(CMDP_OUTPUT); serial::write((char)('A' + i)); serial::write_P(CMDP_ACTIVE);		serial::write_P(CMDP_1_0);	serial::write_line();
			serial::write('$'); serial::write_uint16_t(cmd++);	serial::write_P(CMDP_OUTPUT); serial::write((char)('A' + i)); serial::write_P(CMDP_EXPRESSION);	serial::write_P(CMDP_A_B);
			cmd += 8;
		}
		serial::write_line();
		write_status(CMDE_OK);
	}

	//command parameter
	bool get_cmd_parameter(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer, uint16_t &target, uint16_t min, uint16_t max){

		const char* value_ptr = str::find_digit(str::find(buffer, '='));


		if(value_ptr != STR_END){

			int16_t cmd = atoi(value_ptr);

			if(cmd >= 0 && ((uint16_t)cmd >= min && (uint16_t)cmd <= max)){
				target = cmd;
				return true;
			}
		}

		write_status(CMDE_VALUE);
		return false;
	}

	//number of command digits
	uint16_t get_cmd_digits(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

			const char* digits_ptr = buffer.ptr() + 1;
			uint16_t dc = 0;
			while(str::is_digit(*digits_ptr++)){
				dc++;
			}
			return dc;
		}

	//100
	//200...
	cmd_type get_cmd_type(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer, uint16_t category){
		//check length
		if(get_cmd_digits(buffer) == 3){
			int16_t cmd = atoi(buffer.ptr() + 1);
			if(cmd > 0){
				cmd_type ct;
				ct.target = (cmd - category) / 10;
				ct.option = cmd % 10;
				ct.edit = (str::index_of(buffer, 3, '=') != STR_NO_MATCH);
				ct.valid = true;
				return ct;
			}
		}
		return cmd_type{};
	}

	//SYSTEM SETTINGS
	void handle_system_serial(cmd_type &ct, str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

		config::system_config cfg;
		config::read_system_config(cfg);

		if(ct.edit){
			uint16_t value = 0;
			if(get_cmd_parameter(buffer, value, 0, 1)){
				cfg.serial = (value == 1);
				config::write_system_config(cfg);
				write_status(CMDE_OK);
			}
		}
		else {
			write_setting(CMD_SYSTEM_SERIAL, cfg.serial? 1 : 0);
		}
	}

	void handle_system_buzzer(cmd_type &ct, str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

		config::system_config cfg;
		config::read_system_config(cfg);

		if(ct.edit){
			uint16_t value = 0;
			if(get_cmd_parameter(buffer, value, 0, 1)){
				cfg.buzzer = (value == 1);
				config::write_system_config(cfg);
				write_status(CMDE_OK);
			}
		}
		else {
			write_setting(CMD_SYSTEM_BUZZER, cfg.buzzer? 1 : 0);
		}

	}
	//SYSTEM SETTINGS


	//INPUT SETTINGS
	void handle_input_threshold(cmd_type &ct, str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

		config::input_config cfg;
		config::read_input_config(cfg, ct.target);

		if(ct.edit){
			uint16_t value = 0;
			if(get_cmd_parameter(buffer, value, 0, 100)){
				cfg.threshold = value;
				config::write_input_config(cfg, ct.target);
				write_status(CMDE_OK);
			}
		}
		else {
			write_setting(200 + (10 * ct.target) + ct.option, cfg.threshold);
		}
	}

	void handle_input_hysteresis(cmd_type &ct, str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

		config::input_config cfg;
		config::read_input_config(cfg, ct.target);

		if(ct.edit){
			uint16_t value = 0;
			if(get_cmd_parameter(buffer, value, 0, 100)){
				cfg.hysteresis = value;
				config::write_input_config(cfg, ct.target);
				write_status(CMDE_OK);
			}
		}
		else {
			write_setting(200 + (10 * ct.target) + ct.option, cfg.hysteresis);
		}
	}

	void handle_input_inverted(cmd_type &ct, str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

		config::input_config cfg;
		config::read_input_config(cfg, ct.target);

		if(ct.edit){
			uint16_t value = 0;
			if(get_cmd_parameter(buffer, value, 0, 1)){
				cfg.inverted = (value == 1);
				config::write_input_config(cfg, ct.target);
				write_status(CMDE_OK);
			}
		}
		else {
			write_setting(200 + (10 * ct.target) + ct.option, cfg.inverted? 1 : 0);
		}
	}
	//INPUT SETTINGS


	//OUTPUT SETTINGS
	void handle_output_active(cmd_type &ct, str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

		config::output_config cfg;
		config::read_output_config(cfg, ct.target);

		if(ct.edit){
			uint16_t value = 0;
			if(get_cmd_parameter(buffer, value, 0, 1)){
				cfg.active = (value == 1);
				config::write_output_config(cfg, ct.target);
				write_status(CMDE_OK);
			}
		}
		else {
			write_setting(300 + (10 * ct.target) + ct.option, cfg.active? 1 : 0);
		}
	}

	void handle_output_expression(cmd_type &ct, str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

		if(ct.edit){

			const char* exp_buffer = str::find(buffer, '=') + 1;
			opcodes::op_code op_buffer[EXPRESSION_SIZE];
			expr::expression exp;
			exp.data = (opcodes::op_code*)(&op_buffer[0]);
			parser::parser_result result;
			parser::parse_expression(exp_buffer, exp, result);

			if(result.status == parser::parser_status::OK){
				config::write_expression(exp_buffer, ct.target);
				write_status(CMDE_OK);
			}
			else {

				write_status(CMDE_EXPRESSION);
				serial::write_P(parser::get_compiler_status_msg(result.status));
				serial::write_P(CMDE_NEAR_MSG);
				serial::write_uint16_t(result.index);
				serial::write(" : ");
				serial::write(exp_buffer);
				serial::write_line();
			}
		}
		else {
			char exp_buffer[EXPRESSION_SIZE];
			config::read_expression(exp_buffer, ct.target);
			serial::write(exp_buffer);
			serial::write_line();
		}

	}
	//OUTPUT SETTINGS


	//RESET
	void handle_input_reset(){
		config::reset_input_config();
		io::reset_mcu();
	}

	void handle_output_reset(){
		config::reset_output_config();
		io::reset_mcu();
	}

	void handle_expression_reset(){
		config::reset_expressions();
		io::reset_mcu();
	}

	void handle_system_reset(){
		config::reset_input_config();
		io::reset_mcu();
	}

	void handle_complete_reset(){
		config::reset_system_config();
		config::reset_input_config();
		config::reset_output_config();
		config::reset_expressions();
		io::reset_mcu();
	}

	//RESET




	//$$
	void execute_cmd_view_settings(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){
		write_settings();
	}

	//$S
	void execute_cmd_view_status(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){
		write_status();
	}

	//$RST=$ $RST=I $RST=O
	void execute_cmd_reset(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

		if(buffer.length() != 6){
			write_status(CMDE_INVALID);
			return;
		}
		else {
			for(uint8_t i = 0; i < 5; i++){
				if(buffer.ptr()[i] != pgm_read_byte(PSTR("$RST=") + i )){
					write_status(CMDE_INVALID);
					return;
				}
			}
		}

		switch (buffer.ptr()[5])
		{
			case '$': //reset all
				handle_complete_reset();
				write_status(CMDE_OK);
				return;

			case 'S': //reset system
				handle_system_reset();
				write_status(CMDE_OK);
				return;

			case 'I': //reset inputs
				handle_input_reset();
				write_status(CMDE_OK);
				return;

			case 'O': //reset outputs
				handle_output_reset();
				write_status(CMDE_OK);
				return;

			case 'E': //reset expressions
				handle_expression_reset();
				write_status(CMDE_OK);
				return;

			default:
				write_status(CMDE_VALUE);
				return;
		}
	}

	//$100=1
	//$100
	void execute_cmd_system_option(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){
		cmd_type ct = get_cmd_type(buffer, 100);
		if(ct.valid){
			if(ct.target == 0){
				switch(ct.option){
					case 0: //serial out
						handle_system_serial(ct, buffer);
						return;
					case 1: //buzzer
						handle_system_buzzer(ct, buffer);
						return;
					default:
						write_status(CMDE_OPTION); //invalid target
						return;
				}
				} else {
				write_status(CMDE_TARGET); //invalid target
			}
		}
		else {
			write_status(CMDE_INVALID); //invalid command
		}
	}

	//$200=100
	//$200
	void execute_cmd_input_option(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){
		cmd_type ct = get_cmd_type(buffer, 200);
		if(ct.valid){
			if(ct.target >= 0 && ct.target <= 7){
				switch(ct.option){
					case 0: //threshold
						handle_input_threshold(ct, buffer);
						return;
					case 1: //hysteresis
						handle_input_hysteresis(ct, buffer);
						return;
					case 2: //inverted
						handle_input_inverted(ct, buffer);
						return;
					default:
						write_status(CMDE_OPTION); //invalid target
						return;
				}
			} else {
				write_status(CMDE_TARGET); //invalid target
			}
		}
		else {
			write_status(CMDE_INVALID); //invalid command
		}
	}

	//$300=100
	//$300
	void execute_cmd_output_option(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){
		cmd_type ct = get_cmd_type(buffer, 300);
		if(ct.valid){
			if(ct.target >= 0 && ct.target <= 7){
				switch(ct.option){
					case 0: //active
						handle_output_active(ct, buffer);
					return;

					case 1: //expression
						handle_output_expression(ct, buffer);
					return;

					default:
						write_status(CMDE_OPTION); //invalid target
					return;
				}
				} else {
				write_status(CMDE_TARGET); //invalid target
			}
		}
		else {
			write_status(CMDE_INVALID); //invalid command
		}
	}

	void execute_cmd_run(){
		io::clear_boot_cfg_flag();
		io::reset_mcu();
	}

	void handle_command(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer){

		const char* tmp = buffer.ptr();
		if(*tmp++ == '$'){
			//inputs
			switch(*tmp){
			case '1': //system
				execute_cmd_system_option(buffer);
				break;

			case '2': //input
				execute_cmd_input_option(buffer);
				break;

			case '3': //output
				execute_cmd_output_option(buffer);
				break;

			case '$': //view settings
				execute_cmd_view_settings(buffer);
				break;

			case 'S': //view status
				execute_cmd_view_status(buffer);
				break;

			case 'R': //reset
				execute_cmd_reset(buffer);
				break;

			case 'P': //run
				execute_cmd_run();
				break;

			case STR_END: //help
				write_help();
				break;

			default:
				//invalid command
				write_status(CMDE_INVALID);
				break;
			}
		}//if
	}
};


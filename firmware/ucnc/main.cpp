/*
 * ucnc.cpp
 *
 * Created: 16.01.2021 20:55:30
 * Author : AlrHa
 */ 

#define F_CPU	18432000UL
#define BAUD	115200UL
#define CUBRR	((F_CPU / 16 / BAUD) - 1)

#define EXP_TEXT_LIM 128
#define CONFIG_INITIALIZED 1
#define CONFIG_EN_PULLUP 2
#define CONFIG_EN_SERIAL 4


#define ERR_EEPROM_FULL 10
#define ERR_COMPILER 20

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>


#include "errors.h"
#include "avrtypes.h"
#include "opcodes.h"
#include "expression.h"
#include "compiler.h"
#include "terminal.h"

#include "hal.h"

struct InputConfig{
	uint8_t		threshold	= 0;
	uint8_t		hysteresis  = 0;
	bool		inverted	= false;
	bool		trigger     = false;
	op::OpCode  last_state  = op::OpCode::FALSE;
	
	
	op::OpCode update(uint8_t value){
		if(!inverted){
			uint8_t limit = hysteresis > threshold ? 0 : threshold - hysteresis;
			
			if(!trigger && value >= threshold){
				trigger = true;
				last_state = op::OpCode::TRUE;
			} else if(trigger && value < limit){
				trigger = false;
				last_state = op::OpCode::FALSE;
			}
		} else {
			uint8_t limit = hysteresis + threshold > 100 ? 100 : hysteresis + threshold;
			
			if(!trigger && value <= threshold){
				trigger = true;
				last_state = op::OpCode::TRUE;
			} else if(trigger && value > limit){
				trigger = false;
				last_state = op::OpCode::FALSE;
			}
		}		
		
		return last_state;
	}
	
} inputs[8];

struct OutputConfig{
	avr_size_t text_index = 0;
	bool active = false;
} outputs[8];

struct RuntimeConfig{
	bool enable_pullups = false;
	bool enable_serial = false;
} runtime_config;



void err::on_error(uint8_t ec) {
	while (true)
	{
		for(uint8_t c = 0; c < 4; c++){
			for (uint8_t i = 0; i < 8; i++) {
				set_input_indicators((0x01 << i) | (0x80 >> i));
				_delay_ms(80);
			}
		}		
		_delay_ms(250);
		
		for (uint8_t i = 0; i < 2; i++)
		{
			set_input_indicators(0xFF);
			_delay_ms(50);
			set_input_indicators(0x00);
			_delay_ms(50);
		}
		_delay_ms(250);
		
		set_input_indicators(0x01);
		_delay_ms(250);
		set_input_indicators(ec);	
		_delay_ms(2000);
			
		
		set_input_indicators(0xFF);
		_delay_ms(500);
		set_input_indicators(0x00);
		_delay_ms(500);
		set_input_indicators(0xFF);
	
	}
}


template<avr_size_t size>
void read_expression(OutputConfig* oc, str::StringBuffer<size>& target){
	
	for (avr_size_t i = oc->text_index; i < oc->text_index + size; i++)
	{
		char chr = str::END;
		eeprom_read_block((void*)&chr, (void*)i, 1);
		if(chr != str::END){
			target.append(chr);
		} else {
			return;
		}
	}
}

OutputConfig* find_next_expression(const OutputConfig* exp) {
	OutputConfig* next = nullptr;
	for (uint8_t i = 0; i < 8; i++)
	{
		if (outputs[i].text_index > exp->text_index) {
			if (!next || outputs[i].text_index < next->text_index) {
				next = &outputs[i];
			}
		}
	}
	return next;
}

void move_expression(OutputConfig* exp_source, OutputConfig* exp_target) {
	//uint8_t index_source = exp_source->text_index;
	uint8_t index_target = exp_target->text_index;

	str::StringBuffer<EXP_TEXT_LIM> source;
	read_expression(exp_source, source);
	
	eeprom_update_block(source.ptr(), (void*)(exp_target->text_index), source.length() + 1);

	exp_target->text_index = index_target + source.length() + 1;;
	exp_source->text_index = index_target;
}

template<avr_size_t size>
void update_expression(OutputConfig* exp, str::StringBuffer<size>& text) {
	
	OutputConfig* exp_next = find_next_expression(exp);
	
	while (exp_next) {
		move_expression(exp_next, exp);
		exp_next = find_next_expression(exp);
	}
	
	eeprom_update_block(text.ptr(), (void*)exp->text_index, text.length() + 1); 
}

void reset_configuration()
{
	avr_size_t offset = 0;
	
	uint8_t config = CONFIG_INITIALIZED;
	eeprom_update_block((void*)&config, (void*)offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	
	for (uint8_t i = 0; i < 8; i++)
	{
		inputs[i] = {50, 0, false};
		
		eeprom_update_block(&inputs[i].threshold, (void*)offset, sizeof(uint8_t));
		offset += sizeof(uint8_t);
		
		eeprom_update_block(&inputs[i].hysteresis, (void*)offset, sizeof(uint8_t));
		offset += sizeof(uint8_t);
		
		eeprom_update_block(&inputs[i].inverted, (void*)offset, sizeof(bool));
		offset += sizeof(bool);
	}
	
	
	for (uint8_t i = 0; i < 8; i++) {
		outputs[i] = {offset + (sizeof(OutputConfig) * 8) + (i * 3), true};
	}
	eeprom_update_block((void*)&outputs, (void*)offset, sizeof(OutputConfig) * 8);
	offset += sizeof(OutputConfig) * 8;
	
	const char buffer[] = "$A\0$B\0$C\0$D\0$E\0$F\0$G\0$H\0";
	eeprom_update_block((void*)&buffer, (void*)offset, sizeof(buffer));
	offset += sizeof(buffer);
}

void load_configuration(){
	avr_size_t offset = 0;
	
	uint8_t config = 0;
	eeprom_read_block((void*)&config, (void*)offset, 1);
	offset += sizeof(uint8_t);
	
	if((config & CONFIG_INITIALIZED) != CONFIG_INITIALIZED){
		reset_configuration();
	} else {
		
		if((config & CONFIG_EN_PULLUP) == CONFIG_EN_PULLUP){
			runtime_config.enable_pullups = true;
		} else {
			runtime_config.enable_pullups = false;
		}
		
		if((config & CONFIG_EN_SERIAL) == CONFIG_EN_SERIAL){
			runtime_config.enable_serial = true;
		} else {
			runtime_config.enable_serial = false;
		}
		
		
		//uint8_t	threshold	= 0;
		//uint8_t	hysteresis  = 0;
		//boolinverted	= false;
		for (uint8_t i = 0; i < 8; i++)
		{
			eeprom_read_block(&inputs[i].threshold, (void*)offset, sizeof(uint8_t));
			offset += sizeof(uint8_t);
			
			eeprom_read_block(&inputs[i].hysteresis, (void*)offset, sizeof(uint8_t));
			offset += sizeof(uint8_t);
			
			eeprom_read_block(&inputs[i].inverted, (void*)offset, sizeof(bool));
			offset += sizeof(bool);
			
			if(inputs[i].threshold > 100) inputs[i].threshold = 100;
			if(inputs[i].hysteresis > 100) inputs[i].hysteresis = 100;
		}

		eeprom_read_block(&outputs, (void*)offset, sizeof(OutputConfig) * 8);
		offset += sizeof(OutputConfig) * 8;
	}
}

void save_configuration()
{
	avr_size_t offset = 0;
	
	uint8_t config = CONFIG_INITIALIZED;
	if(runtime_config.enable_pullups){
		config |= CONFIG_EN_PULLUP; 
	}
	
	if(runtime_config.enable_serial){
		config |= CONFIG_EN_SERIAL;
	}
	eeprom_update_block(&config, (void*)offset, sizeof(uint8_t));	
	offset += sizeof(uint8_t);
	
	//uint8_t	threshold	= 0;
	//uint8_t	hysteresis  = 0;
	//boolinverted	= false;
	for (uint8_t i = 0; i < 8; i++)
	{
		eeprom_update_block(&inputs[i].threshold, (void*)offset, sizeof(uint8_t));
		offset += sizeof(uint8_t);
		
		eeprom_update_block(&inputs[i].hysteresis, (void*)offset, sizeof(uint8_t));
		offset += sizeof(uint8_t);
		
		eeprom_update_block(&inputs[i].inverted, (void*)offset, sizeof(bool));
		offset += sizeof(bool);
	}
	
	
	eeprom_update_block(&outputs, (void*)offset, sizeof(OutputConfig) * 8);
	offset += sizeof(OutputConfig) * 8;
}

avr_size_t get_eeprom_free(){
	
	int16_t eeprom_free = 1024;
	//init and pullup flag
	eeprom_free -= sizeof(uint8_t);
	//inputs
	eeprom_free -= (3 * 8);
	//outputs	
	eeprom_free -= (sizeof(OutputConfig) * 8);
	
	for (uint8_t oi = 0; oi < 8; oi++)
	{
		for (avr_size_t i = outputs[oi].text_index; i < 1024; i++)
		{
			char chr = str::END;
			eeprom_read_block((void*)&chr, (void*)i, 1);							
			eeprom_free--;
			if(chr == str::END){
				break;
			}
		}
	}
	
	if(eeprom_free < 0 ){		
		err::on_error(ERR_EEPROM_FULL);
	} else {
		return (avr_size_t)eeprom_free;
	}
	
}


__attribute__((noreturn)) void run_mode();
__attribute__((noreturn)) void edit_mode();


int main(void)
{	
	wdt_really_off();
    setup();
	
    clear_outputs();
    clear_input_indicators();
		
	load_configuration();	

	if(aux_btn_down()){
		edit_mode();		
	} else {
		run_mode();		
	}	
}



void handle_compiler_error(comp::CompilerResult result){
	if(runtime_config.enable_serial){
		term::write_line_P(comp::get_compiler_status_msg(result.status));	
	}
}

template<avr_size_t buffer_size, avr_size_t text_size>
comp::CompilerResult compile_expression(ctr::Array<op::OpCode, buffer_size>&	exp_buffer, 
										str::StringBuffer<text_size>&			text,
										ex::Expression&							expression){
	
	comp::CompilerResult				compiler_result;
	ctr::Array<op::OpCode, buffer_size>	stack_buffer;	
	
	expression.set_buffer(&exp_buffer[0]);	
	comp::compile_expression(text, expression, stack_buffer, compiler_result);
	
	return compiler_result;	
}


void print_status(uint8_t input_data, uint8_t output_data){
	term::hide_cursor();
	term::cursor_move(0,0);
	term::write_string_P(PSTR("INPUT : "));
	for (uint8_t i = 0; i < 8; i++){
		term::write_char('[');
		if((input_data & (0x80 >> i)) > 0){
			term::write_char('#');
			} else {
			term::write_char(' ');
		}
		term::write_char(']');
		term::write_char(' ');	
	}
	
	term::cursor_move(0,1);
	term::write_string_P(PSTR("OUTPUT: "));
	for (uint8_t i = 0; i < 8; i++){
		term::write_char('[');
		if((output_data & (0x80 >> i)) > 0){			
			term::write_char('#');			
		} else {
			term::write_char(' ');		
		}
		term::write_char(']');
		term::write_char(' ');
	}
	term::cursor_move(0,2);
	
	term::write_string_P(PSTR("ADCVAL: "));
	for (uint8_t i = 0; i < 8; i++){
		if(adc_value_read[i] < 100){
			term::write_char('0');
			if(adc_value_read[i] < 10){
				term::write_char('0');
			}			
		}
		term::write_uint8_t(adc_value_read[i]);
		term::write_char(' ');
	}
	
}


//RUN MODE	
__attribute__((noreturn)) void run_mode() {	
	
	term::clear_screen();
	
	ctr::Array<uint8_t, 256>		result_table;

	for(uint8_t i = 0; i < 8; i++){
				
		if(outputs[i].active){
			
			//read text expression from eeprom
			str::StringBuffer<EXP_TEXT_LIM> exp_text;
			read_expression(&outputs[i], exp_text);
		
			ctr::Array<op::OpCode, 128>		target_buffer;
			ex::Expression expression;
		
			comp::CompilerResult result = compile_expression(target_buffer, exp_text, expression);
			
			if(result.status == comp::CompilerStatus::OK){
				ctr::Array<op::OpCode, 128>	stack_buffer;
				//execute expression and update truth table		
				for (avr_size_t ti = 0; ti < result_table.length(); ti++)
				{				
					ex::ExpressionData	data((uint8_t)ti);
					op::OpCode exp_result = expression.eval(data, stack_buffer);
					if(exp_result == op::OpCode::TRUE){
						result_table[ti] |= (1 << (7 - i));
					} else {				
						result_table[ti] &= ~(1 << (7 - i));
					}
				}
			} else {								
				err::on_error(ERR_COMPILER);
			}
		} else {
			for (avr_size_t ti = 0; ti < result_table.length(); ti++) {				
				result_table[ti] &= ~(1 << (7 - i));				
			}
		}
		
	}
	
	
	if(runtime_config.enable_pullups){
		enable_pullups();
	}	
	
	while(true) {		
		
		adc_busy_wait();
		adc_start_conversion();		
		
		ex::ExpressionData	data;
		
		for (uint8_t i = 0; i < 8; i++) {
			inputs[i].update(adc_value_read[i]);
			data.set_at(i, inputs[i].update(adc_value_read[i]));
		}
		
		set_outputs(result_table[data.data()]);
		set_input_indicators(data.data());
		
		if(runtime_config.enable_serial){
			print_status(data.data(), result_table[data.data()]);
		}
	}
}
//RUN MODE













/*MENU############################################################*/
#define ROWS2D(D2) ((avr_size_t)(sizeof(D2) / sizeof(D2[0])))
#define COLS2D(D2) ((avr_size_t)(sizeof(D2[0])))

enum class RootMenuState : uint8_t{
	START,
	STATUS,
	INPUT,
	OUTPUT,
	CONFIG,
	SAVE,
	RESTORE,
	HELP,
	HARDRESET,
	EXIT
};

enum class EditMenuState : uint8_t{
	START,
	EXIT
};


void print_header(){
	term::write_line_P(PSTR("::[UCNC 1.0 - IvyBit 2021]"));
}

void print_table_row(){
	for(uint8_t i = 0; i < 8; i++){
		term::write_string_P(PSTR(" [   ] "));
	}
}

void handle_menu_start(RootMenuState& menu_root){
	term::clear_screen();
	term::hide_cursor();
	print_header();
	
	term::cursor_move(5, 10);
	term::write_string_P(PSTR("::[ ] : Display Live Device Status"));
	
	term::cursor_move(5, 11);
	term::write_string_P(PSTR("::[ ] : Open Input Setup"));
	
	term::cursor_move(5, 12);
	term::write_string_P(PSTR("::[ ] : Open Output Setup"));
	
	term::cursor_move(5, 13);
	term::write_string_P(PSTR("::[ ] : Open Configuration"));
			
	term::cursor_move(5, 14);
	term::write_string_P(PSTR("::[ ] : Open Help Menu"));
	
	term::cursor_move(5, 15);
	term::write_string_P(PSTR("::[ ] : Reset Device And Restore Defaults"));
	
	term::cursor_move(5, 16);
	term::write_string_P(PSTR("::[ ] : Exit Setup And Enter Run Mode"));
	
	uint8_t r = 0;
	
	while(menu_root == RootMenuState::START){
		
		term::cursor_move(8, r + 10);
		term::write_char('#');
		term::cursor_move(8, r + 10);
		
		term::ArrowKey ak = term::read_arrow();
		switch (ak)
		{
			case term::ArrowKey::UP:
			if(r > 0){
				term::write_char(' ');
				r--;
			}
			
			break;
			
			case term::ArrowKey::DOWN:
			if(r < 6){
				term::write_char(' ');
				r++;
			}
			break;
			
			case term::ArrowKey::SELECT:
			switch (r)
			{
				case 0: menu_root = RootMenuState::STATUS; break;
				case 1: menu_root = RootMenuState::INPUT; break;
				case 2: menu_root = RootMenuState::OUTPUT; break;
				case 3: menu_root = RootMenuState::CONFIG; break;
				case 4: menu_root = RootMenuState::HELP; break;
				case 5: menu_root = RootMenuState::HARDRESET; break;
				case 6: menu_root = RootMenuState::EXIT; break;
			}
			break;
			
			default : break;
		}
	}
	
	
	term::show_cursor();
}

void handle_menu_status(RootMenuState& menu_root){
	term::clear_screen();
	term::hide_cursor();
	print_header();
		
	uint8_t adc_buffer[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	char input = str::END;
	while ( input != ESC ){
		
		term::save_cursor();
		
		adc_start_conversion();
		adc_busy_wait();
		
		for(uint8_t i = 0; i < 8; i++){
			term::cursor_down(2);
			if(adc_buffer[i] != adc_value_read[i]){
				adc_buffer[i] = adc_value_read[i];
			
				
				if(inputs[i].update(adc_value_read[i]) == op::OpCode::TRUE){
					term::write_string_P(PSTR("::[\x1B[32m"));
				} else {
					term::write_string_P(PSTR("::[\x1B[31m"));
				}
				term::write_char(('A' + i));
				term::write_string_P(PSTR("\x1B[0m] - ["));
			
				
				bool highlight = true;
				bool threshold = true;	
						
				
				if(inputs[i].inverted){	
					term::write_string_P(PSTR("\x1B[100m"));				
					for(uint8_t p = 0; p < 62; p++) {
						if(highlight && (100.0 / 62.0) * p >= adc_value_read[i]){
							if(inputs[i].update(adc_value_read[i]) == op::OpCode::TRUE){
								term::write_string_P(PSTR("\x1B[42m"));
							} else {
								term::write_string_P(PSTR("\x1B[41m"));
							}
							highlight = false;
						}
						
						if(threshold && (100.0 / 62.0) * p >= inputs[i].threshold){
							threshold = false;
							term::write_char('|');
							} else {
							term::write_char(' ');
						}
					}	
					term::write_string_P(PSTR("\x1B[100m"));
				} else {
					if(inputs[i].update(adc_value_read[i]) == op::OpCode::TRUE){
						term::write_string_P(PSTR("\x1B[42m"));
					} else {
						term::write_string_P(PSTR("\x1B[41m"));
					}
					for(uint8_t p = 0; p < 62; p++) {
						if(highlight && (100.0 / 62.0) * p >= adc_value_read[i]){
							term::write_string_P(PSTR("\x1B[100m"));
							highlight = false;
						}	
					
						if(threshold && (100.0 / 62.0) * p >= inputs[i].threshold){
							threshold = false;
							term::write_char('|');
						} else {					
							term::write_char(' ');
						}									
					}
				}
				term::write_string_P(PSTR("\x1B[40m"));
				term::write_string_P(PSTR("] - ["));
				if(inputs[i].update(adc_value_read[i]) == op::OpCode::TRUE){
					term::write_string_P(PSTR("\x1B[32m"));
				} else {
					term::write_string_P(PSTR("\x1B[31m"));
				}
			
			
				char buffer[4];
				utoa(adc_value_read[i], buffer, 10);
				if(adc_value_read[i] < 100){
					term::write_char('0');
				}
				if(adc_value_read[i] < 10){
					term::write_char('0');
				}
			
				term::write_string(buffer);
			
				term::write_string_P(PSTR("\x1B[0m]\r"));	
			}
		 }
		 term::restore_cursor();
		 input = term::read_char_delay();
	}
	term::show_cursor();
	menu_root = RootMenuState::START;
}

void handle_menu_inputs(RootMenuState& menu_root) {
	term::clear_screen();
	term::hide_cursor();
	print_header();
	

	term::cursor_move(5, 10);
	term::write_string_P(PSTR("::[INP] :"));
	for(uint8_t i = 0; i < 8; i++){
		term::write_string_P(PSTR(" [_"));
		term::write_char('A' + i);
		term::write_string_P(PSTR("_] "));
	}
	term::cursor_move(5, 11);
	term::write_string_P(PSTR("::[INV] :"));
	print_table_row();
	
	term::cursor_move(5, 12);
	term::write_string_P(PSTR("::[LIM] :"));
	print_table_row();
	
	term::cursor_move(5, 13);
	term::write_string_P(PSTR("::[HYS] :"));
	print_table_row();
	
	for (uint8_t i = 0; i < 8; i++) 
	{
		{//INVERT
			term::cursor_move(17 + 7 * i, 11);
			if(inputs[i].inverted){
				term::write_char('#');
			} else {
				term::write_char(' ');
			}
		}//INVERT
		
		{//THRESHOLD
			term::cursor_move(16 + 7 * i, 12);
			char buffer[4];
			utoa(inputs[i].threshold, buffer, 10);
			if(inputs[i].threshold < 100){
				term::write_char(' ');
				if(inputs[i].threshold < 10){
					term::write_char(' ');					
				}				
			}
			term::write_string(buffer);
		}//THRESHOLD
		
		{//HYSTERESIS
			term::cursor_move(16 + 7 * i, 13);
			char buffer[4];
			utoa(inputs[i].hysteresis, buffer, 10);
			if(inputs[i].hysteresis < 100){
				term::write_char(' ');
				if(inputs[i].hysteresis < 10){
					term::write_char(' ');
				}
			}
			term::write_string(buffer);
		}//HYSTERESIS
	}
	
    term::show_cursor();
	uint8_t r = 0;
	uint8_t c = 0;

	while(menu_root == RootMenuState::INPUT){	
	
		term::cursor_move(17 + 7 * c, r + 11);	
	
		term::ArrowKey ak = term::read_arrow();
		switch (ak)
		{
			case term::ArrowKey::UP:
			if(r > 0){
				r--;
			}		
			break;
		
			case term::ArrowKey::DOWN:
			if(r < 2){
				r++;
			}
			break;
			
			case term::ArrowKey::LEFT:
			if(c > 0){
				c--;
			}
			break;
			
			case term::ArrowKey::RIGHT:
			if(c < 7){
				c++;
			}
			break;
		
			case term::ArrowKey::ESCAPE:
				menu_root = RootMenuState::START;
			break;
			
			case term::ArrowKey::SELECT:
			switch (r) {	
				//INPUT INVERTED
				case 0: {
					str::StringBuffer<1> s;
					term::cursor_move(0,23);
					term::write_string_P(PSTR("::[Invert Input? (y/n)]"));
					while(s.length() < 1 || (s[0] != 'y' && s[0] != 'n')){
						term::cursor_move(0,24);
						term::read_line(s, 1);
					}
					term::cursor_move(0,23);
					term::clear_line();
					term::cursor_move(0,25);
					term::clear_line();
					term::cursor_move(17 + 7 * c, r + 11);
					if(s[0] == 'y'){
						inputs[c].inverted = true;
						term::write_char('#');
					} else {
						inputs[c].inverted = false;
						term::write_char(' ');
					}
					
					
					
					if(c < 7)c++;
				} break;
				
				
				//INPUT LIMIT
				case 1: {
					str::StringBuffer<3> s;
					term::cursor_move(0,23);
					term::write_string_P(PSTR("::[Threshold? (0-100)]"));
					bool inp_valid = false;
					int value = 0;
					while(!inp_valid){
						term::cursor_move(0,24);
						term::read_line(s, 3);
						if(s.is_numeric()){
							value = atoi(s.ptr());
							if(value >= 0 && value <= 100){
								inp_valid = true;
							}
						}						
					}
					
					inputs[c].threshold = (uint8_t)value;
					
					term::cursor_move(0,23);
					term::clear_line();
					term::cursor_move(0,25);
					term::clear_line();
					term::cursor_move(16 + 7 * c, r + 11);
					if(value < 100){
						term::write_char(' ');
						if(value < 10){
							term::write_char(' ');
						}
					}
					
					term::write_string(s);
					if(c < 7)c++;
				} break;
				
				//INPUT HYSTERESIS
				case 2: {
					str::StringBuffer<3> s;
					term::cursor_move(0,23);
					term::write_string_P(PSTR("::[Hysteresis? (0-100)]"));
					bool inp_valid = false;
					int value = 0;
					while(!inp_valid){
						term::cursor_move(0,24);
						term::read_line(s, 3);
						if(s.is_numeric()){
							value = atoi(s.ptr());
							if(value >= 0 && value <= 100){
								inp_valid = true;
							}
						}
					}
					
					inputs[c].hysteresis = (uint8_t)value;
					
					term::cursor_move(0,23);
					term::clear_line();
					term::cursor_move(0,25);
					term::clear_line();
					term::cursor_move(16 + 7 * c, r + 11);
					if(value < 100){
						term::write_char(' ');
						if(value < 10){
							term::write_char(' ');
						}
					}
					
					term::write_string(s);
					if(c < 7)c++;
				} break;
	
			}
			break;
		}
	}
	
	save_configuration();
	term::show_cursor();
	menu_root = RootMenuState::START;
}

template<avr_size_t buffer_size>
bool handle_expression_edit(OutputConfig& output, str::StringBuffer<buffer_size>& exp_text){
	ctr::Array<op::OpCode, 128>	 target_buffer;
	ex::Expression				 expression;
	
	comp::CompilerResult result = compile_expression(target_buffer, exp_text, expression);
	
	
		
	const char* msg = comp::get_compiler_status_msg(result.status);	
	
	term::save_cursor();
	term::hide_cursor();
	term::cursor_move(5,20);	
	term::clear_line();
	if(msg){
		term::write_line_P(msg);
	} else {
		term::write_line_P(PSTR("Unknown error"));	
	}
	
	term::cursor_move(5,20);	
	_delay_ms(1000);
	term::clear_line();
	term::show_cursor();
	term::restore_cursor();		
		
	if(result.status == comp::CompilerStatus::OK){	
		update_expression(&output, exp_text);
		save_configuration();	
		return true;
	} else {		
		return false;
	}
}

void handle_menu_outputs(RootMenuState& menu_root) {
	term::clear_screen();
	term::hide_cursor();
	print_header();
	

	term::cursor_move(5, 10);
	term::write_string_P(PSTR("::[OUT] :"));
	for(uint8_t i = 0; i < 8; i++){
		term::write_string_P(PSTR(" [_"));
		term::write_char('A' + i);
		term::write_string_P(PSTR("_] "));		
	}
	
	term::cursor_move(5, 11);
	term::write_string_P(PSTR("::[ACT] :"));	
	print_table_row();
	
	term::cursor_move(5, 12);
	term::write_string_P(PSTR("::[EXP] :"));
	print_table_row();

	for (uint8_t i = 0; i < 8; i++)
	{
		//ACTIVE
		term::cursor_move(17 + 7 * i, 11);
		if(outputs[i].active){
			term::write_char('#');
		} else{			
			term::write_char(' ');
		}
		
		//EXPRESSION LENGTH
		term::cursor_move(16 + 7 * i, 12);
		term::write_string_P(PSTR("MOD"));
	}
	
	term::show_cursor();
	uint8_t r = 0;
	uint8_t c = 0;

	while(menu_root == RootMenuState::OUTPUT){
		
		
		{//show selected expression
			term::save_cursor();
			
			term::cursor_move(5,14);
			term::clear_line();		
			term::write_string_P(PSTR("::[MEM] : "));
			term::write_uint16_t(get_eeprom_free());
			term::write_string_P(PSTR(" bytes free"));
			
			
			str::StringBuffer<EXP_TEXT_LIM> expression;
			read_expression(&outputs[c], expression);
			
		
			term::cursor_move(5,16);
			term::clear_line();		
			term::write_string_P(PSTR("::[EXP] : "));		
			term::write_string(expression, 40);
			
			term::cursor_move(5,17);
			term::clear_line();		
			term::write_string_P(PSTR("::[LEN] : "));	
			term::write_uint16_t(expression.length());
			term::write_string_P(PSTR(" bytes"));			
		
			term::restore_cursor();
		}		
		
		term::cursor_move(17 + 7 * c, r + 11);
		
		term::ArrowKey ak = term::read_arrow();
		switch (ak) {
			case term::ArrowKey::UP:
				if(r > 0){
					r--;
				}
			break;
			
			case term::ArrowKey::DOWN:
				if(r < 1){
					r++;
				}
			break;
			
			case term::ArrowKey::LEFT:
				if(c > 0){
					c--;
				}
			break;
			
			case term::ArrowKey::RIGHT:
				if(c < 7){
					c++;
				}
			break;
			
			case term::ArrowKey::ESCAPE:
				menu_root = RootMenuState::START;
			break;
			
			case term::ArrowKey::SELECT:
			switch (r) {
				//INPUT ACTIVE
				case 0: {
					str::StringBuffer<1> s;
					term::cursor_move(0,23);
					term::write_string_P(PSTR("::[Enable Output? (y/n)]"));
					while(s.length() < 1 || (s[0] != 'y' && s[0] != 'n')){
						term::cursor_move(0,24);
						term::read_line(s, 1);
					}
					term::cursor_move(0,23);
					term::clear_line();
					term::cursor_move(0,25);
					term::clear_line();
					term::cursor_move(17 + 7 * c, r + 11);
					if(s[0] == 'y'){
						term::write_char('#');
						outputs[c].active = true;
					} else {
						term::write_char(' ');
						outputs[c].active = false;
					}
					save_configuration();
					if(c < 7)c++;
				} break;
				
				
				//INPUT EXPRESSION
				case 1: {
					str::StringBuffer<EXP_TEXT_LIM> expression;
					read_expression(&outputs[c], expression);
					
					term::cursor_move(0,23);
					term::write_string_P(PSTR("::[Expression?]"));
					term::cursor_move(0,24);
					
					bool expression_valid = false;
					do 
					{
						avr_size_t eeprom_free = get_eeprom_free();
						term::cursor_move(0,24);
						term::clear_line();
						term::read_line(expression, 60, eeprom_free);							
						expression_valid = handle_expression_edit(outputs[c], expression);
					} while (!expression_valid);	
										
					term::cursor_move(0,23);
					term::clear_line();
					term::cursor_move(0,25);
					term::clear_line();
					term::cursor_move(16 + 7 * c, r + 11);
					term::write_string_P(PSTR("MOD"));
					if(c < 7)c++;
				} break;
			}
			break;
		}
	}
			
				
	term::show_cursor();
	menu_root = RootMenuState::START;
}

void handle_menu_config(RootMenuState& menu_root){
	term::clear_screen();
	term::hide_cursor();
	print_header();
	
	
	term::cursor_move(5, 10);
	term::write_string_P(PSTR("::[ENP] [   ] : Enable Output Pull Ups "));
	if(runtime_config.enable_pullups){
		term::cursor_move(15, 10);
		term::write_char('#');
	}
	
	term::cursor_move(5, 11);
	term::write_string_P(PSTR("::[SSO] [   ] : Enable Serial Status Output"));
	if(runtime_config.enable_serial){
		term::cursor_move(15, 11);
		term::write_char('#');
	}
	
	term::show_cursor();
	uint8_t r = 0;
	
	while(menu_root == RootMenuState::CONFIG){
		term::cursor_move(15, r + 10);
	
		term::ArrowKey ak = term::read_arrow();
		
		switch (ak) {
			case term::ArrowKey::UP:
			if(r > 0){
				r--;
			}
			break;
		
			case term::ArrowKey::DOWN:
			if(r < 1){
				r++;
			}
			break;	
			
		
			case term::ArrowKey::ESCAPE:
				menu_root = RootMenuState::START;
			break;
		
			case term::ArrowKey::SELECT:
			switch (r) {
				//INPUT ACTIVE
				case 0: {
					str::StringBuffer<1> s;
					term::cursor_move(0,23);
					term::write_string_P(PSTR("::[Enable Output Pull Ups? (y/n)]"));
					while(s.length() < 1 || (s[0] != 'y' && s[0] != 'n')){
						term::cursor_move(0,24);
						term::read_line(s, 1);
					}
					term::cursor_move(0,23);
					term::clear_line();
					term::cursor_move(0,25);
					term::clear_line();
					term::cursor_move(15, r + 10);
					if(s[0] == 'y'){
						term::write_char('#');
						runtime_config.enable_pullups = true;
					} else {
						term::write_char(' ');
						runtime_config.enable_pullups = false;
					}
					save_configuration();
				} break;
			
			
				//INPUT EXPRESSION
				case 1: {
					str::StringBuffer<1> s;
					term::cursor_move(0,23);
					term::write_string_P(PSTR("::[Enable Serial Status Output? (y/n)]"));
					while(s.length() < 1 || (s[0] != 'y' && s[0] != 'n')){
						term::cursor_move(0,24);
						term::read_line(s, 1);
					}
					term::cursor_move(0,23);
					term::clear_line();
					term::cursor_move(0,25);
					term::clear_line();
					term::cursor_move(15, r + 10);
					if(s[0] == 'y'){
						term::write_char('#');
						runtime_config.enable_serial = true;
					} else {
						term::write_char(' ');
						runtime_config.enable_serial = false;
					}
					save_configuration();
				} break;
				
			}
			break;
			default: break;
		}
	
		
	}
	menu_root = RootMenuState::START;
}

void handle_menu_help(RootMenuState& menu_root){
	term::clear_screen();
	term::hide_cursor();
	print_header();
	
	
	term::cursor_down(2);
	

	term::write_string("continue...");
	while(term::read_char() != CR);
	menu_root = RootMenuState::START;
}

void handle_menu_reset(RootMenuState& menu_root){
	term::clear_screen();
	term::hide_cursor();
	print_header();	
	
	term::cursor_move(29, 14);
	term::write_string_P(PSTR("Are you sure? yes/no"));
	
	term::cursor_move(0,24);
	term::clear_line();
	str::StringBuffer<3> input;
	term::read_line(input, 5);
	
	if(str::index_of_P(input, PSTR("yes")) != str::NO_MATCH){
		reset_configuration();
		load_configuration();
	}

	menu_root = RootMenuState::START;
}


//EDIT MODE
__attribute__((noreturn)) void edit_mode(){
	
	while (true) {
		RootMenuState menu_root = RootMenuState::START; //ROOT MENU
	
		while(menu_root != RootMenuState::EXIT){
			term::clear_screen();
		
			switch (menu_root)
			{
				case RootMenuState::START:
					handle_menu_start(menu_root);
				break;
			
				case RootMenuState::STATUS:
					handle_menu_status(menu_root);
				break;
				
				case RootMenuState::INPUT:
					handle_menu_inputs(menu_root);
				break;
				
				case RootMenuState::OUTPUT:
					handle_menu_outputs(menu_root);
				break;	
				
				case RootMenuState::CONFIG:
					handle_menu_config(menu_root);
				break;			
				
				case RootMenuState::HELP:
					handle_menu_help(menu_root);
				break;
				
				case RootMenuState::HARDRESET:
					handle_menu_reset(menu_root);
				break;
			
				case RootMenuState::EXIT:
					menu_root = RootMenuState::EXIT;
				break;
				
				default: break;
			}	
		}
		
		term::clear_screen();
		reset_mcu();
	}
}
//EDIT MODE	

/*MENU############################################################*/	
	



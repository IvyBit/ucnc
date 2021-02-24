/*
 * ucnc.cpp
 *
 * Created: 16.01.2021 20:55:30
 * Author : AlrHa
 */ 
#define AVR

#define F_CPU	18432000UL
#define BAUD	115200UL
#define CUBRR	((F_CPU / 16 / BAUD) - 1)

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

#include "edit.h"

#include "hal.h"

struct InputConfig{
	uint8_t		threshold	= 0;
	uint8_t		hysteresis  = 0;
	bool		inverted	= false;
	op::OpCode  last_state  = op::OpCode::FALSE;
	
	op::OpCode update(uint8_t value){
		if(value > threshold + hysteresis){
			if(inverted){
				last_state = op::OpCode::FALSE;
			} else {
				last_state =  op::OpCode::TRUE;
			}
		} else if(value < threshold - hysteresis){
			if(inverted){
				last_state =  op::OpCode::TRUE;
			} else {
				last_state =  op::OpCode::FALSE;
			}
		}
		
		return last_state;
	}
};

struct OutputConfig{
	avr_size_t text_index = 0;
	bool active = false;
};

volatile uint8_t	adc_values[16]	= {0};
volatile uint8_t*	adc_value_write	= nullptr;
volatile uint8_t*	adc_value_read	= nullptr;
volatile bool	    adc_busy		= false;

InputConfig			inputs[8];
OutputConfig		outputs[8];

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
	uint8_t index_source = exp_source->text_index;
	uint8_t index_target = exp_target->text_index;

	str::StringBuffer<120> source;
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

template<avr_size_t buffer_size>
void save_expression(str::StringBuffer<buffer_size> text){
	
}






__attribute__((noreturn)) void run_mode();
__attribute__((noreturn)) void edit_mode();


void load_configuration();
void save_configuration();
void reset_configuration();

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


void load_configuration(){
	avr_size_t offset = 0;
	
	uint8_t initialized = 0;
	eeprom_read_block((void*)&initialized, (void*)0, 1);
	offset += sizeof(uint8_t);
	
	if(initialized != 0xFF){
		reset_configuration();
	} else {
		
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

void reset_configuration() 
{
	avr_size_t offset = 0; 
	
	uint8_t initialized = 0xFF;
	eeprom_update_block((void*)&initialized, (void*)offset, sizeof(uint8_t));
	offset += sizeof(uint8_t);	
	
	for (uint8_t i = 0; i < 8; i++)
	{		
		inputs[i] = {45, 0, false};
			
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

void save_configuration()
{	
	avr_size_t offset = 1; 
	
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


//RUN MODE	
__attribute__((noreturn)) void run_mode() {	
	
	ctr::Array<op::OpCode, 128>		target_buffer;
	comp::ExpressionCompiler		compiler;
	comp::CompilerResult			compiler_result;
	ctr::Array<uint8_t, 256>		truth_table;
	
	for(uint8_t i = 0; i < 8; i++){
		
		if(outputs[i].active){
			
			str::StringBuffer<128>		exp_text;
			ctr::Array<op::OpCode, 128>	stack_buffer;
		
			//read text expression from eeprom
			read_expression(&outputs[i], exp_text);
		
			ex::Expression exp;
			exp.set_buffer(&target_buffer[0]);
			
					
			//compile expression
			compiler.compile_expression(exp_text, exp, stack_buffer, compiler_result);
		
			//execute expression and update truth table		
			for (avr_size_t ti = 0; ti < truth_table.length(); ti++)
			{
				ex::ExpressionData	data((uint8_t)ti);
				op::OpCode exp_result = exp.eval(data, stack_buffer);
				if(exp_result == op::OpCode::TRUE){
					truth_table[ti] |= (1 << (7 - i));
				} else {				
					truth_table[ti] &= ~(1 << (7 - i));
				}
			}
		} else {
			for (avr_size_t ti = 0; ti < truth_table.length(); ti++) {				
				truth_table[ti] &= ~(1 << (7 - i));				
			}
		}
		
	}
	
	ex::ExpressionData	data;
	while(true) {		
		
		adc_busy_wait();
		adc_start_conversion();		
		
		for (uint8_t i = 0; i < 8; i++) {
			inputs[i].update(adc_value_read[i]);
			data.set_at(i, inputs[i].update(adc_value_read[i]));
		}
		
		set_input_indicators(data.data());
		set_outputs(truth_table[data.data()]);
	}
}
//RUN MODE













/*MENU############################################################*/

void print_header(){
	term::write_line_P(PSTR("\x1b[90m::[\x1b[37mUCNC 1.0 - IvyBit 2021\x1b[90m]\x1b[0m\r\n"));
}

void handle_menu_start(RootMenuState& menu_root){
	term::clear_screen();
	term::hide_cursor();
	print_header();
	print_banner();
	
	term::cursor_move(5, 10);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[ ] : \x1B[37mDisplay Live Device Status\x1B[0m"));
	
	term::cursor_move(5, 11);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[ ] : \x1B[37mOpen Input Setup\x1B[0m"));
	
	term::cursor_move(5, 12);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[ ] : \x1B[37mOpen Output Setup\x1B[0m"));
			
	term::cursor_move(5, 13);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[ ] : \x1B[37mOpen Help Menu\x1B[0m"));
	
	term::cursor_move(5, 14);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[ ] : \x1B[37mReset Device And Restore Defaults\x1B[0m"));
	
	term::cursor_move(5, 15);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[ ] : \x1B[37mExit Setup And Enter Run Mode\x1B[0m"));
	
	uint8_t r = 0;
	
	while(menu_root == RootMenuState::START){
		
		term::cursor_move(8, r + 10);
		term::write_string_P(PSTR("\x1b[33m#\x1B[0m"));
		term::cursor_move(8, r + 10);
		
		term::ArrowKey ak = term::read_arrow();
		switch (ak)
		{
			case term::ArrowKey::UP:
			if(r > 0){
				term::write_string_P(PSTR("\x1B[0m "));
				r--;
			}
			
			break;
			
			case term::ArrowKey::DOWN:
			if(r < 5){
				term::write_string_P(PSTR("\x1B[0m "));
				r++;
			}
			break;
			
			case term::ArrowKey::SELECT:
			switch (r)
			{
				case 0: menu_root = RootMenuState::STATUS; break;
				case 1: menu_root = RootMenuState::INPUT; break;
				case 2: menu_root = RootMenuState::OUTPUT; break;
				case 3: menu_root = RootMenuState::HELP; break;
				case 4: menu_root = RootMenuState::HARDRESET; break;
				case 5: menu_root = RootMenuState::EXIT; break;
			}
			break;
		}
	}
	
	
	term::show_cursor();
}

void handle_menu_status(RootMenuState& menu_root){
	term::clear_screen();
	term::hide_cursor();
	print_header();
	print_banner();
	term::cursor_up();
		
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
			
				term::write_string_P(PSTR("::[\x1B[33m"));
				term::write_char(('A' + i));
				term::write_string_P(PSTR("\x1B[0m] - ["));
			
				bool highlight = true;
				term::write_string_P(PSTR("\x1B[43m"));
				for(uint8_t p = 0; p < 62; p++) {
					if(highlight && (100.0 / 62.0) * p >= adc_value_read[i]){
						term::write_string_P(PSTR("\x1B[100m"));
						highlight = false;
					}
					
					term::write_char(' ');
				}
				term::write_string_P(PSTR("\x1B[40m"));
				term::write_string_P(PSTR("] - [\x1B[33m"));
			
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
	print_banner();
	

	term::cursor_move(5, 10);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[\x1b[33mINP\x1b[0m] : [_A_]__[_B_]__[_C_]__[_D_]__[_E_]__[_F_]__[_G_]__[_H_]"));
	
	term::cursor_move(5, 11);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[\x1b[33mINV\x1b[0m] : [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]"));
	
	term::cursor_move(5, 12);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[\x1b[33mLIM\x1b[0m] : [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]"));
	
	term::cursor_move(5, 13);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[\x1b[33mHYS\x1b[0m] : [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]"));
	
	for (uint8_t i = 0; i < 8; i++) 
	{
		{//INVERT
			term::cursor_move(17 + 7 * i, 11);
			if(inputs[i].inverted){
				term::write_string_P(PSTR("\x1b[33m#\x1b[0m"));
				} else {
				term::write_string_P(PSTR("\x1b[33m \x1b[0m"));
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
			term::write_string_P(PSTR("\x1b[33m"));
			term::write_string(buffer);
			term::write_string_P(PSTR("\x1b[0m"));
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
			term::write_string_P(PSTR("\x1b[33m"));
			term::write_string(buffer);
			term::write_string_P(PSTR("\x1b[0m"));
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
						term::write_string_P(PSTR("\x1b[31m#\x1b[0m"));
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
					term::write_string_P(PSTR("::[Input Threshold? (0-100)]"));
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
					
					term::write_string_P(PSTR("\x1b[31m"));
					term::write_string(s);
					term::write_string_P(PSTR("\x1b[0m"));
					if(c < 7)c++;
				} break;
				
				//INPUT HYSTERESIS
				case 2: {
					str::StringBuffer<3> s;
					term::cursor_move(0,23);
					term::write_string_P(PSTR("::[Input Hysteresis? (0-100)]"));
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
					
					term::write_string_P(PSTR("\x1b[31m"));
					term::write_string(s);
					term::write_string_P(PSTR("\x1b[0m"));
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

void handle_menu_outputs(RootMenuState& menu_root) {
	term::clear_screen();
	term::hide_cursor();
	print_header();
	print_banner();
	

	term::cursor_move(5, 10);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[\x1b[33mOUT\x1b[0m] : [_A_]__[_B_]__[_C_]__[_D_]__[_E_]__[_F_]__[_G_]__[_H_]"));
	
	term::cursor_move(5, 11);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[\x1b[33mACT\x1b[0m] : [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]"));
	
	term::cursor_move(5, 12);
	term::write_string_P(PSTR("\x1B[37m::\x1B[0m[\x1b[33mEXP\x1b[0m] : [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]  [   ]"));
	

	for (uint8_t i = 0; i < 8; i++)
	{
		//ACTIVE
		term::cursor_move(17 + 7 * i, 11);
		if(outputs[i].active){			
			term::write_string_P(PSTR("\x1b[33m#\x1b[0m"));
		} else{			
			term::write_char(' ');
		}
		
		//EXPRESSION LENGTH
		term::cursor_move(16 + 7 * i, 12);
		term::write_string_P(PSTR("\x1b[33mMOD\x1b[0m"));
	}
	
	term::show_cursor();
	uint8_t r = 0;
	uint8_t c = 0;

	while(menu_root == RootMenuState::OUTPUT){
		
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
						term::write_string_P(PSTR("\x1b[31m#\x1b[0m"));
						outputs[c].active = true;
					} else {
						term::write_char(' ');
						outputs[c].active = false;
					}
					if(c < 7)c++;
				} break;
				
				
				//INPUT EXPRESSION
				case 1: {
					str::StringBuffer<120> s;
					read_expression(&outputs[c], s);
					//s.append("(~$A & $B) | ($A & ~$B)");
					term::cursor_move(0,23);
					term::write_string_P(PSTR("::[Expression?]"));
					term::cursor_move(0,24);
					term::read_line(s, 60);
					
					update_expression(&outputs[c], s);
										
					term::cursor_move(0,23);
					term::clear_line();
					term::cursor_move(0,25);
					term::clear_line();
					term::cursor_move(16 + 7 * c, r + 11);
					term::write_string_P(PSTR("\x1b[31mMOD\x1b[0m"));
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


void handle_menu_help(RootMenuState& menu_root){
	term::clear_screen();
	
	
	term::cursor_down(2);
	

	term::write_string("continue...");
	while(term::read_char() != CR);
	menu_root = RootMenuState::START;
}

void handle_menu_reset(RootMenuState& menu_root){
	term::clear_screen();
	
	
	term::cursor_down(2);
	
	reset_configuration();
	load_configuration();

	term::write_string("continue...");
	while(term::read_char() != CR);
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
		//reset();
	}
}
//EDIT MODE	

/*MENU############################################################*/	
	




#pragma once

//https://espterm.github.io/docs/VT100%20escape%20codes.html //

#define ESC (27)
#define CH_A ('A')
#define CH_B ('B')
#define CH_C ('C')
#define CH_D ('D')
#define DEL (127)
#define BS  (8)
#define CR  (13)

#include "avrtypes.h"
#include "strings.h"
#include <stdlib.h>
namespace term {
	
	enum class ArrowKey: uint8_t{
		LEFT,
		RIGHT,
		UP,
		DOWN,
		SELECT,
		ESCAPE
	};
	
	
	char read_char() {
		while ( !(UCSR0A & (1 << RXC0)) );
		return UDR0;
	}
	
	char read_char_delay() {
		if((UCSR0A & (1 << RXC0))){
			return UDR0;			
		}else{
			_delay_ms(20);
			if((UCSR0A & (1 << RXC0))){				
				return UDR0;
			}else{
				return str::END;
			}
		}
	}
	
	ArrowKey read_arrow(){
		while(true){
			char input = term::read_char();
			if(input == ESC){
				input = term::read_char_delay();
				if(input == '['){
					input = term::read_char();
					//left
					if(input == CH_D){
						return ArrowKey::LEFT;
						//right
						} else if(input == CH_C){
						return ArrowKey::RIGHT;
						//up
						} else if(input == CH_A){
						return ArrowKey::UP;
						//down
						} else if(input == CH_B){
						return ArrowKey::DOWN;
					}
				} else if(input == str::END){
					return ArrowKey::ESCAPE;
				}
			} else if(input == CR){
				return ArrowKey::SELECT;
			}
		}
	}


    void write_char(const char chr) {
		while ( !( UCSR0A & (1<<UDRE0)) );
		UDR0 = chr;
    }
	
	void write_string_P(const char* src){
		for (; pgm_read_byte(src) != '\0'; src++)
		{			
			write_char(pgm_read_byte(src));
		}
	}

    void write_string(const char* src) {
        for (; *src != '\0'; src++)
        {
            write_char(*src);
        }
    }
	
	void write_line_P(const char* src) {
		for (; pgm_read_byte(src) != '\0'; src++)
		{
			write_char(pgm_read_byte(src));
		}
		write_string("\r\n");
	}
		
	void write_line(const char* src) {
		for (; *src != '\0'; src++)
		{
			write_char(*src);
		}
		write_string("\r\n");
	}
	

    void show_cursor(){
	    write_string("\x1B[?25h");
    }
    
    void hide_cursor(){
	    write_string("\x1B[?25l");
    }
    	

    //move cursor to top left position
    void cursor_home() {
        write_string("\x1B[H");
    }    


    //move cursor up 1 line
    void cursor_up() {
        write_string("\x1B[1A");
    }

    //move cursor up N lines
    void cursor_up(uint8_t lines) {
        char str[4]{ 0 };
        utoa(lines, &str[0], 10);
        
        write_string("\x1B[");
        write_string(str);
        write_char('A');
    }



    //move cursor down 1 line
    void cursor_down() {    
        write_string("\x1B[1B");
    }

    //move cursor down N lines
    void cursor_down(uint8_t lines) {
        char str[4]{ 0 };
        utoa(lines, &str[0], 10);
   
        write_string("\x1B[");
        write_string(str);
        write_char('B');
    }




    //move cursor left 1 column
    void cursor_left() {
        write_string("\x1B[1D");
    }

    //move cursor left N column
    void cursor_left(uint8_t cols) {
		if(cols > 0){
			char str[4]{ 0 };
			utoa(cols, &str[0], 10);

			write_string("\x1B[");
			write_string(str);
			write_char('D');
		}
    }



    //move cursor right 1 column
    void cursor_right() {
        write_string("\x1B[1C");
    }

    //move cursor right N columns
    void cursor_right(uint8_t cols) {
		if(cols > 0) {
			char str[4]{ 0 };
			utoa(cols, &str[0], 10);
			
			write_string("\x1B[");
			write_string(str);
			write_char('C');
		}
    }



    //move cursor to screen location (0,0)
    void cursor_move(uint8_t column, uint8_t line) {
        char colstr[4]{ 0 };
        char linstr[4]{ 0 };
        utoa(column + 1, &colstr[0], 10);
        utoa(line + 1, &linstr[0], 10);

        write_string("\x1B[");
        write_string(linstr);
        write_char(';');
        write_string(colstr);
        write_char('H');
    }


	void disable_input(){
		term::write_char(ESC);
		term::write_string("[2h");
	}
	
	void enable_input(){
		term::write_char(ESC);
		term::write_string("[2l");
	}
	
	void save_cursor(){
		term::write_char(ESC);
		term::write_char('7');
	}
	
	void restore_cursor(){
		term::write_char(ESC);
		term::write_char('8');
	}
	
	void clear_line(){
		write_string("\x1B[K");
	}
	
	 void clear_screen() {
		term::cursor_home();
		for (uint8_t r = 0; r < 25; r++)
		{
			term::clear_line();
			term::cursor_down();
		}
		term::cursor_home();
	 }
	 

	
	/*
	::[?]:
	*/
	const char INPUT[] PROGMEM ="::[\x1b[33m?\x1b[0m] : ";
	template<avr_size_t buffer_size>
	void read_line(str::StringBuffer<buffer_size>& dest, avr_size_t view_width){
		
		avr_size_t view_index = 0;
		avr_size_t cursor_index = 0;
		term::write_string_P(INPUT);
		
		while (true)
		{
			term::save_cursor();
			term::hide_cursor();
			term::cursor_left(cursor_index + sizeof(INPUT));
			term::write_string_P(INPUT);
			term::write_string("\x1b[33m");
			for (avr_size_t index = 0; index < view_width; index++)
			{
				if(dest[index + view_index] != '\0'){
					if(index + view_index < buffer_size){
						term::write_char(dest[index + view_index]);
					}
					} else {
					term::write_char(' ');
				}
			}
			term::write_string("\x1B[0m");
			term::show_cursor();
			term::restore_cursor();
			
			char input = term::read_char();
			
			if(input == ESC){
				input = term::read_char();
				if(input == '['){
					input = term::read_char();
					//right
					if(input == CH_C){
						if(view_index + cursor_index < dest.length()){
							if(cursor_index + 1 < view_width){
								cursor_index++;
								term::cursor_right();
							} else if(view_index < buffer_size){
								view_index++;
							}
						}
						//left
						} else if(input == CH_D){
						if(cursor_index  > 0){
							cursor_index--;
							term::cursor_left();
						} else if(view_index > 0){
							view_index--;
						}
					}
				}
				}else if((str::is_graph(input) || input == ' ') && dest.length() < (int16_t)buffer_size){
					
					if(view_index + cursor_index < dest.length() && dest.length() != 0){
						dest.insert(input, view_index + cursor_index);
					} else {
						dest.append(input);
					}
					if(cursor_index < view_width){
						cursor_index++;
						term::cursor_right();
					} else {
						view_index++;
					}
					
				}else if(input == DEL){
					
					if(view_index + cursor_index < dest.length() && dest.length() > 0){
						dest.remove_at(view_index + cursor_index, 1);
					}
					
				}else if(input == BS){
					
					if(view_index + cursor_index  > 0 && dest.length() > 0){
						dest.remove_at(view_index + cursor_index - 1, 1);
						if(cursor_index > 0){
							cursor_index--;
							term::cursor_left();
						} else {
							view_index--;
						}
					}
					
				}else if(input == CR){
					return;
				}
		}//WHILE
	}	
	


			
};
/*  
   _________________________________________________________________________
   ::[START]                                   ::[IvyBit 2021 - UCNC v. 1.0]

   ______/\/\/\/\__________________________/\/\/\/\/\____/\/\______/\/\_____      
   _______/\/\____/\/\__/\/\__/\/\__/\/\__/\/\____/\/\__________/\/\/\/\/\__ 
   ______/\/\____/\/\__/\/\__/\/\__/\/\__/\/\/\/\/\____/\/\______/\/\_______  
   _____/\/\______/\/\/\______/\/\/\/\__/\/\____/\/\__/\/\______/\/_________   
   __/\/\/\/\______/\____________/\/\__/\/\/\/\/\____/\/\/\____/\/\/\_______    
   _______________________/\/\/\/\__________________________________________     
                                                                         
   ::[MENU]
   ::[INFO] : 1 - Show System Informations
   ::[EDIT] : 2 - Enter Editor Mode
   ::[HELP] : 3 - Show Help
   ::[EXIT] : 4 - Exit And Run

   ::[?] : hel_

   _________________________________________________________________________
   ::[EDIT]                                    ::[IvyBit 2021 - UCNC v. 1.0]
   
   ______/\/\/\/\__________________________/\/\/\/\/\____/\/\______/\/\_____      
   _______/\/\____/\/\__/\/\__/\/\__/\/\__/\/\____/\/\__________/\/\/\/\/\__ 
   ______/\/\____/\/\__/\/\__/\/\__/\/\__/\/\/\/\/\____/\/\______/\/\_______  
   _____/\/\______/\/\/\______/\/\/\/\__/\/\____/\/\__/\/\______/\/_________   
   __/\/\/\/\______/\____________/\/\__/\/\/\/\/\____/\/\/\____/\/\/\_______    
   _______________________/\/\/\/\__________________________________________
   
   ::[MENU]
   ::[MODIFY] : 2 - Load And Modify Pin Expression
   ::[TEST]   : 3 - Run Expression
   ::[INPUTS] : 4 - Configure Inputs
   ::[PRINT]  : 5 - Print Truth Table
   ::[HELP]   : 1 - Show Help
   ::[EXIT]   : 0 - Back To Previous Menu


   ::[?] : 1_
   
   
   _________________________________________________________________________
   ::[MODIFY]                                  ::[IvyBit 2021 - UCNC v. 1.0]
   
   ______/\/\/\/\__________________________/\/\/\/\/\____/\/\______/\/\_____      
   _______/\/\____/\/\__/\/\__/\/\__/\/\__/\/\____/\/\__________/\/\/\/\/\__ 
   ______/\/\____/\/\__/\/\__/\/\__/\/\__/\/\/\/\/\____/\/\______/\/\_______  
   _____/\/\______/\/\/\______/\/\/\/\__/\/\____/\/\__/\/\______/\/_________   
   __/\/\/\/\______/\____________/\/\__/\/\/\/\/\____/\/\/\____/\/\/\_______    
   _______________________/\/\/\/\__________________________________________
   
   ::[MENU]
   ::[OUTPUT A-H] : AH - Select Output 
   ::[HELP]       :  1 - Show Help
   ::[EXIT]       :  0 - Back To Previous Menu

   ::[OUTPUT?] : A_

   ::[EXPRESSION?] : ($A & $B & $C)_
   ::[ERROR] : ...
   ::[OK] : ...
   ::[SET EXPRESSION A]




   
   _________________________________________________________________________                                                                                  
   ::[INFO]                                    ::[IvyBit 2021 - UCNC v. 1.0]

   ::[MEMORY]   
   ::[###############################[66%]###############__________________]   
   ::[Total] :  1024 Bytes    ::[Used] :   687 Bytes  ::[Free] :   337 Bytes  
   ::[A] : 12 Bytes   ::[B] : 12 Bytes   ::[C] : 12 Bytes   ::[D] : 12 Bytes
   ::[E] : 12 Bytes   ::[F] : 12 Bytes   ::[G] : 12 Bytes   ::[H] : 12 Bytes

   ::[PIN CONFIG]
   ::[A] : $A | $B
   ::[B] : ~$A & $E
   ::[C] : (~$A & $B) || ($A & ~$B)
   ::[D] : $G = $F
   ::[E] : (~$A & $E) ~= $E
   ::[F] : ~$E
   ::[G] : ~$F
   ::[H] : $A | $B | $C | $D | $E | $F | $G | $H...                           

   ::[MENU]
   ::[HELP] : 1 - Show Help
   ::[EXIT] : 0 - Back To Previous Menu

   ::[?] : _|
*/

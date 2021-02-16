
#pragma once

//https://espterm.github.io/docs/VT100%20escape%20codes.html //

#define ESC (27)
#define CH_C (67)
#define CH_D (68)
#define DEL (127)
#define BS  (8)

#include "avrtypes.h"
#include "strings.h"
#include <stdlib.h>
namespace term {

	char read_char() {
		while ( !(UCSR0A & (1 << RXC0)) );
		return UDR0;
	}

    void write_char(const char chr) {
		while ( !( UCSR0A & (1<<UDRE0)) );
		UDR0 = chr;
    }
	

	

	
	
	//int8_t read_int8_t(){
		//char buffer[4] = {0};
		//uint8_t index = 0;
		//char input = '\0';
		//do {
			//input = read_char();	
			//
			//if(index < 4){
				//if(input == '-' && index == 0) {
					//buffer[index++] = input;
					//term::write_char(input);
				//}
				//else if(str::is_digit(input)) {
					//buffer[index++] = input;
					//term::write_char(input);
				//}	
			//}
								//
		//} while (input != '\r');		
		//
		//term::write_string("\r\n");
		//
		//int result = atoi(buffer);
		//if(result > 127 || result < -127){
			//return 0;
		//} else {
			//return (int8_t)result;
		//}
	//}
	
	
	
	

    void write_string(const char* src) {
        for (; *src != '\0'; src++)
        {
            write_char(*src);
        }
    }
	
	void write_line(const char* src) {
		for (; *src != '\0'; src++)
		{
			write_char(*src);
		}
		write_string("\r\n");
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
    void cursor_left(uint8_t lines) {
        char str[4]{ 0 };
        utoa(lines, &str[0], 10);

        write_string("\x1B[");
        write_string(str);
        write_char('D');
    }



    //move cursor right 1 column
    void cursor_right() {
        write_string("\x1B[1C");
    }

    //move cursor right N columns
    void cursor_right(uint8_t lines) {
        char str[4]{ 0 };
        utoa(lines, &str[0], 10);
  
        write_string("\x1B[");
        write_string(str);
        write_char('C');
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
	
	
		//reads a maximum of N characters including '\0'
		template<avr_size_t buffer_size>
		void read_line(str::StringBuffer<buffer_size>& dest){
			
			char input = '\0';
			int16_t length = dest.length();
			int16_t index = length;
			
			term::write_string(dest);
			
			
			do {
				
				input = read_char();
				
				if((str::is_graph(input) || str::is_blank(input)) && length < buffer_size){
					dest.insert(input, index++);
					term::write_char(input);
					length++;
					term::save_cursor();	
					term::write_string(&dest[index]);
					term::restore_cursor();
					
				}else if(input == DEL){
				if(index < length && length > 0){
						dest.remove_at(index, 1);
						length--;
						
						term::save_cursor();
						term::write_string(&dest[index]);
						term::write_char(' ');
						term::restore_cursor();
					}
				}  else if(input == ESC){
					input = read_char();
					if(input == '['){
						input = read_char();
						//right
						if(input == CH_C){
							if(index < length){
								index++;
								term::cursor_right();
							}
							//left
							} else if(input == CH_D){
							if(index > 0){
								index--;
								term::cursor_left();
							}
						}
					}
				}
				
			} while (input != '\r');
			
			term::write_string("\r\n");
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

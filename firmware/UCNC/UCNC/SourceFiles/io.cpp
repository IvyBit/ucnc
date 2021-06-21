/*
 * io.cpp
 *
 * Created: 06.06.2021 00:01:02
 *  Author: alrha
 */

#include "definitions.h"
#include "io.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>



#define DISP_SRCLR	(PINC0)
#define DISP_SRCLK	(PINC1)
#define DISP_RCLK	(PINC2)
#define DISP_OE		(PINC3)
#define DISP_SER	(PINC4)

#define BTN_AUX		(PINC5)
#define BTN_RESET	(PINC6)

#define	OUT_SER		(PIND2)
#define	OUT_RCLK	(PIND3)
#define	OUT_SRCLK	(PIND4)
#define	OUT_SRCLR	(PIND5)

#define	OUT_TONE	(PIND6)
#define	OUT_LED		(PIND7)


#define SSEL_A		(PINB0)
#define SSEL_B		(PINB1)
#define SSEL_C		(PINB2)
#define ASIN		(PINE2)




//inverted select signal
#define ASA	((uint8_t)(~0x03 & 0x07)) //011 -> 100 -> 0x04
#define ASB	((uint8_t)(~0x00 & 0x07)) //000 -> 111 -> 0x07
#define ASC	((uint8_t)(~0x05 & 0x07)) //101 -> 010 -> 0x02
#define ASD	((uint8_t)(~0x01 & 0x07)) //001 -> 110 -> 0x06
#define ASE	((uint8_t)(~0x07 & 0x07)) //111 -> 000 -> 0x00
#define ASF	((uint8_t)(~0x02 & 0x07)) //010 -> 101 -> 0x05
#define ASG	((uint8_t)(~0x06 & 0x07)) //110 -> 001 -> 0x01
#define ASH	((uint8_t)(~0x04 & 0x07)) //100 -> 011 -> 0x03




namespace io {

	const uint8_t ASI_MAP[8] PROGMEM= { ASA, ASB, ASC, ASD, ASE, ASF, ASG, ASH };
	uint32_t boot_cfg __attribute__ ((section (".noinit")));

	volatile uint8_t	adc_values[16]	= {0};
	volatile uint8_t*	adc_value_write	= nullptr;
	volatile uint8_t*	adc_value_read	= nullptr;
	volatile bool	    adc_busy		= false;

	void setup(){
		//make PINC0-4 outputs
		DDRC |= ((1 << DISP_SRCLR) | (1 << DISP_SRCLK) | (1 << DISP_RCLK) | (1 << DISP_OE) | (1 << DISP_SER));
		//make PC5 an input
		DDRC &= ~(1 << BTN_AUX);
		//set PORTC to low and disable pullup for PC5
		PORTC = 0x00;
		//disable input clear
		PORTC |= (1 << DISP_SRCLR);
		//enable input indicators
		PORTC &= ~(1 << DISP_OE);

		//make PIND2-7 outputs
		DDRD |= ((1 << OUT_SER) | (1 << OUT_RCLK) | (1 << OUT_SRCLK) | (1 << OUT_SRCLR)| (1 << OUT_TONE) | (1 << OUT_LED) );
		//disable output clear
		PORTD |= (1 << OUT_SRCLR);
		//enable outputs
		//PORTD &= ~(1 << OUT_OE);

		//make PB0-PB2 outputs
		DDRB |= (1 << SSEL_A) |(1 << SSEL_B) |(1 << SSEL_C);

		DDRE = 0x00;
		PORTE |= (1 << PINE0) | (1 << PINE1);

		//select 5v reference input ADC6 and left align result
		ADMUX |= (1 << REFS0) | (1 << MUX2) | (1 << MUX1) | (1 << ADLAR);
		//enable ADC, enable interrupt, set prescaler to 16
		ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADPS2)| (0 << ADPS1)| (0 << ADPS0);

		//select input A as adc source
		PORTB = (uint8_t)((PORTB & (uint8_t)~0x07) | pgm_read_byte(&ASI_MAP[0]));


		//TIMER 0 8BIT BUZZER TONE
		OCR0A = 70;
		TCCR0A |= (1 << COM0A0) | (1 << WGM01); //TOGGLE OC0A/PD6
		//TCCR0B |= (1 << CS01) | (1 << CS00);	//64 DIV PRESCLARER


		//TIMER1 16BIT BUZZER DELAY
		TCCR1A = 0x00;
		TCCR1B = (1 << WGM12); //CTC MODE NO CLOCK SOURCE
		TIMSK1 |= (1 << OCIE1A); //ENABLE TIMER1_COMPA INTERRUPT

		//TIMER3 16BIT LED
		TCCR3A = 0x00;
		TCCR3B = (1 << WGM32); //CTC MODE NO CLOCK SOURCE
		TIMSK3 |= (1 << OCIE3A); //ENABLE TIMER3_COMPA INTERRUPT



		//enable interrupts
		sei();
	}


	void set_indicators(uint16_t indicators){

		for (uint8_t i = 0; i < 16; i++)
		{
			//shift value into register
			if((indicators >> i) & 0x01){
				PORTC |= (1 << DISP_SER);
				}else{
				PORTC &= ~(1 << DISP_SER);
			}
			//clock register
			PORTC |= (1 << DISP_SRCLK);
			PORTC &= ~(1 << DISP_SRCLK);
		}

		//clock output register
		PORTC |= (1 << DISP_RCLK);
		PORTC &= ~(1 << DISP_RCLK);
	}

	void clear_indicators(){
		//clear register
		PORTC &= ~(1 << DISP_SRCLR);
		PORTC |= (1 << DISP_SRCLR);

		//clock output register
		PORTC |= (1 << DISP_RCLK);
		PORTC &= ~(1 << DISP_RCLK);
	}

	void set_outputs(uint8_t outputs){
		for (uint8_t i = 0; i < 8; i++)
		{
			//shift value into register
			if(((outputs >> i) & 0x01)){
				PORTD |= (1 << OUT_SER);
			}else{
				PORTD &= ~(1 << OUT_SER);
			}
			//clock register
			PORTD |= (1 << OUT_SRCLK);
			PORTD &= ~(1 << OUT_SRCLK);
		}

		//clock output register
		PORTD |= (1 << OUT_RCLK);
		PORTD &= ~(1 << OUT_RCLK);
	}

	void clear_outputs(){
		//clear register
		PORTD &= ~(1 << OUT_SRCLR);
		PORTD |= (1 << OUT_SRCLR);

		//clock output register
		PORTD |= (1 << OUT_RCLK);
		PORTD &= ~(1 << OUT_RCLK);
	}



	/*BUTTON*/
	ISR(PCINT1_vect){
		cli();
		set_boot_cfg_flag();
		reset_mcu();
	}

	bool get_aux_pushed(){
		return (PINC & (1 << BTN_AUX)) != (1 << BTN_AUX);
	}

	void enable_btn_reset_async(){
		PCMSK1 |= (1 << PCINT13);
		PCICR |= (1 << PCIE1);
	}

	void disable_btn_reset_async(){
		PCICR &= ~(1 << PCIE1);
		PCMSK1 &= ~(1 << PCINT13);
	}
	/*BUTTON*/


	/*ADC*/
	ISR(ADC_vect){

		static uint8_t		adc_index = 0;

		uint8_t current_index = adc_index;
		adc_index++;

		if(adc_index > 7) adc_index = 0;

		PORTB = (uint8_t)((PORTB & (uint8_t)~0x07) | pgm_read_byte(&ASI_MAP[adc_index]));

		uint8_t av = 0;

		//av |= ADCL;
		av = ADCH;//(uint16_t)(ADCH << 8);

		adc_value_write[current_index] = (uint8_t)(av * (100.0 / ADC_LIMIT));
		if(adc_value_write[current_index] > 100){
			adc_value_write[current_index] = 100;
		}

		if(adc_index < 7){
			//start conversion
			ADCSRA |= (1 << ADSC);
		}else{
			adc_busy = false;
		}
	}

	bool adc_is_busy(){
		return adc_busy;
	}

	void adc_busy_wait(){
		while(adc_busy){}
	}

	void adc_start_conversion(){

		adc_busy = true;

		//toggle adc target buffer
		if(adc_value_read != adc_values){
		   adc_value_read = &adc_values[0];
		   adc_value_write = &adc_values[8];
		}
		else{
			adc_value_read = &adc_values[8];
			adc_value_write = &adc_values[0];
		}

		//start conversion
		ADCSRA |= (1 << ADSC);
	}

	void adc_update(uint8_t (&buffer)[8]){
		for (uint8_t i = 0; i < 8; i++){
			buffer[i] = adc_value_read[i];
		}
	}
	/*ADC*/





	/*BUZZER*/
	ISR(TIMER1_COMPA_vect){
		//STOP TIMER 0 PRESCLAER
		TCCR1B = (1 << WGM12);
		//BUZZER OFF
		TCCR0B = 0x00;	//0 DIV PRESCLARER
	}

	void buzzer_on_for(uint16_t milliseconds){
		if(milliseconds == 0xFFFF) milliseconds--;
		//BUZZER ON
		TCCR0B |= (1 << CS01) | (1 << CS00);	//64 DIV PRESCLARER
		//DUMB DELAY LOOP
		for (uint16_t i = 0; i < milliseconds; i++){
			_delay_ms(1);
		}
		//BUZZER OFF
		TCCR0B = 0x00;	//0 DIV PRESCLARER
	}

	void buzzer_on_for_async(uint16_t milliseconds){
		TCNT1 = 0x00;
		//((A3/B3) * C3 ) / 1000-1
		OCR1A = (((F_CPU / 1024.0) * milliseconds) / 1000.0) - 1;
		//OCR1A = (uint16_t)(milliseconds / (1000.0 / (F_CPU / 1024.0))) - 1;
		//START TIMER 1024 PRESCLAER
		TCCR1B |= ((1 <<CS12) | (1 << CS10));
		//BUZZER ON
		TCNT0 = 0x00;
		TCCR0B |= (1 << CS01) | (1 << CS00);	//64 DIV PRESCLARER
	}

	void buzzer_on(){
		TCNT0 = 0x00;
		TCCR0B |= (1 << CS01) | (1 << CS00);	//64 DIV PRESCLARER
	}

	void buzzer_on_off(){
		TCCR0B = 0x00;	//0 DIV PRESCLARER
	}
	/*BUZZER*/




	/*LED*/
	ISR(TIMER3_COMPA_vect){
		//STOP TIMER 0 PRESCLAER
		TCCR3B = (1 << WGM32);
		//BUZZER OFF
		PORTD &= ~(1 << OUT_LED);
	}

	void led_on_for(uint16_t milliseconds){
		if(milliseconds == 0xFFFF) milliseconds--;
		//BUZZER ON
		PORTD |= (1 << OUT_LED);
		//DUMB DELAY LOOP
		for (uint16_t i = 0; i < milliseconds; i++){
			_delay_ms(1);
		}
		//BUZZER OFF
		PORTD &= ~(1 << OUT_LED);
	}

	void led_on_for_async(uint16_t milliseconds){
		TCNT3 = 0x00;
		OCR3A = (uint16_t)(milliseconds / (1000.0 / (F_CPU / 1024.0)));
		//START TIMER 1024 PRESCLAER
		TCCR3B |= ((1 <<CS32) | (1 << CS30));
		//BUZZER ON
		PORTD |= (1 << OUT_LED);
	}

	void led_on(){
		PORTD |= (1 << OUT_LED);
	}

	void led_off(){
		PORTD &= ~(1 << OUT_LED);
	}
	/*LED*/



	void disable_watchdog() {
		MCUSR &= ~(1 << WDRF);
		wdt_disable();
	}

	void reset_mcu(){
		//disable interrupts
		cli();

		wdt_enable(WDTO_2S);

		while(1){
			set_indicators(0x00FF);
			_delay_ms(50);
			set_indicators(0xFF00);
			_delay_ms(50);
		}
	}





	void set_boot_cfg_flag(){
		boot_cfg = 0xDEADBEEF;
	}

	void clear_boot_cfg_flag(){
		boot_cfg = 0x00000000;
	}

	bool boot_cfg_flag_set(){
		return boot_cfg == 0xDEADBEEF;
	}
};

#if defined(NDEBUG) || defined(DEBUGCFG)

#include "appcfg.h"
#include "io.h"
#include "stringbuffer.h"
#include "serial.h"
#include "protocol.h"
#include "config.h"
#include <util\delay.h>


namespace apps{

	appcfg::appcfg(){

	}

	bool appcfg::on_entry(){
		prot::write_welcome_cfg();
		io::set_indicators(0x00FF);
		io::buzzer_on_for(200);
		_delay_ms(100);
		io::buzzer_on_for(200);


		if(config::check_config()){
			return true;
		} else {

			config::reset_config();
			prot::write_status(CMDE_CHECKSUM);
			return false;

		}


	}




	bool appcfg::on_run(){
		str::stringbuffer<INPUT_BUFFER_SIZE> input_buffer;
		uint16_t result = serial::read_line(input_buffer);
		switch(result){
			case SER_OVERF:
				prot::write_status(CMDE_LENGTH);
				//error:11” : _(“Max characters per line exceeded. Line was not processed and executed.”)
				return true;

			case SER_RESET:
				return false;

			default:{
				prot::handle_command(input_buffer);
				return true;
			}
		}
	}


	bool appcfg::on_exit(){
		return true;
	}

	appcfg::~appcfg(){

	}

};


#endif
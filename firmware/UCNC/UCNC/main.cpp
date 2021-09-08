/*
	SHIT TO DO
#	1. Somehow get into config mode while in run mode...
#	2. Fix protocol.h $A isn't a fucking integer...
#	3. Make buzzer go bzzz bzzz bzzz
#	4. Make led go blinky blinky
	5. Write a novel on how to use the bloody thing...
	6. Profit... well fuck
*/

#include "core.h"
#include <util\delay.h>



void core::system_entry(){

#if defined(NDEBUG) || (defined(DEBUGCFG) && defined(DEBUGPLD))

		if(io::get_aux_pushed() || io::boot_cfg_flag_set()){
			io::clear_boot_cfg_flag();
			core::app_context<apps::appcfg>::execute();
		}
		else {
			core::app_context<apps::apppld>::execute();
		}

#elif defined(DEBUGPLD)

		core::app_context<apps::apppld>::execute();

#elif defined(DEBUGCFG)

		core::app_context<apps::appcfg>::execute();

#endif


}


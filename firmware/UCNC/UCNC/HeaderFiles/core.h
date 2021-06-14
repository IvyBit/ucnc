/*
 * core.h
 *
 * Created: 05.06.2021 17:19:55
 *  Author: alrha
 */

#include "definitions.h"

#ifndef CORE_H_
#define CORE_H_


#include <stdint-gcc.h>
#include <avr/io.h>

#if defined(NDEBUG)
	#include "appcfg.h"
	#include "apppld.h"

#elif defined(DEBUGCFG) && defined(DEBUGPLD)
	#include "appcfg.h"
	#include "apppld.h"

#elif defined(DEBUGCFG)
	#include "appcfg.h"

#elif defined(DEBUGPLD)
	#include "apppld.h"

#else
	#error "UNDEFINED CONFIGURATION"
#endif


#include "io.h"
#include "serial.h"
#include "context.h"

namespace core {



	void system_entry();

};
#endif /* CORE_H_ */
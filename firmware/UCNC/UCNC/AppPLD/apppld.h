/*
 * apppld.h
 *
 * Created: 05.06.2021 17:38:55
 *  Author: alrha
 */

#if defined(NDEBUG) || defined(DEBUGPLD)
#include "array.h"
#include "opcodes.h"
#include "translation.h"
#include "config.h"
#ifndef APPPLD_H_
#define APPPLD_H_



namespace apps{
	class apppld{
		public:
		apppld( const apppld& ) = delete; // non construction-copyable
		apppld& operator=( const apppld& ) = delete; // non copyable
		apppld();
		~apppld();
		void halt() __attribute__((noreturn));
		bool on_entry();
		bool on_run();
		bool on_exit();
		private:
		config::system_config			sys_config;
		uint8_t							truth_table[256];
		translation::translation_config io_translators[8];

	};
}



#endif /* APPPLD_H_ */
#endif
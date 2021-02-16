#pragma once
#include "avrtypes.h"

namespace err {

	struct Error
	{
		avr_size_t source;
		avr_size_t data;
	};
	#ifndef AVR
	__declspec(noreturn) extern void on_error(const Error &ec);
	#else
	__attribute__((noreturn)) extern void on_error(const Error &ec);	
	#endif
};
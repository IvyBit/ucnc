#pragma once
#include "avrtypes.h"

namespace err {

	struct Error
	{
		avr_size_t source;
		avr_size_t data;
	};

	__attribute__((noreturn)) extern void on_error(const Error &ec);
	
};
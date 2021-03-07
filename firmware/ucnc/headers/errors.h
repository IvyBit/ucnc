#pragma once
#include "avrtypes.h"

namespace err {

	__attribute__((noreturn)) extern void on_error(uint8_t ec);
	
};
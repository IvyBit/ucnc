#include "core.h"
#include "protocol.h"

int main(){

	io::setup();
	io::disable_watchdog();

	serial::setup();

	core::system_entry();

	io::reset_mcu();
}
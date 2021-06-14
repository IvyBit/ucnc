/*
 * protocol.h
 *
 * Created: 06.06.2021 17:02:57
 *  Author: alrha
 */

/*

Serial configuration: 115200 8-N-1 (8-bits, no parity, 1-stop bit)

ctrl-x	- Reset

$RST=$	- Restore input and output default settings and reboot
$RST=I	- Restore input default settings and reboot
$RST=O	- Restore output default settings and reboot

$$ 	- View settings
$S	- View status

$100	- Serial Output 		(1/0)
$101	- Buzzer 				(1/0)

$210	- Input A threshold 	(0-100)
$211	- Input A hysteresis 	(0-100)
$212	- Input A inverted 		(1/0)

$220	- Input B threshold 	(0-100)
$221	- Input B hysteresis 	(0-100)
$222	- Input B inverted 		(1/0)

$230	- Input C threshold 	(0-100)
$231	- Input C hysteresis 	(0-100)
$232	- Input C inverted 		(1/0)

$240	- Input D threshold 	(0-100)
$241	- Input D hysteresis 	(0-100)
$242	- Input D inverted 		(1/0)

$250	- Input E threshold 	(0-100)
$251	- Input E hysteresis 	(0-100)
$252	- Input E inverted 		(1/0)

$260	- Input F threshold 	(0-100)
$261	- Input F hysteresis 	(0-100)
$262	- Input F inverted 		(1/0)

$270	- Input G threshold 	(0-100)
$271	- Input G hysteresis 	(0-100)
$272	- Input G inverted 		(1/0)

$280	- Input H threshold 	(0-100)
$281	- Input H hysteresis 	(0-100)
$282	- Input H inverted 		(1/0)


$310	- Output A active 		(1/0)
$211	- Output A expression 	("$A | $B ...")
$212	- Output A override 	(1/0)

$320	- Output B active 		(1/0)
$321	- Output B expression 	("$A | $B ...")
$322	- Output B override 	(1/0)

$330	- Output C active 		(1/0)
$331	- Output C expression 	("$A | $B ...")
$332	- Output C override 	(1/0)

$340	- Output D active 		(1/0)
$341	- Output D expression 	("$A | $B ...")
$342	- Output D override 	(1/0)

$350	- Output E active 		(1/0)
$351	- Output E expression 	("$A | $B ...")
$352	- Output E override 	(1/0)

$360	- Output F active 		(1/0)
$361	- Output F expression 	("$A | $B ...")
$362	- Output F override 	(1/0)

$370	- Output G active 		(1/0)
$371	- Output G expression 	("$A | $B ...")
$372	- Output G override 	(1/0)

$380	- Output H active 		(1/0)
$381	- Output H expression 	("$A | $B ...")
$382	- Output H override 	(1/0)


"\t- Input "
"\t- Output "
" threshold\t\t("
" hysteresis\t\t("
" inverted\t\t("
" active\t\t("
" expression\t\t("
" override\t\t("

*/


#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "definitions.h"
#include "stringbuffer.h"
#include <avr/pgmspace.h>

#define CMDE_OK 0
#define CMDE_INVALID 1
#define CMDE_TARGET 2
#define CMDE_OPTION 3
#define CMDE_VALUE 4
#define CMDE_LENGTH 5
#define CMDE_CHECKSUM 6
#define CMDE_EXPRESSION 7

namespace prot{


	void write_welcome_cfg();
	void write_welcome_pld();

	void write_status(uint16_t error);


	void handle_command(str::stringbuffer<INPUT_BUFFER_SIZE> &buffer);


};
#endif /* PROTOCOL_H_ */

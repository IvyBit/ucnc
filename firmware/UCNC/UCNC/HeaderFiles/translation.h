/*
 * mapping.h
 *
 * Created: 11.06.2021 12:50:12
 *  Author: alrha
 */

 #include "opcodes.h"
#ifndef TRANSLATION_H_
#define TRANSLATION_H_

namespace translation{

	struct translation_config{
		uint8_t	threshold	= 0;
		uint8_t	hysteresis  = 0;
		bool	inverted	= false;
		bool	trigger     = false;
		bool	last_state  = false;
	};

	bool translate(translation_config &tc, uint8_t input_level);

};
#endif /* TRANSLATION_H_ */
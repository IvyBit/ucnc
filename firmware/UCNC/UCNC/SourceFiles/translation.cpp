/*
 * translation.cpp
 *
 * Created: 11.06.2021 12:56:17
 *  Author: alrha
 */
 #include "translation.h"

 namespace translation{

	bool translate(translation_config &tc, uint8_t input_level){
		if(!tc.inverted){
			uint8_t limit = tc.hysteresis > tc.threshold ? 0 : tc.threshold - tc.hysteresis;

			if(!tc.trigger && input_level >= tc.threshold){
				tc.trigger = true;
				tc.last_state = true;
			} else if(tc.trigger && input_level <= limit){
				tc.trigger = false;
				tc.last_state = false;
			}
		} else {
			uint8_t limit = tc.hysteresis + tc.threshold > 100 ? 100 : tc.hysteresis + tc.threshold;

			if(!tc.trigger && input_level <= tc.threshold){
				tc.trigger = true;
				tc.last_state = true;
			} else if(tc.trigger && input_level >= limit){
				tc.trigger = false;
				tc.last_state = false;
			}
		}

		return tc.last_state;
	}

 };
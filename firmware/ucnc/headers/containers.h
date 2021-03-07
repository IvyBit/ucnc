#pragma once

#include <stdlib.h>

#define ERR_INVALID_INDEX 3


#include "errors.h"
#include "avrtypes.h"


namespace ctr{

#define NONCOPY(T) T(const T&) = delete; \
				   T& operator=(const T&) = delete;


	template<typename T, avr_size_t size>
	class Array {
	public:
		NONCOPY(Array)
		Array() = default;
		~Array() {}

		const avr_size_t length() {
			return size;
		}

		T& operator[](avr_size_t index) {
			if (index < size) {
				return _data[index];
			}
			else {
				err::on_error(ERR_INVALID_INDEX);
			}			
		}

		T* ptr() {
			return &_data[0];
		}
	private:
		T _data[size];
	};	

};


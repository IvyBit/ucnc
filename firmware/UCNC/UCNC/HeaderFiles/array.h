/*
 * array.h
 *
 * Created: 10.06.2021 00:40:38
 *  Author: alrha
 */

 #include <stdint.h>

#ifndef ARRAY_H_
#define ARRAY_H_

namespace containers{

	template<typename T, uint16_t size>
	class array {
		public:
		array<T, size>( const array<T, size>& ) = delete; // non construction-copyable
		array<T, size>& operator=( const array<T, size>& ) = delete; // non copyable
		array() = default;
		~array() {}

		const uint16_t length() {
			return size;
		}

		T& operator[](uint16_t index) {
			return _data[index];
		}

		T* ptr() {
			return &_data[0];
		}
		private:
		T _data[size];
	};
};

#endif /* ARRAY_H_ */
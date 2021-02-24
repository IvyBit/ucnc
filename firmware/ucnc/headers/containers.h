#pragma once

#include <stdlib.h>

#ifndef AVR
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

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
				err::on_error({0, 0});
			}			
		}

		T* ptr() {
			return &_data[0];
		}
	private:
		T _data[size];
	};

	template<typename T, avr_size_t size>
	class FixedStack {
	public:
		NONCOPY(FixedStack)
		FixedStack() : _stack_index(0), _data() {}
		~FixedStack() {}
		
		void clear(){
			_stack_index = 0;
		}
		
		bool empty() {
			return _stack_index == 0;
		}

		avr_size_t max_length() {
			return size;
		}

		avr_size_t length() {
			return _stack_index;
		}

		T& operator[](avr_size_t index) {
			if (index < size) {
				return _data[index];
			} 
			else {
				err::on_error({ 0, 0 });
			}
		}

		void push(const T& element) {
			if (_stack_index < size) {
				_data[_stack_index++] = element;
			}
			else {
				err::on_error({ 0, 0 });
			}
		}

		void pop() {
			if (_stack_index > 0) {
				_data[--_stack_index] = T{};
			}
		}

		T& top() {
			if (_stack_index > 0 && _stack_index < size) {
				return _data[_stack_index - 1];
			}
			else {
				err::on_error({ 0, 0 });
			}
		}

	private:
		avr_size_t _stack_index;
		Array<T, size> _data;
	};


};


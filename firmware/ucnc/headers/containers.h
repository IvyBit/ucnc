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


//
	////!WARNING! MAKE SURE THIS MONSTER IS AS SHORT LIVED AS POSSIBLE!
	////Seriously... this could cause a stack heap collision! NEVER EVER MAKE THIS THING GLOBAL!
	//template<typename T, avr_size_t chunk_size = 5>
	//class ArrayList {
	//public:
		//NONCOPY(ArrayList)
			//ArrayList() : _elements(0), _chunks(1), _data() {
			//_data = (Array<T, chunk_size>**)malloc(sizeof(Array<T, chunk_size>**) * _chunks);
			//if (_data) {			
				//for (avr_size_t index = 0; index < _chunks; index++)
				//{
					//_data[index] = (Array<T, chunk_size>*)malloc(sizeof(Array<T, chunk_size>));
//
					//T* array_data = _data[index]->ptr();
					//for (avr_size_t array_index = 0; array_index < chunk_size; array_index++)
					//{
						//array_data[array_index] = T{};
					//}
				//}
					//_elements = _chunks * chunk_size;
				//}
			//else {
				//err::on_error({ 0, 0 });
			//}
		//}
//
		//~ArrayList() {
			//for (avr_size_t index = 0; index < _chunks; index++)
			//{
				//if (_data[index] != nullptr) {
					//free(_data[index]);
				//}
			//}
			//free(_data);
		//}
//
		//void expand() {
			//Array<T, chunk_size>** new_data_ptr = (Array<T, chunk_size>**)malloc(sizeof(Array<T, chunk_size>**) * (_chunks + 1));
//
			////copy old pointers
			//for (avr_size_t index = 0; index < _chunks; index++)
			//{
				//new_data_ptr[index] = _data[index];
			//}
//
			////allocate memory for new array
			//new_data_ptr[_chunks] = (Array<T, chunk_size>*)malloc(sizeof(Array<T, chunk_size>));
//
			//if (new_data_ptr[_chunks]) {			
				////initialize new array
				//T* array_data = new_data_ptr[_chunks]->ptr();
				//for (avr_size_t array_index = 0; array_index < chunk_size; array_index++)
				//{
					//array_data[array_index] = T{};
				//}
//
				////release old pointer array
				//free(_data);
//
				//_data = new_data_ptr;
				//_chunks++;
				//_elements += chunk_size;
			//}
			//else {
				//err::on_error({ 0, 0 });
			//}
		//}
//
		//T& operator[](avr_size_t index) {
			//if (index < _elements) {
				//if (index < chunk_size) {
					//return (*_data[0])[index];
				//}
				//else {
					//return (*_data[index / chunk_size])[index % chunk_size];
				//}
			//}
			//else {
				//err::on_error({ 0, 0 });
			//}
		//}
//
		//Array<T, chunk_size>* chunk_ptr(avr_size_t index) {
			//if (index > _chunks) {
				//return _data[index];
			//}
			//else {
				//err::on_error({ 0, 0 });
			//}
		//}
//
		//avr_size_t length() {
			//return _elements;
		//}
//
		//avr_size_t chunks() {
			//return _chunks;
		//}
//
	//private:
		//avr_size_t			   _elements;
		//avr_size_t			   _chunks;
		////pointer to a pointer to an array...
		//Array<T, chunk_size>** _data;
	//};


	//template<typename T, avr_size_t chunk_size>
	//class Stack {
	//public:
		//NONCOPY(Stack)
		//Stack() : _stack_index(0), _data() {}
		//~Stack() {}
//
		//bool empty() {
			//return _stack_index == 0;
		//}
//
		//avr_size_t max_length() {
			//return _data.length();
		//}
//
		//avr_size_t length() {
			//return _stack_index;
		//}
//
		//T& operator[](avr_size_t index) {
			//if (index < _stack_index) {
				//return _data[index];
			//}
			//else {
				//err::on_error({ 0, 0 });
			//}
		//}
//
		//void push(const T& element) {
			//if (_stack_index < _data.length()) {
				//_data[_stack_index++] = element;
			//}
			//else {
				//_data.expand();
				//_data[_stack_index++] = element;
			//}
		//}
//
		//void pop() {
			//if (_stack_index > 0) {
				//_data[--_stack_index] = T{};
			//}
		//}
//
		//T& top() {
			//if (_stack_index > 0) {
				//return _data[_stack_index - 1];
			//}
			//else {
				//err::on_error({ 0, 0 });
			//}
		//}
//
	//private:
		//avr_size_t _stack_index;
		//ArrayList<T, chunk_size> _data;
	//};



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


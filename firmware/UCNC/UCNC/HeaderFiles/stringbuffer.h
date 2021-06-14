#include "definitions.h"
#include "stringsfunctions.h"

#ifndef STRINGBUFFER_H_
#define STRINGBUFFER_H_

namespace str{



	template<uint16_t size>
	class stringbuffer final {
		public:
		stringbuffer( const stringbuffer& ) = delete; // non construction-copyable
		stringbuffer& operator=( const stringbuffer& ) = delete; // non copyable
		stringbuffer() :_cursor(0), _buffer{ 0 }{}


		void clear() {
			_cursor = 0;
			_buffer[0] = STR_END;
		}

		bool append(char chr){
			if (_cursor < size) {
				_buffer[_cursor++] = chr;
				_buffer[_cursor] = STR_END;
				return true;
			}
			return false;
		}

		bool append(const char* src) {
			if (_cursor < size) {
				for (const char* p = src; *p != STR_END && _cursor < size; p++) {
					_buffer[_cursor++] = *p;
				}
				_buffer[_cursor] = STR_END;
				return true;
			}
			return false;
		}

		uint16_t length() {
			return _cursor;
		}



		void remove_at(uint16_t index, uint16_t length){

			if(index + length <= size){

				for (;index < size; index++)
				{
					if(index + length < _cursor){
						_buffer[index] = _buffer[index + length];
						}else{
						_buffer[index] = STR_END;
					}
				}
				this->_cursor -= length;
			}
		}



		bool insert(const char chr, uint16_t insert_index) {

			if (insert_index <= _cursor) {

				if (insert_index + 1 <= size) {

					uint16_t right_index = _cursor > size - 1 ? size - 1 : _cursor;

					for (uint16_t index = right_index; index >= insert_index + 1; index--)
					{
						_buffer[index] = _buffer[index - 1];
					}

					_buffer[insert_index] = chr;

					_cursor = right_index + 1;
					_buffer[_cursor] = STR_END;
					return true;
				}
			}
			return false;
		}

		bool insert(const char* src, uint16_t insert_index) {
			//	A B C D E F G H	I _ _ _|
			//	A B X X X X C D E F G I|
			if (insert_index < size) {

				uint16_t src_length = str::length(src);

				if (insert_index + src_length <= size) {

					uint16_t right_index = _cursor - 1 + src_length > size - 1 ? size - 1 : _cursor - 1 + src_length;

					for (uint16_t index = right_index; index >= insert_index + src_length; index--)
					{
						_buffer[index] = _buffer[index - src_length];
					}

					for (uint16_t index = insert_index; index < insert_index + src_length; index++)
					{
						_buffer[index] = *src++;
					}

					_cursor = right_index + 1;
					_buffer[_cursor] = STR_END;
					return true;
				}
			}
			return false;
		}

		const char* ptr() {
			return _buffer;
		}

		operator const char* () {
			return _buffer;
		}

		private:
		uint16_t _cursor;
		//requires space for string terminator
		char _buffer[size + 1];
	};




};
#endif /* STRING_H_ */
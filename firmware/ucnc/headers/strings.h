#pragma once
#include "avrtypes.h"


namespace str {
    const int16_t NO_MATCH = -1;
    const char    END = '\0';


	
















    char* find_end(char* src) {
        while (*src != str::END) {
            src++;
        }
        return src;
    }

    const char* find_end(const char* src) {
        while (*src != str::END) {
            src++;
        }
        return src;
    }


    //returns the previous character from the string or NULL if the pointer already points to NULL
    char lead(const char* src) {
        if (*src == END) return str::END;
        return *(src - 1);
    }

    //returns the next character from the string or NULL if the pointer already points to NULL
    char lag(const char* src) {
        if (*src == END) return str::END;
        return *(src + 1);
    }

    //returns the index of the first match or NO_MATCH (-1) starting at start_index
    int16_t index_of(const char* src, int16_t start_index, char match) {
        for (; src[start_index] != END; start_index++) {
            if (src[start_index] == match) return start_index;
        }
        return NO_MATCH;
    }

    //returns the index of the first match or NO_MATCH (-1)
    int16_t index_of(const char* src, char match) {
        return index_of(src, 0, match);
    }

    //returns the index of the first match or NO_MATCH (-1) starting at start_index
    int16_t index_of(const char* src, int16_t start_index, const char* match) {
        for (; src[start_index] != END; start_index++) {
            for (const char* m = match; *m != END; m++) {
                if (*m == src[start_index]) return start_index;
            }
        }
        return NO_MATCH;
    }

    //returns the index of the first match or NO_MATCH (-1)
    int16_t index_of(const char* src, const char* match) {
        return index_of(src, 0, match);
    }


    //returns true if src contains chr
    bool contains(const char* src, const char chr) {
        return str::index_of(src, chr) != str::NO_MATCH;
    }

    //returns true if match occurs in src
    bool contains(const char* src, const char* match) {
        return str::index_of(src, match) != str::NO_MATCH;
    }
       
    //Checks whether chr is either a decimal digit or an uppercase or lowercase letter. 
    bool is_alphanum(const char chr) {
        if ((chr >= '0' && chr <= '9') ||
            (chr >= 'a' && chr <= 'z') ||
            (chr >= 'A' && chr <= 'Z')) {
            return true;
        }
        else {
            return false;
        }
    }

    //Checks whether chr is an alphabetic letter.
    bool is_alpha(const char chr) {
        if ((chr >= 'a' && chr <= 'z') ||
            (chr >= 'A' && chr <= 'Z')) {
            return true;
        }
        else {
            return false;
        }
    }

    //Checks whether chr is a blank character.
    bool is_blank(const char chr) {
        if (chr == ' ' || chr == '\t') {
            return true;
        }
        else {
            return false;
        }
    }

    //Checks whether chr is a decimal digit character.
    bool is_digit(const char chr) {
        if (chr >= '0' && chr <= '9') {
            return true;
        }
        else {
            return false;
        }
    }

    //Checks whether chr is a lowercase letter.
    bool is_lower(const char chr) {
        if ((chr >= 'a' && chr <= 'z')) {
            return true;
        }
        else {
            return false;
        }
    }

    //Checks if parameter chr is an uppercase alphabetic letter.
    bool is_upper(const char chr) {
        if ((chr >= 'A' && chr <= 'Z')) {
            return true;
        }
        else {
            return false;
        }
    }

    //considers codes 0x00 (NUL) and 0x1f (US), plus 0x7f (DEL).
    bool is_control(const char chr) {
        return (uint8_t)(chr) == (uint8_t)(0x7F) || ((uint8_t)(chr) > (uint8_t)(0x00) && (uint8_t)(chr) < (uint8_t)(0x20));
    }

    //Checks whether chr is a printable character.
    bool is_print(const char chr) {
        return (uint8_t)(chr) > (uint8_t)(0x1F) && (uint8_t)(chr) < (uint8_t)(0x7F);
    }

    //Checks whether chr is a character with graphical representation.
    bool is_graph(const char chr) {
        return chr != ' ' && is_print(chr);
    }

    //Checks whether chr is a punctuation character.
    bool is_punct(const char chr) {
        return is_graph(chr) && !is_alphanum(chr);
    }

    //Checks whether chr is a white-space character.
    bool is_space(const char chr) {
        return chr == ' ' || (chr > 0x08 && chr < 0x0E);
    }

    //Checks whether chr is a hexdecimal digit character.
    bool is_hex(const char chr) {
        return (chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F');
    }

    //Convert uppercase letter to lowercase
    char to_lower(char chr) {
        if (is_upper(chr)) {
            return (char)((uint8_t)(chr)+(uint8_t)(0x20));
        }
        else {
            return chr;
        }
    }

    //Convert uppercase string to lowercase
    void to_lower(char* src) {
        for (char* lc = src; *lc != END; lc++) {
            if (*lc >= 'a' && *lc <= 'A') *lc = (char)(*lc - 0x20);
        }
    }

    //Convert lowercase letter to uppercase
    char to_upper(char chr) {
        if (is_lower(chr)) {
            return (char)((uint8_t)(chr)-(uint8_t)(0x20));
        }
        else {
            return chr;
        }
    }

    //Convert lowercase string to uppercase
    void to_upper(char* src) {
        for (char* lc = src; *lc != END; lc++) {
            if (*lc >= 'A' && *lc <= 'Z') *lc = (char)(*lc + 0x20);
        }
    }

    avr_size_t length(const char* src) {        
        avr_size_t index = 0;
        for (; src[index] != '\0'; index++){}
        return index;
    }

    //skip whitespace and tab space
    void skip_blank(const char* &src) {
        while (is_blank(*src)) src++;
    }
	
	
	
	
	
	
	
	
	template<avr_size_t size>
	class StringBuffer final {
		public:
		NONCOPY(StringBuffer)
		StringBuffer() :_cursor(0), _buffer{ 0 }{}


		void clear() {
			_cursor = 0;
			_buffer[0] = '\0';
		}
		
		bool append(const char chr){
			if (_cursor < size) {
				_buffer[_cursor++] = chr;
				_buffer[_cursor] = '\0';
				return true;
			}
			return false;
		}

		bool append(const char* src) {
			if (_cursor < size) {
				for (const char* p = src; *p != '\0' && _cursor < size; p++) {
					_buffer[_cursor++] = *p;
				}
				_buffer[_cursor] = '\0';
				return true;
			}
			return false;
		}

		avr_size_t length() {
			return _cursor;
		}



		void remove_at(avr_size_t index, avr_size_t length){
			
			if(index + length < size){
				
				for (;index < size; index++)
				{
					if(index + length < _cursor){
						_buffer[index] = _buffer[index + length];
					}else{
						_buffer[index] = str::END;
					}
				}					
				this->_cursor -= length;						
			}
		}



		bool insert(const char chr, avr_size_t insert_index) {

			if (insert_index <= _cursor) {
				
				if (insert_index + 1 <= size) {
					
					avr_size_t right_index = _cursor > size - 1 ? size - 1 : _cursor;

					for (avr_size_t index = right_index; index >= insert_index + 1; index--)
					{
						_buffer[index] = _buffer[index - 1];
					}

					_buffer[insert_index] = chr;

					_cursor = right_index + 1;
					_buffer[_cursor] = '\0';
					return true;
				}
			}
			return false;
		}

		bool insert(const char* src, avr_size_t insert_index = 2) {
			//	A B C D E F G H	I _ _ _|
			//	A B X X X X C D E F G I|
			if (insert_index < size) {

				avr_size_t src_length = str::length(src);

				if (insert_index + src_length <= size) {

					avr_size_t right_index = _cursor - 1 + src_length > size - 1 ? size - 1 : _cursor - 1 + src_length;

					for (avr_size_t index = right_index; index >= insert_index + src_length; index--)
					{
						_buffer[index] = _buffer[index - src_length];
					}

					for (avr_size_t index = insert_index; index < insert_index + src_length; index++)
					{
						_buffer[index] = *src++;
					}

					_cursor = right_index + 1;
					_buffer[_cursor] = '\0';
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
		avr_size_t _cursor;
		//requires space for string terminator
		char _buffer[size + 1];
	};
}


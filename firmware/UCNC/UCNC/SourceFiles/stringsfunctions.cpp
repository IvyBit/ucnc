
#include "definitions.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

namespace str{

	const char* find(const char* src, char chr) {
		while (*src != STR_END) {
			if(*src == chr){
				return src;
			}
			src++;
		}
		return nullptr;
	}

	char* find_end(char* src) {
		while (*src != STR_END) {
			src++;
		}
		return src;
	}

	const char* find_end(const char* src) {
		while (*src != STR_END) {
			src++;
		}
		return src;
	}


	//returns the previous character from the string or NULL if the pointer already points to NULL
	char lead(const char* src) {
		if (*src ==STR_END) return STR_END;
		return *(src - 1);
	}

	//returns the next character from the string or NULL if the pointer already points to NULL
	char lag(const char* src) {
		if (*src ==STR_END) return STR_END;
		return *(src + 1);
	}

	//returns the index of the first match or NO_MATCH (-1) starting at start_index
	int16_t index_of(const char* src, int16_t start_index, char match) {
		for (; src[start_index] !=STR_END; start_index++) {
			if (src[start_index] == match) return start_index;
		}
		return STR_NO_MATCH;
	}

	//returns the index of the first match or NO_MATCH (-1)
	int16_t index_of(const char* src, char match) {
		return index_of(src, 0, match);
	}

	//returns the index of the first match or NO_MATCH (-1) starting at start_index
	int16_t index_of(const char* src, int16_t start_index, const char* match) {

		for (; src[start_index] !=STR_END; start_index++) {
			bool matched = true;
			uint16_t index = start_index;
			for (const char* m = match; *m !=STR_END; m++) {
				if(*m != src[index++]) {
					matched = false;
					break;
				}
			}
			if(matched){
				return start_index;
			}
		}
		return STR_NO_MATCH;
	}

	//returns the index of the first match or NO_MATCH (-1) starting at start_index
	int16_t index_of_P(const char* src, int16_t start_index, const char* match) {

		for (; src[start_index] !=STR_END; start_index++) {
			bool matched = true;
			uint16_t index = start_index;
			for (const char* m = match; pgm_read_byte(m) !=STR_END; m++) {
				if(pgm_read_byte(m) != src[index++]) {
					matched = false;
					break;
				}
			}
			if(matched){
				return start_index;
			}
		}
		return STR_NO_MATCH;
	}

	//returns the index of the first match or NO_MATCH (-1)
	int16_t index_of(const char* src, const char* match) {
		return index_of(src, 0, match);
	}

	//returns the index of the first match or NO_MATCH (-1)
	int16_t index_of_P(const char* src, const char* match) {
		return index_of_P(src, 0, match);
	}


	//returns true if src contains chr
	bool contains(const char* src, const char chr) {
		return str::index_of(src, chr) != STR_NO_MATCH;
	}

	//returns true if match occurs in src
	bool contains(const char* src, const char* match) {
		return str::index_of(src, match) != STR_NO_MATCH;
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
		for (char* lc = src; *lc !=STR_END; lc++) {
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
		for (char* lc = src; *lc !=STR_END; lc++) {
			if (*lc >= 'A' && *lc <= 'Z') *lc = (char)(*lc + 0x20);
		}
	}

	uint16_t length(const char* src) {
		uint16_t index = 0;
		for (; src[index] != '\0'; index++){}
		return index;
	}

	//skip whitespace and tab space
	void skip_blank(const char* &src) {
		while (is_blank(*src)) src++;
	}

	//skip non numeric
	const char* find_digit(const char* src) {
		while (*src != STR_END) {
			if(is_digit(*src)){
				return src;
			}
			src++;
		}
		return nullptr;
	}


};
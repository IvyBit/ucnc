
#include <stdint-gcc.h>

#ifndef STRING_H_
#define STRING_H_

namespace str {

	//returns pointer to first match or nullptr
	const char* find(const char* src, char chr);

	char* find_end(char* src);

	const char* find_end(const char* src);

	//returns the previous character from the string or NULL if the pointer already points to NULL
	char lead(const char* src);

	//returns the next character from the string or NULL if the pointer already points to NULL
	char lag(const char* src);

	//returns the index of the first match or NO_MATCH (-1) starting at start_index
	int16_t index_of(const char* src, int16_t start_index, char match);

	//returns the index of the first match or NO_MATCH (-1)
	int16_t index_of(const char* src, char match);

	//returns the index of the first match or NO_MATCH (-1) starting at start_index
	int16_t index_of(const char* src, int16_t start_index, const char* match);

	//returns the index of the first match or NO_MATCH (-1) starting at start_index
	int16_t index_of_P(const char* src, int16_t start_index, const char* match);

	//returns the index of the first match or NO_MATCH (-1)
	int16_t index_of(const char* src, const char* match);

	//returns the index of the first match or NO_MATCH (-1)
	int16_t index_of_P(const char* src, const char* match);

	//returns true if src contains chr
	bool contains(const char* src, const char chr);

	//returns true if match occurs in src
	bool contains(const char* src, const char* match);

	//Checks whether chr is either a decimal digit or an uppercase or lowercase letter.
	bool is_alphanum(const char chr);

	//Checks whether chr is an alphabetic letter.
	bool is_alpha(const char chr);

	//Checks whether chr is a blank character.
	bool is_blank(const char chr);

	//Checks whether chr is a decimal digit character.
	bool is_digit(const char chr);

	//Checks whether chr is a lowercase letter.
	bool is_lower(const char chr);

	//Checks if parameter chr is an uppercase alphabetic letter.
	bool is_upper(const char chr);

	//considers codes 0x00 (NUL) and 0x1f (US), plus 0x7f (DEL).
	bool is_control(const char chr);

	//Checks whether chr is a printable character.
	bool is_print(const char chr);

	//Checks whether chr is a character with graphical representation.
	bool is_graph(const char chr);

	//Checks whether chr is a punctuation character.
	bool is_punct(const char chr);

	//Checks whether chr is a white-space character.
	bool is_space(const char chr);

	//Checks whether chr is a hexdecimal digit character.
	bool is_hex(const char chr);

	//Convert uppercase letter to lowercase
	char to_lower(char chr);

	//Convert uppercase string to lowercase
	void to_lower(char* src);

	//Convert lowercase letter to uppercase
	char to_upper(char chr);

	//Convert lowercase string to uppercase
	void to_upper(char* src);

	uint16_t length(const char* src);

	//skip whitespace and tab space
	void skip_blank(const char* &src);

	//skip non numeric
	const char* find_digit(const char* src);

};
#endif /* STRING_H_ */
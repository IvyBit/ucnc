/*
 * parser.h
 *
 * Created: 09.06.2021 22:13:37
 *  Author: alrha
 */

#include <avr/pgmspace.h>
#include "expression.h"

#ifndef PARSER_H_
#define PARSER_H_
namespace parser{

	const char CERR_OK[] PROGMEM							= "Ok";
	const char CERR_ERROR_MAX_LENGTH_EXCEEDED	[] PROGMEM	= "Max length exceeded";
	const char CERR_ERROR_INVALID_OPERAND[] PROGMEM			= "Invalid operand";
	const char CERR_ERROR_MISSING_BRACKET[] PROGMEM			= "Missing parenthesis";
	const char CERR_ERROR_INVALID_CHARACTER[] PROGMEM		= "Invalid character";
	const char CERR_ERROR_INVALID_RHS_EXPRESSION[] PROGMEM	= "Invalid rhs expression";
	const char CERR_ERROR_INVALID_LHS_EXPRESSION[] PROGMEM	= "Invalid lhs expression";
	const char CERR_ERROR_MISSING_RHS_EXPRESSION[] PROGMEM	= "Missing rhs expression";
	const char CERR_ERROR_MISSING_LHS_EXPRESSION[] PROGMEM	= "Missing lhs expression";
	const char CERR_ERROR_EMPTY_EXPRESSION[] PROGMEM		= "Empty expression";

	enum class parser_status : uint8_t {
		OK = 0x00,					//No Error
		ERROR_MAX_LENGTH_EXCEEDED		= 0x01,
		ERROR_INVALID_OPERAND			= 0x02,
		ERROR_MISSING_BRACKET			= 0x03,
		ERROR_INVALID_CHARACTER			= 0x04,
		ERROR_INVALID_RHS_EXPRESSION	= 0x05,
		ERROR_INVALID_LHS_EXPRESSION	= 0x06,
		ERROR_MISSING_RHS_EXPRESSION	= 0x07,
		ERROR_MISSING_LHS_EXPRESSION	= 0x08,
		ERROR_EMPTY_EXPRESSION			= 0x09,
	};

	struct parser_result {
		uint16_t		index = 0;
		parser_status	status = parser_status::OK;
	};

	const char* get_compiler_status_msg(parser_status status);



	bool fetch_operand(const char* &exp_src, expr::expression& target, uint16_t& target_index);

	bool fetch_operator(const char*& exp_src, expr::expression& target, uint16_t& target_index);

	bool fetch_limiter(const char*& exp_src, expr::expression& target, uint16_t& target_index);

	void validate_syntax(const char* exp_src, parser_result &cr);



	void tokenize_expression(const char* exp_src, expr::expression& target, parser_result& cr);

	void validate_expression(expr::expression& target, parser_result& cr);

	void convert_postfix(expr::expression& target, parser_result& cr);



	bool parse_expression(const char* exp_src,
							expr::expression& target,
							parser_result& cr);

};
#endif /* PARSER_H_ */
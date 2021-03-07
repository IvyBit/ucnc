#pragma once



#include "avrtypes.h"
#include "strings.h"
#include "opcodes.h"
#include "containers.h"

namespace comp {

	const char CERR_OK[] PROGMEM							= "\x1B[32mOk\x1B[0m";
	const char CERR_ERROR_MAX_LENGTH_EXCEEDED	[] PROGMEM	= "\x1B[31mMax length exceeded\x1B[0m";
	const char CERR_ERROR_INVALID_OPERAND[] PROGMEM			= "\x1B[31mInvalid operand\x1B[0m";
	const char CERR_ERROR_MISSING_BRACKET[] PROGMEM			= "\x1B[31mMissing bracket\x1B[0m";
	const char CERR_ERROR_INVALID_CHARACTER[] PROGMEM		= "\x1B[31mInvalid character\x1B[0m";
	const char CERR_ERROR_INVALID_RHS_EXPRESSION[] PROGMEM	= "\x1B[31mInvalid rhs expression\x1B[0m";
	const char CERR_ERROR_INVALID_LHS_EXPRESSION[] PROGMEM	= "\x1B[31mInvalid lhs expression\x1B[0m";
	const char CERR_ERROR_MISSING_RHS_EXPRESSION[] PROGMEM	= "\x1B[31mMissing rhs expression\x1B[0m";
	const char CERR_ERROR_MISSING_LHS_EXPRESSION[] PROGMEM	= "\x1B[31mMissing lhs expression\x1B[0m";
	const char CERR_ERROR_EMPTY_EXPRESSION[] PROGMEM		= "\x1B[31mEmpty expression\x1B[0m";
	
	enum class CompilerStatus : uint8_t {
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
	

	
	const char* get_compiler_status_msg(CompilerStatus status){
		switch (status)
		{
			case CompilerStatus::OK:							return CERR_OK;
			case CompilerStatus::ERROR_MAX_LENGTH_EXCEEDED:		return CERR_ERROR_MAX_LENGTH_EXCEEDED;
			case CompilerStatus::ERROR_INVALID_OPERAND:			return CERR_ERROR_INVALID_OPERAND;
			case CompilerStatus::ERROR_MISSING_BRACKET:			return CERR_ERROR_MISSING_BRACKET;
			case CompilerStatus::ERROR_INVALID_CHARACTER:		return CERR_ERROR_INVALID_CHARACTER;
			case CompilerStatus::ERROR_INVALID_RHS_EXPRESSION:	return CERR_ERROR_INVALID_RHS_EXPRESSION;
			case CompilerStatus::ERROR_INVALID_LHS_EXPRESSION:	return CERR_ERROR_INVALID_LHS_EXPRESSION;
			case CompilerStatus::ERROR_MISSING_RHS_EXPRESSION:	return CERR_ERROR_MISSING_RHS_EXPRESSION;
			case CompilerStatus::ERROR_MISSING_LHS_EXPRESSION:	return CERR_ERROR_MISSING_LHS_EXPRESSION;
			case CompilerStatus::ERROR_EMPTY_EXPRESSION:		return CERR_ERROR_EMPTY_EXPRESSION;
			default : return nullptr;
		}
	}


	struct CompilerResult {
		avr_size_t		index = 0;
		CompilerStatus	status = CompilerStatus::OK;
	};


	
		bool fetch_operand(const char* &exp_src, ex::Expression& target, avr_size_t& target_index) {
		
			str::skip_blank(exp_src);

			if (*exp_src == '$') {

				exp_src++;
	
				switch (*exp_src)
				{
				case 'A':
					target[target_index++] = op::OpCode::OPD_A;
					break;
				case 'B':
					target[target_index++] = op::OpCode::OPD_B;
					break;
				case 'C':
					target[target_index++] = op::OpCode::OPD_C;
					break;
				case 'D':
					target[target_index++] = op::OpCode::OPD_D;
					break;
				case 'E':
					target[target_index++] = op::OpCode::OPD_E;
					break;
				case 'F':
					target[target_index++] = op::OpCode::OPD_F;
					break;
				case 'G':
					target[target_index++] = op::OpCode::OPD_G;
					break;
				case 'H':
					target[target_index++] = op::OpCode::OPD_H;
					break;
				default:
					return false;
				}
				exp_src++;
				return true;
			}
			else {
				return false;
			}
		}
		
		bool fetch_operator(const char*& exp_src, ex::Expression& target, avr_size_t& target_index) {

			str::skip_blank(exp_src);

			switch (*exp_src)
			{
			case '~':
				switch (str::lag(exp_src))
				{
				case '&':
					target[target_index++] = op::OpCode::NAND;
					exp_src++;
					break;
				case '|':
					target[target_index++] = op::OpCode::NOR;
					exp_src++;
					break;
				case '=':
					target[target_index++] = op::OpCode::XOR;
					exp_src++;
					break;
				default:
					target[target_index++] = op::OpCode::NOT;
					break;
				}
				break; 

			case '&':
				target[target_index++] = op::OpCode::AND;
				break;

			case '|':
				target[target_index++] = op::OpCode::OR;
				break;

			case '=':
				target[target_index++] = op::OpCode::EQ;
				break;
			default:
				return false;
				break;
			}	

			exp_src++;
			return true;		
		}

		bool fetch_limiter(const char*& exp_src, ex::Expression& target, avr_size_t& target_index) {

			str::skip_blank(exp_src);

			switch (*exp_src)
			{
			case '(':
				target[target_index++] = op::OpCode::EPS;
				break;

			case ')':
				target[target_index++] = op::OpCode::EPE;
				break;

			default:
				return false;
				break;
			}

			exp_src++;
			return true;
		}


		void tokenize_expression(const char* exp_src, ex::Expression& target, CompilerResult& cr) {

			const char* moving_ptr = exp_src;
			avr_size_t target_index = 0;			
			bool fetched = false;
			
			do
			{
				fetched = false;

				//fetch next operand
				if(!fetched) fetched |= fetch_operand(moving_ptr, target, target_index);

				//fetch next operator
				if (!fetched) fetched |= fetch_operator(moving_ptr, target, target_index);

				//fetch sub expression limiter
				if (!fetched) fetched |= fetch_limiter(moving_ptr, target, target_index);


			} while (fetched);
			
			target.set_length(target_index);
			
			if(target_index == 0){
				cr.status = CompilerStatus::ERROR_EMPTY_EXPRESSION;
			}
		}

	   
	    //check if expression contains invalid characters or brackets don't match
		template<avr_size_t stack_buffer_size>
		void validate_syntax(const char* exp_src, ctr::Array<op::OpCode, stack_buffer_size>& stack_buffer, CompilerResult &cr) {

			int8_t brackets = 0;
			for (const char* vsrc = exp_src; *vsrc != '\0'; vsrc++) {
			
				if (cr.index >= stack_buffer_size) {
					cr.status = CompilerStatus::ERROR_MAX_LENGTH_EXCEEDED;
				}

				//only test non alphanum and non blank symbols
				if (!str::is_alpha(*vsrc) && !str::is_blank(*vsrc)) {
					switch (*vsrc)
					{
					case '$':
						if (str::lag(vsrc) < 'A' || str::lag(vsrc) > 'H') {
							cr.status = CompilerStatus::ERROR_INVALID_OPERAND;
							return;
						}
						vsrc ++;
						break;

					case '(' :
						brackets++;
						break;

					case ')':
						brackets--;
						break;

					//ignore valid symbols
					case ' ': case '\t': case '~': case '&': case '|': case '=': break;

					default:
						cr.status = CompilerStatus::ERROR_INVALID_CHARACTER;
						return;
					}
				}


				if (brackets < 0) {
					cr.status = CompilerStatus::ERROR_MISSING_BRACKET;
					return;
				}

				cr.index++;
			}

			//check for bracket mismatch
			if (brackets != 0) {
				cr.status = CompilerStatus::ERROR_MISSING_BRACKET;
			}
		}


		void validate_syntax(ex::Expression& target, CompilerResult& cr) {

			for (avr_size_t index = 0; index < target.length(); index++)
			{
				const op::OpCode op = target[index];
				const op::OpCode* rhs = (index + 1) < target.length() ? &target[index + 1] : nullptr;
				const op::OpCode* lhs = (index > 0) ? &target[index - 1] : nullptr;

				if (op::is_operator(op)) {
				
					//NOT is special because it doesn't require an lhs operand
					if (op == op::OpCode::NOT) {
						//normal operators require rhs operand
						if (!rhs) {
							cr.status = CompilerStatus::ERROR_MISSING_RHS_EXPRESSION;
							return;
						}

						//check if the lhs token is an operand or the start of a expression
						if (*rhs != op::OpCode::EPS && !op::is_operand(*rhs)) {
							cr.status = CompilerStatus::ERROR_INVALID_RHS_EXPRESSION;
							return;
						}
					}
					else {
						//normal operators need both lhs and rhs operands
						if (!rhs) {
							cr.status = CompilerStatus::ERROR_MISSING_RHS_EXPRESSION;
							return;
						}

						//normal operators need both lhs and rhs operands
						if (!lhs) {
							cr.status = CompilerStatus::ERROR_MISSING_LHS_EXPRESSION;
							return;
						}

						//check if the lhs token is an operand or the end of a expression
						if (*lhs != op::OpCode::EPE && !op::is_operand(*lhs)) {
							cr.status = CompilerStatus::ERROR_INVALID_LHS_EXPRESSION;
							return;
						}
						//check if the rhs token is an operand or the start of a expression
						if (*rhs != op::OpCode::EPS && !op::is_operand(*rhs) && *rhs != op::OpCode::NOT) {
							cr.status = CompilerStatus::ERROR_INVALID_RHS_EXPRESSION;
							return;
						}
					}
				}//OPERATOR


				if (op::is_operand(op)) {
					//if an rhs token exists check it
					if (rhs) {
						//operand can either have an operator or closing brackets
						if (op::is_operand(*rhs) && !op::is_end(*rhs)) {
							cr.status = CompilerStatus::ERROR_INVALID_RHS_EXPRESSION;
							return;
						}
					}

					//if an lhs token exists check it
					if (lhs) {
						//operand can either have an operator or opening brackets
						if (op::is_operand(*lhs) && !op::is_start(*lhs)) {
							cr.status = CompilerStatus::ERROR_INVALID_LHS_EXPRESSION;
							return;
						}
					}				
				}//OPERAND


				if (op::is_subexp(op)) {
					if (op::is_start(op)) {
						//if an rhs token exists check it
						if (rhs) {
							if (!op::is_operand(*rhs) && *rhs != op::OpCode::NOT && !op::is_start(*rhs)) {
								cr.status = CompilerStatus::ERROR_INVALID_RHS_EXPRESSION;
								return;
							}
						}
						else {
							cr.status = CompilerStatus::ERROR_MISSING_RHS_EXPRESSION;
							return;
						}

						//if an lhs token exists check it
						if (lhs) {
							if (!op::is_operator(*lhs) && !op::is_start(*lhs)) {
								cr.status = CompilerStatus::ERROR_INVALID_LHS_EXPRESSION;
								return;
							}
						}
					}//OPENING BRACKETS

					if (op::is_end(op)) {
						//if an rhs token exists check it
						if (rhs) {
							if (!op::is_operator(*rhs) && ! op::is_end(*rhs)) {
								cr.status = CompilerStatus::ERROR_INVALID_RHS_EXPRESSION;
								return;
							}
						}

						//if an lhs token exists check it
						if (lhs) {
							if (!op::is_operand(*lhs) && !op::is_end(*lhs)) {
								cr.status = CompilerStatus::ERROR_INVALID_LHS_EXPRESSION;
								return;
							}
						}
						else {
							cr.status = CompilerStatus::ERROR_MISSING_LHS_EXPRESSION;
							return;
						}
					}//CLOSING BRACKETS

				}//SUBEXPRESSION

			}
		}
	
	
		template<avr_size_t stack_buffer_size>
		void convert_postfix( ex::Expression& target, ctr::Array<op::OpCode, stack_buffer_size>& stack_buffer, CompilerResult& cr) {
			
			avr_size_t stack_index = 0; 
			avr_size_t target_index = 0;
			
			for (avr_size_t index = 0; index < target.length(); index++)
			{		
			
				op::OpCode op = target[index];

			
				if (op != op::OpCode::NONE) {

					if (op::is_operand(op)) {
						target[target_index++] = op;
					}
					else if (op::is_start(op)) {
						stack_buffer[stack_index++] = op;
					}
					else if (op::is_end(op)) {

						while (stack_index > 0 &&
							!op::is_start(stack_buffer[stack_index - 1])) {

							target[target_index++] = stack_buffer[--stack_index];
						}
						stack_index--;
					}
					else if (op::is_operator(op)) {
						//check precedence of current and last operator
						while (stack_index > 0 &&
							//token is not the end of an expression
							!op::is_start(stack_buffer[stack_index - 1]) &&
							//operator in the stack has higher precedence
							op::is_gte(stack_buffer[stack_index - 1], op)) {

							target[target_index++] = stack_buffer[--stack_index];
						}
						stack_buffer[stack_index++] = op;
					}
				}
			}

			//append all remaining tokens
			while (stack_index > 0) {
				target[target_index++] = stack_buffer[--stack_index];
			}
			target.set_length(target_index);

		}

		template<avr_size_t text_size, avr_size_t stack_buffer_size>
		bool compile_expression(str::StringBuffer<text_size>& exp_src,
								ex::Expression& target,
								ctr::Array<op::OpCode, stack_buffer_size>& stack_buffer,
								CompilerResult& cr) {
			
			if (cr.status != CompilerStatus::OK) return false;
			validate_syntax(exp_src, stack_buffer, cr);
			

			if (cr.status != CompilerStatus::OK) return false;
			tokenize_expression(exp_src, target, cr);


			if (cr.status != CompilerStatus::OK) return false;
			validate_syntax(target, cr);


			if (cr.status != CompilerStatus::OK) return false;
			convert_postfix(target, stack_buffer, cr);

			return cr.status == CompilerStatus::OK;
		}
		

};
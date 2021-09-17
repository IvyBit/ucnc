/*
 * parser.cpp
 *
 * Created: 10.06.2021 17:10:49
 *  Author: alrha
 */
 #include "config.h"
 #include "parser.h"
 #include "array.h"
 #include "stringbuffer.h"
 namespace parser{

	const char* get_compiler_status_msg(parser_status status){
		switch (status)
		{
			case parser_status::OK:							return CERR_OK;
			case parser_status::ERROR_MAX_LENGTH_EXCEEDED:	return CERR_ERROR_MAX_LENGTH_EXCEEDED;
			case parser_status::ERROR_INVALID_OPERAND:		return CERR_ERROR_INVALID_OPERAND;
			case parser_status::ERROR_MISSING_BRACKET:		return CERR_ERROR_MISSING_BRACKET;
			case parser_status::ERROR_INVALID_CHARACTER:		return CERR_ERROR_INVALID_CHARACTER;
			case parser_status::ERROR_INVALID_RHS_EXPRESSION:	return CERR_ERROR_INVALID_RHS_EXPRESSION;
			case parser_status::ERROR_INVALID_LHS_EXPRESSION:	return CERR_ERROR_INVALID_LHS_EXPRESSION;
			case parser_status::ERROR_MISSING_RHS_EXPRESSION:	return CERR_ERROR_MISSING_RHS_EXPRESSION;
			case parser_status::ERROR_MISSING_LHS_EXPRESSION:	return CERR_ERROR_MISSING_LHS_EXPRESSION;
			case parser_status::ERROR_EMPTY_EXPRESSION:		return CERR_ERROR_EMPTY_EXPRESSION;
			default : return nullptr;
		}
	}


		bool fetch_operand(const char* &exp_src, expr::expression& target, uint16_t& target_index) {

			str::skip_blank(exp_src);

			if (*exp_src == '$') {

				exp_src++;

				switch (*exp_src)
				{
					case 'A':
					target.data[target_index++] = opcodes::op_code::OPD_A;
					break;
					case 'B':
					target.data[target_index++] = opcodes::op_code::OPD_B;
					break;
					case 'C':
					target.data[target_index++] = opcodes::op_code::OPD_C;
					break;
					case 'D':
					target.data[target_index++] = opcodes::op_code::OPD_D;
					break;
					case 'E':
					target.data[target_index++] = opcodes::op_code::OPD_E;
					break;
					case 'F':
					target.data[target_index++] = opcodes::op_code::OPD_F;
					break;
					case 'G':
					target.data[target_index++] = opcodes::op_code::OPD_G;
					break;
					case 'H':
					target.data[target_index++] = opcodes::op_code::OPD_H;
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

		bool fetch_operator(const char*& exp_src, expr::expression& target, uint16_t& target_index) {

			str::skip_blank(exp_src);

			switch (*exp_src)
			{
				case '~':
				switch (str::lag(exp_src))
				{
					case '&':
					target.data[target_index++] = opcodes::op_code::NAND;
					exp_src++;
					break;
					case '|':
					target.data[target_index++] = opcodes::op_code::NOR;
					exp_src++;
					break;
					case '=':
					target.data[target_index++] = opcodes::op_code::XOR;
					exp_src++;
					break;
					default:
					target.data[target_index++] = opcodes::op_code::NOT;
					break;
				}
				break;

				case '&':
				target.data[target_index++] = opcodes::op_code::AND;
				break;

				case '|':
				target.data[target_index++] = opcodes::op_code::OR;
				break;

				case '=':
				target.data[target_index++] = opcodes::op_code::EQ;
				break;
				default:
				return false;
				break;
			}

			exp_src++;
			return true;
		}

		bool fetch_limiter(const char*& exp_src, expr::expression& target, uint16_t& target_index) {

			str::skip_blank(exp_src);

			switch (*exp_src)
			{
				case '(':
				target.data[target_index++] = opcodes::op_code::EPS;
				break;

				case ')':
				target.data[target_index++] = opcodes::op_code::EPE;
				break;

				default:
				return false;
				break;
			}

			exp_src++;
			return true;
		}

		void validate_syntax(const char* exp_src, parser_result &cr) {

			int8_t brackets = 0;
			for (const char* vsrc = exp_src; *vsrc != '\0' && *vsrc != '#'; vsrc++) {

				if (cr.index >= EXPRESSION_SIZE) {
					cr.status = parser_status::ERROR_MAX_LENGTH_EXCEEDED;
				}

				//only test non alphanum and non blank symbols
				if (!str::is_alpha(*vsrc) && !str::is_blank(*vsrc)) {
					switch (*vsrc)
					{
						case '$':
						if (str::lag(vsrc) < 'A' || str::lag(vsrc) > 'H') {
							cr.status = parser_status::ERROR_INVALID_OPERAND;
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
						cr.status = parser_status::ERROR_INVALID_CHARACTER;
						return;
					}
				}


				if (brackets < 0) {
					cr.status = parser_status::ERROR_MISSING_BRACKET;
					return;
				}

				cr.index++;
			}

			//check for bracket mismatch
			if (brackets != 0) {
				cr.status = parser_status::ERROR_MISSING_BRACKET;
			}
		}



		void tokenize_expression(const char* exp_src, expr::expression& target, parser_result& cr) {

			const char* moving_ptr = exp_src;
			uint16_t target_index = 0;
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

			target.length = target_index;

			if(target_index == 0){
				cr.status = parser_status::ERROR_EMPTY_EXPRESSION;
			}
		}

		void validate_expression(expr::expression& target, parser_result& cr) {

			for (uint16_t index = 0; index < target.length; index++)
			{
				const opcodes::op_code op = target.data[index];
				const opcodes::op_code* rhs = (index + 1) < target.length ? &target.data[index + 1] : nullptr;
				const opcodes::op_code* lhs = (index > 0) ? &target.data[index - 1] : nullptr;

				if (opcodes::is_operator(op)) {

					//NOT is special because it doesn't require an lhs operand
					if (op == opcodes::op_code::NOT) {
						//normal operators require rhs operand
						if (!rhs) {
							cr.status = parser_status::ERROR_MISSING_RHS_EXPRESSION;
							return;
						}

						//check if the lhs token is an operand or the start of a expression
						if (*rhs != opcodes::op_code::EPS && !opcodes::is_operand(*rhs)) {
							cr.status = parser_status::ERROR_INVALID_RHS_EXPRESSION;
							return;
						}
					}
					else {
						//normal operators need both lhs and rhs operands
						if (!rhs) {
							cr.status = parser_status::ERROR_MISSING_RHS_EXPRESSION;
							return;
						}

						//normal operators need both lhs and rhs operands
						if (!lhs) {
							cr.status = parser_status::ERROR_MISSING_LHS_EXPRESSION;
							return;
						}

						//check if the lhs token is an operand or the end of a expression
						if (*lhs != opcodes::op_code::EPE && !opcodes::is_operand(*lhs)) {
							cr.status = parser_status::ERROR_INVALID_LHS_EXPRESSION;
							return;
						}
						//check if the rhs token is an operand or the start of a expression
						if (*rhs != opcodes::op_code::EPS && !opcodes::is_operand(*rhs) && *rhs != opcodes::op_code::NOT) {
							cr.status = parser_status::ERROR_INVALID_RHS_EXPRESSION;
							return;
						}
					}
				}//OPERATOR


				if (opcodes::is_operand(op)) {
					//if an rhs token exists check it
					if (rhs) {
						//operand can either have an operator or closing brackets
						if (opcodes::is_operand(*rhs) && !opcodes::is_end(*rhs)) {
							cr.status = parser_status::ERROR_INVALID_RHS_EXPRESSION;
							return;
						}
					}

					//if an lhs token exists check it
					if (lhs) {
						//operand can either have an operator or opening brackets
						if (opcodes::is_operand(*lhs) && !opcodes::is_start(*lhs)) {
							cr.status = parser_status::ERROR_INVALID_LHS_EXPRESSION;
							return;
						}
					}
				}//OPERAND


				if (opcodes::is_subexp(op)) {
					if (opcodes::is_start(op)) {
						//if an rhs token exists check it
						if (rhs) {
							if (!opcodes::is_operand(*rhs) && *rhs != opcodes::op_code::NOT && !opcodes::is_start(*rhs)) {
								cr.status = parser_status::ERROR_INVALID_RHS_EXPRESSION;
								return;
							}
						}
						else {
							cr.status = parser_status::ERROR_MISSING_RHS_EXPRESSION;
							return;
						}

						//if an lhs token exists check it
						if (lhs) {
							if (!opcodes::is_operator(*lhs) && !opcodes::is_start(*lhs)) {
								cr.status = parser_status::ERROR_INVALID_LHS_EXPRESSION;
								return;
							}
						}
					}//OPENING BRACKETS

					if (opcodes::is_end(op)) {
						//if an rhs token exists check it
						if (rhs) {
							if (!opcodes::is_operator(*rhs) && ! opcodes::is_end(*rhs)) {
								cr.status = parser_status::ERROR_INVALID_RHS_EXPRESSION;
								return;
							}
						}

						//if an lhs token exists check it
						if (lhs) {
							if (!opcodes::is_operand(*lhs) && !opcodes::is_end(*lhs)) {
								cr.status = parser_status::ERROR_INVALID_LHS_EXPRESSION;
								return;
							}
						}
						else {
							cr.status = parser_status::ERROR_MISSING_LHS_EXPRESSION;
							return;
						}
					}//CLOSING BRACKETS

				}//SUBEXPRESSION

			}
		}

		void convert_postfix(expr::expression& target, parser_result& cr) {


			containers::array<opcodes::op_code, EXPRESSION_SIZE> stack_buffer;
			uint16_t stack_index = 0;
			uint16_t target_index = 0;

			for (uint16_t index = 0; index < target.length; index++)
			{

				opcodes::op_code op = target.data[index];


				if (op != opcodes::op_code::NONE) {

					if (opcodes::is_operand(op)) {
						target.data[target_index++] = op;
					}
					else if (opcodes::is_start(op)) {
						stack_buffer[stack_index++] = op;
					}
					else if (opcodes::is_end(op)) {

						while (stack_index > 0 &&
						!opcodes::is_start(stack_buffer[stack_index - 1])) {

							target.data[target_index++] = stack_buffer[--stack_index];
						}
						stack_index--;
					}
					else if (opcodes::is_operator(op)) {
						//check precedence of current and last operator
						while (stack_index > 0 &&
						//token is not the end of an expression
						!opcodes::is_start(stack_buffer[stack_index - 1]) &&
						//operator in the stack has higher precedence
						opcodes::is_gte(stack_buffer[stack_index - 1], op)) {

							target.data[target_index++] = stack_buffer[--stack_index];
						}
						stack_buffer[stack_index++] = op;
					}
				}
			}

			//append all remaining tokens
			while (stack_index > 0) {
				target.data[target_index++] = stack_buffer[--stack_index];
			}
			target.length = target_index;

		}



		bool parse_expression(const char* exp_src,
								expr::expression& target,
								parser_result& cr) {


			if (cr.status != parser_status::OK) return false;
			validate_syntax(exp_src, cr);


			if (cr.status != parser_status::OK) return false;
			tokenize_expression(exp_src, target, cr);


			if (cr.status != parser_status::OK) return false;
			validate_expression(target, cr);


			if (cr.status != parser_status::OK) return false;
			convert_postfix(target, cr);

			return cr.status == parser_status::OK;
		}


 };
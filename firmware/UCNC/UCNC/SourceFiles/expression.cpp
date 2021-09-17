/*
 * expression.cpp
 *
 * Created: 10.06.2021 00:23:35
 *  Author: alrha
 */
 #include "expression.h"
 namespace expr{

	opcodes::op_code get_data_bit_at(expression_data data, uint8_t index){
		if((data & (1 << (7 - index))) != 0){
			return opcodes::op_code::TRUE;
			} else {
			return opcodes::op_code::FALSE;
		}
	}
	const opcodes::op_code get_data_bit_a(expression_data data) { return get_data_bit_at(data, 0); }
	const opcodes::op_code get_data_bit_b(expression_data data) { return get_data_bit_at(data, 1); }
	const opcodes::op_code get_data_bit_c(expression_data data) { return get_data_bit_at(data, 2); }
	const opcodes::op_code get_data_bit_d(expression_data data) { return get_data_bit_at(data, 3); }
	const opcodes::op_code get_data_bit_e(expression_data data) { return get_data_bit_at(data, 4); }
	const opcodes::op_code get_data_bit_f(expression_data data) { return get_data_bit_at(data, 5); }
	const opcodes::op_code get_data_bit_g(expression_data data) { return get_data_bit_at(data, 6); }
	const opcodes::op_code get_data_bit_h(expression_data data) { return get_data_bit_at(data, 7); }

	void set_data_bit_at(expression_data &data, opcodes::op_code state, uint8_t index){
		if (state == opcodes::op_code::TRUE)
		data |= (1 << (7 - index));
		else
		data &= ~(1 << (7 - index));
	}
	void set_data_bit_a(expression_data &data, opcodes::op_code state ) { set_data_bit_at(data, state, 0); }
	void set_data_bit_b(expression_data &data, opcodes::op_code state ) { set_data_bit_at(data, state, 1); }
	void set_data_bit_c(expression_data &data, opcodes::op_code state ) { set_data_bit_at(data, state, 2); }
	void set_data_bit_d(expression_data &data, opcodes::op_code state ) { set_data_bit_at(data, state, 3); }
	void set_data_bit_e(expression_data &data, opcodes::op_code state ) { set_data_bit_at(data, state, 4); }
	void set_data_bit_f(expression_data &data, opcodes::op_code state ) { set_data_bit_at(data, state, 5); }
	void set_data_bit_g(expression_data &data, opcodes::op_code state ) { set_data_bit_at(data, state, 6); }
	void set_data_bit_h(expression_data &data, opcodes::op_code state ) { set_data_bit_at(data, state, 7); }

	opcodes::op_code execute(expression& exp_target, expression_data& input_data) {

		opcodes::op_code stack_buffer[EXPRESSION_SIZE];
		uint16_t stack_index = 0;

		for (uint16_t index = 0; index < exp_target.length; index++) {

			opcodes::op_code op = exp_target.data[index];

			if (op != opcodes::op_code::NONE) {

				switch (op){

					case opcodes::op_code::NOT: {
						opcodes::op_code rhs;

						rhs = stack_buffer[--stack_index];

						if (rhs == opcodes::op_code::TRUE)  {
							stack_buffer[stack_index++] = opcodes::op_code::FALSE;
						} else {
							stack_buffer[stack_index++] = opcodes::op_code::TRUE;
						}

					} break;


					case opcodes::op_code::AND: {
						opcodes::op_code rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if (((uint8_t)rhs & (uint8_t)lhs) == (uint8_t)opcodes::op_code::TRUE)  {
							stack_buffer[stack_index++] = opcodes::op_code::TRUE;
						} else {
							stack_buffer[stack_index++] = opcodes::op_code::FALSE;
						}
					} break;

					case opcodes::op_code::NAND: {
						opcodes::op_code rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if (!(rhs == opcodes::op_code::TRUE && lhs == opcodes::op_code::TRUE))  {
							stack_buffer[stack_index++] = opcodes::op_code::TRUE;
						} else {
							stack_buffer[stack_index++] = opcodes::op_code::FALSE;
						}
					} break;


					case opcodes::op_code::OR: {
						opcodes::op_code rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if (rhs == opcodes::op_code::TRUE || lhs == opcodes::op_code::TRUE)  {
							stack_buffer[stack_index++] = opcodes::op_code::TRUE;
						} else {
							stack_buffer[stack_index++] = opcodes::op_code::FALSE;
						}
					} break;

					case opcodes::op_code::NOR: {
						opcodes::op_code rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if (!(rhs == opcodes::op_code::TRUE || lhs == opcodes::op_code::TRUE))  {
							stack_buffer[stack_index++] = opcodes::op_code::TRUE;
						} else {
							stack_buffer[stack_index++] = opcodes::op_code::FALSE;
						}
					} break;

					case opcodes::op_code::XOR: {
						opcodes::op_code rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if ((rhs != lhs)) {
							stack_buffer[stack_index++] = opcodes::op_code::TRUE;
						} else {
							stack_buffer[stack_index++] = opcodes::op_code::FALSE;
						}
					} break;


					case opcodes::op_code::EQ: {
						opcodes::op_code rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if ((rhs == lhs)) {
							stack_buffer[stack_index++] = opcodes::op_code::TRUE;
							} else {
							stack_buffer[stack_index++] = opcodes::op_code::FALSE;
						}
					} break;

					case opcodes::op_code::OPD_A:
						stack_buffer[stack_index++] = get_data_bit_a(input_data);
						break;
					case opcodes::op_code::OPD_B:
						stack_buffer[stack_index++] = get_data_bit_b(input_data);
						break;
					case opcodes::op_code::OPD_C:
						stack_buffer[stack_index++] = get_data_bit_c(input_data);
						break;
					case opcodes::op_code::OPD_D:
						stack_buffer[stack_index++] = get_data_bit_d(input_data);
						break;
					case opcodes::op_code::OPD_E:
						stack_buffer[stack_index++] = get_data_bit_e(input_data);
						break;
					case opcodes::op_code::OPD_F:
						stack_buffer[stack_index++] = get_data_bit_f(input_data);
						break;
					case opcodes::op_code::OPD_G:
						stack_buffer[stack_index++] = get_data_bit_g(input_data);
						break;
					case opcodes::op_code::OPD_H:
						stack_buffer[stack_index++] = get_data_bit_h(input_data);
						break;

					default:
						return opcodes::op_code::INVALID;
						break;
				}
			}
			else {
				break;
			}
		}//LOOP

		if(stack_index > 0) {
			return stack_buffer[--stack_index];
		} else {
			return opcodes::op_code::FALSE;
		}
	}



 };
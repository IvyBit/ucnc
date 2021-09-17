/*
 * expression.h
 *
 * Created: 09.06.2021 23:12:35
 *  Author: alrha
 */

 #include "opcodes.h"
 #include "array.h"
 #include "config.h"
#ifndef EXPRESSION_H_
#define EXPRESSION_H_

namespace expr{

		struct expression{
			opcodes::op_code*  data		= nullptr;
			uint16_t		   length	= 0;
		};

		typedef uint8_t expression_data;


		opcodes::op_code get_data_bit_at(expression_data data, uint8_t index);

		void set_data_bit_at(expression_data &data, opcodes::op_code state, uint8_t index);


		const opcodes::op_code get_data_bit_a(expression_data data);
		const opcodes::op_code get_data_bit_b(expression_data data);
		const opcodes::op_code get_data_bit_c(expression_data data);
		const opcodes::op_code get_data_bit_d(expression_data data);
		const opcodes::op_code get_data_bit_e(expression_data data);
		const opcodes::op_code get_data_bit_f(expression_data data);
		const opcodes::op_code get_data_bit_g(expression_data data);
		const opcodes::op_code get_data_bit_h(expression_data data);


		void set_data_bit_a(opcodes::op_code state, expression_data &data);
		void set_data_bit_b(opcodes::op_code state, expression_data &data);
		void set_data_bit_c(opcodes::op_code state, expression_data &data);
		void set_data_bit_d(opcodes::op_code state, expression_data &data);
		void set_data_bit_e(opcodes::op_code state, expression_data &data);
		void set_data_bit_f(opcodes::op_code state, expression_data &data);
		void set_data_bit_g(opcodes::op_code state, expression_data &data);
		void set_data_bit_h(opcodes::op_code state, expression_data &data);



		opcodes::op_code execute(expression& exp, expression_data& input_data) ;

};
#endif /* EXPRESSION_H_ */
/*
 * op_codes.h
 *
 * Created: 09.06.2021 22:14:49
 *  Author: alrha
 */

 #include "stdint.h"

#ifndef OPCODES_H_
#define OPCODES_H_

namespace opcodes{

	enum class op_code : uint8_t {
		NONE,	//		0000 0000

		//Operators
		NOT		= (1 << 3) | (0 << 0),	//~		0000 1000
		AND		= (1 << 3) | (1 << 0),	//&&	0000 1001
		NAND	= (1 << 3) | (2 << 0),	//~&	0000 1010
		OR		= (1 << 3) | (3 << 0),	//||	0000 1011
		NOR		= (1 << 3) | (4 << 0),	//~|	0000 1100
		XOR		= (1 << 3) | (5 << 0),	//~=	0000 1101
		EQ		= (1 << 3) | (6 << 0),	//==	0000 1110

		//Operands
		OPD_A	= (1 << 4) | (0 << 0),	//$A	0001 0000
		OPD_B	= (1 << 4) | (1 << 0),	//$B	0001 0001
		OPD_C	= (1 << 4) | (2 << 0),	//$C	0001 0010
		OPD_D	= (1 << 4) | (3 << 0),	//$D	0001 0011
		OPD_E	= (1 << 4) | (4 << 0),	//$E	0001 0100
		OPD_F	= (1 << 4) | (5 << 0),	//$F	0001 0101
		OPD_G	= (1 << 4) | (6 << 0),	//$G	0001 0110
		OPD_H	= (1 << 4) | (7 << 0),	//$H	0001 0111

		//Misc tokens
		EPS		= (1 << 5) | (0 << 0),	//(		0010 0000
		EPE		= (1 << 5) | (1 << 0),	//)		0010 0001
		MS		= (1 << 5) | (2 << 0),	//[		0010 0010
		ME		= (1 << 5) | (3 << 0),	//[		0010 0011

		FALSE	= (1 << 6) | (0 << 0),	//F		0100 0000
		TRUE	= (1 << 6) | (1 << 0),	//T		0100 0001

		INVALID = 0xFF
	};


	int8_t precedence(op_code op);

	bool is_gte(op_code lhs_operator, op_code rhs_operator);

	bool is_valid(op_code op);


	bool is_true(op_code op);

	bool is_operator(op_code op);

	bool is_operand(op_code op);

	bool is_subexp(op_code op);

	bool is_start(op_code op);

	bool is_end(op_code op);

};
#endif /* op_codeS_H_ */
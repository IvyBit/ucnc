

#pragma once

#define ERR_UNKNOWN_OP 2

#include "errors.h"
#include "avrtypes.h"


namespace op{
	
enum class OpCode : uint8_t {
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
};



uint8_t precedence(OpCode op) {
	switch (op)
	{
	case op::OpCode::NOT: 
		return 3;
	case op::OpCode::AND: case op::OpCode::NAND: case op::OpCode::EQ:
		return 2;	
	case op::OpCode::OR: case op::OpCode::NOR: case op::OpCode::XOR:
		return 1;	
	default:
		err::on_error(ERR_UNKNOWN_OP);
	}
}




	bool is_gte(OpCode lhs_operator, OpCode rhs_operator) {
		return  precedence(lhs_operator) >= precedence(rhs_operator);
	}

	bool is_valid(OpCode op) {
		switch (op)
		{
		case op::OpCode::NONE:
			break;
		case op::OpCode::NOT:
			break;
		case op::OpCode::AND:
			break;
		case op::OpCode::NAND:
			break;
		case op::OpCode::OR:
			break;
		case op::OpCode::NOR:
			break;
		case op::OpCode::XOR:
			break;
		case op::OpCode::EQ:
			break;
		case op::OpCode::OPD_A:
			break;
		case op::OpCode::OPD_B:
			break;
		case op::OpCode::OPD_C:
			break;
		case op::OpCode::OPD_D:
			break;
		case op::OpCode::OPD_E:
			break;
		case op::OpCode::OPD_F:
			break;
		case op::OpCode::OPD_G:
			break;
		case op::OpCode::OPD_H:
			break;
		case op::OpCode::EPS:
			break;
		case op::OpCode::EPE:
			break;
		case op::OpCode::MS:
			break;
		case op::OpCode::ME:
			break;
		case op::OpCode::FALSE:
			break;
		case op::OpCode::TRUE:
			break;
		default:
			return false;
		}
		return true;
	}


	bool is_true(OpCode op) {
		return op == op::OpCode::TRUE;
	}

	bool is_operator(OpCode op) {
		return ((uint8_t)op & (uint8_t)OpCode::NOT) == (uint8_t)OpCode::NOT;
	}

	bool is_operand(OpCode op) {
		return ((uint8_t)op & (uint8_t)OpCode::OPD_A) == (uint8_t)OpCode::OPD_A;
	}

	bool is_subexp(OpCode op) {
		return op == OpCode::EPS || op == OpCode::EPE;
	}

	bool is_start(OpCode op) {
		return op == OpCode::EPS;
	}

	bool is_end(OpCode op) {
		return op == OpCode::EPE;
	}

};




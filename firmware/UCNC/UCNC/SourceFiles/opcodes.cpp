/*
 * opcodes.cpp
 *
 * Created: 09.06.2021 22:18:48
 *  Author: alrha
 */
 #include "opcodes.h"

 namespace opcodes{

	//(NOT) > (AND = NAND = EQ) > (OR = NOR = XOR)
	 int8_t precedence(op_code op) {
		 switch (op)
		 {
			 case op_code::NOT:
			 return 3;
			 case op_code::AND: case op_code::NAND: case op_code::EQ:
			 return 2;
			 case op_code::OR: case op_code::NOR: case op_code::XOR:
			 return 1;
			 default:
			 return -1;
		 }
	 }

	 bool is_gte(op_code lhs_operator, op_code rhs_operator) {
		 return  precedence(lhs_operator) >= precedence(rhs_operator);
	 }

	 bool is_valid(op_code op) {
		 switch (op)
		 {
			 case op_code::NONE:
			 case op_code::NOT:
			 case op_code::AND:
			 case op_code::NAND:
			 case op_code::OR:
			 case op_code::NOR:
			 case op_code::XOR:
			 case op_code::EQ:
			 case op_code::OPD_A:
			 case op_code::OPD_B:
			 case op_code::OPD_C:
			 case op_code::OPD_D:
			 case op_code::OPD_E:
			 case op_code::OPD_F:
			 case op_code::OPD_G:
			 case op_code::OPD_H:
			 case op_code::EPS:
			 case op_code::EPE:
			 case op_code::MS:
			 case op_code::ME:
			 case op_code::FALSE:
			 case op_code::TRUE:
			 break;

			 default:
			 return false;
		 }
		 return true;
	 }


	 bool is_true(op_code op) {
		 return op == op_code::TRUE;
	 }

	 bool is_operator(op_code op) {
		 return ((uint8_t)op & (uint8_t)op_code::NOT) == (uint8_t)op_code::NOT;
	 }

	 bool is_operand(op_code op) {
		 return ((uint8_t)op & (uint8_t)op_code::OPD_A) == (uint8_t)op_code::OPD_A;
	 }

	 bool is_subexp(op_code op) {
		 return op == op_code::EPS || op == op_code::EPE;
	 }

	 bool is_start(op_code op) {
		 return op == op_code::EPS;
	 }

	 bool is_end(op_code op) {
		 return op == op_code::EPE;
	 }

 };
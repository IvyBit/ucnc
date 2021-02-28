#pragma once
#include "avrtypes.h"
#include "opcodes.h"
#include "containers.h"

namespace ex {
	
	class ExpressionData final {		
		public:
		NONCOPY(ExpressionData)
		ExpressionData(){}
		ExpressionData(const uint8_t data) : _data(data){}
		~ExpressionData(){}
			
		uint8_t data(){ return _data;}
			
		void data(const uint8_t data){ _data = data;}
			
		void set_at(uint8_t index, op::OpCode state){
			if (state == op::OpCode::TRUE)	
				_data |= (1 << (7 - index)); 
			else 
				_data &= ~(1 << (7 - index)); 
		}
		
		op::OpCode get_at(uint8_t index){
			if((_data & (1 << (7 - index))) != 0){
				return op::OpCode::TRUE;
			} else {
				return op::OpCode::FALSE;
			}
		}
		
		const op::OpCode a() { return (_data & (1 << 7)) > 0 ? op::OpCode::TRUE : op::OpCode::FALSE; }
		const op::OpCode b() { return (_data & (1 << 6)) > 0 ? op::OpCode::TRUE : op::OpCode::FALSE; }
		const op::OpCode c() { return (_data & (1 << 5)) > 0 ? op::OpCode::TRUE : op::OpCode::FALSE; }
		const op::OpCode d() { return (_data & (1 << 4)) > 0 ? op::OpCode::TRUE : op::OpCode::FALSE; }
		const op::OpCode e() { return (_data & (1 << 3)) > 0 ? op::OpCode::TRUE : op::OpCode::FALSE; }
		const op::OpCode f() { return (_data & (1 << 2)) > 0 ? op::OpCode::TRUE : op::OpCode::FALSE; }
		const op::OpCode g() { return (_data & (1 << 1)) > 0 ? op::OpCode::TRUE : op::OpCode::FALSE; }
		const op::OpCode h() { return (_data & (1 << 0)) > 0 ? op::OpCode::TRUE : op::OpCode::FALSE; }


		void a(op::OpCode state) { if (state == op::OpCode::TRUE)	_data |= (1 << 7); else _data &= ~(1 << 7); }
		void b(op::OpCode state) { if (state == op::OpCode::TRUE)	_data |= (1 << 6); else _data &= ~(1 << 6); }
		void c(op::OpCode state) { if (state == op::OpCode::TRUE)	_data |= (1 << 5); else _data &= ~(1 << 5); }
		void d(op::OpCode state) { if (state == op::OpCode::TRUE)	_data |= (1 << 4); else _data &= ~(1 << 4); }
		void e(op::OpCode state) { if (state == op::OpCode::TRUE)	_data |= (1 << 3); else _data &= ~(1 << 3); }
		void f(op::OpCode state) { if (state == op::OpCode::TRUE)	_data |= (1 << 2); else _data &= ~(1 << 2); }
		void g(op::OpCode state) { if (state == op::OpCode::TRUE)	_data |= (1 << 1); else _data &= ~(1 << 1); }
		void h(op::OpCode state) { if (state == op::OpCode::TRUE)	_data |= (1 << 0); else _data &= ~(1 << 0); }
			
	private:
		uint8_t _data = 0;
	};
	
	class Expression final{
		public:
		NONCOPY(Expression)
		Expression(){}
		~Expression(){}
					
		avr_size_t length(){
			return _length;
		}
		
		void set_length(avr_size_t length){
			_length = length;
		}
		
		void set_buffer(op::OpCode* buffer_address){
			_data = buffer_address;
		}
		
		op::OpCode& operator[](avr_size_t index){
			return _data[index];
		}
		
		template<avr_size_t stack_buffer_size>
		op::OpCode eval(ExpressionData& input, ctr::Array<op::OpCode, stack_buffer_size>& stack_buffer) {

			avr_size_t stack_index = 0;
			
			for (avr_size_t index = 0; index < _length; index++)
			{
				op::OpCode op = _data[index];
			
				if (op != op::OpCode::NONE) {

					switch (op){

					case op::OpCode::NOT: {
						op::OpCode rhs;
	
						rhs = stack_buffer[--stack_index];		

						if (rhs == op::OpCode::TRUE)  {
							stack_buffer[stack_index++] = op::OpCode::FALSE;
						} else {
							stack_buffer[stack_index++] = op::OpCode::TRUE;
						}

					} break;


					case op::OpCode::AND: {
						op::OpCode rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if (((uint8_t)rhs & (uint8_t)lhs) == (uint8_t)op::OpCode::TRUE)  {
							stack_buffer[stack_index++] = op::OpCode::TRUE;
						} else {
							stack_buffer[stack_index++] = op::OpCode::FALSE;
						}
					} break;

					case op::OpCode::NAND: {
						op::OpCode rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if (!(rhs == op::OpCode::TRUE && lhs == op::OpCode::TRUE))  {
							stack_buffer[stack_index++] = op::OpCode::TRUE;
						} else {
							stack_buffer[stack_index++] = op::OpCode::FALSE;
						}
					} break;


					case op::OpCode::OR: {
						op::OpCode rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if (rhs == op::OpCode::TRUE || lhs == op::OpCode::TRUE)  {
							stack_buffer[stack_index++] = op::OpCode::TRUE;
						} else {
							stack_buffer[stack_index++] = op::OpCode::FALSE;
						}
					} break;
					
					case op::OpCode::NOR: {
						op::OpCode rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if (!(rhs == op::OpCode::TRUE || lhs == op::OpCode::TRUE))  {
							stack_buffer[stack_index++] = op::OpCode::TRUE;
						} else {
							stack_buffer[stack_index++] = op::OpCode::FALSE;
						}
					} break;

					case op::OpCode::XOR: {
						op::OpCode rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];

						if ((rhs != lhs)) {
							stack_buffer[stack_index++] = op::OpCode::TRUE;
						} else {
							stack_buffer[stack_index++] = op::OpCode::FALSE;
						}
					} break;

				
					case op::OpCode::EQ: {
						op::OpCode rhs, lhs;

						rhs = stack_buffer[--stack_index];
						lhs = stack_buffer[--stack_index];
						
						if ((rhs == lhs)) {
							stack_buffer[stack_index++] = op::OpCode::TRUE;
						} else {
							stack_buffer[stack_index++] = op::OpCode::FALSE;
						}
					} break;

					case op::OpCode::OPD_A:
						stack_buffer[stack_index++] = input.a();
						break;						
					case op::OpCode::OPD_B:
						stack_buffer[stack_index++] = input.b();
						break;
					case op::OpCode::OPD_C:
						stack_buffer[stack_index++] = input.c();
						break;
					case op::OpCode::OPD_D:
						stack_buffer[stack_index++] = input.d();
						break;
					case op::OpCode::OPD_E:
						stack_buffer[stack_index++] = input.e();
						break;
					case op::OpCode::OPD_F:
						stack_buffer[stack_index++] = input.f();
						break;
					case op::OpCode::OPD_G:
						stack_buffer[stack_index++] = input.g();
						break;
					case op::OpCode::OPD_H:
						stack_buffer[stack_index++] = input.h();
						break;

					default:
					err::on_error({0, 0});
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
				return op::OpCode::FALSE;	
			}
				
		}

	private:
		op::OpCode*  _data;
		avr_size_t  _length = 0;
	};

};
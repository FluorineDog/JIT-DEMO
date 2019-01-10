//
// Created by mike on 1/10/19.
//

#include "workflow.h"
#include <iostream>

using std::cout;

enum class JITState{
	Init,
	Compiling,
	Terminate,
};


struct ExecState{
	JITState state;
	uint32_t ori_eip;
	llvm::CodeExecutor executor;
}decoding;

void rtl_add(rtlreg_t *dest, rtlreg_t *a, rtlreg_t *b){
	auto& eng = decoding.executor;
	auto va = eng.get_value(a);
	auto vb = eng.get_value(b);
	auto vc = eng().CreateAdd(va, vb);
	eng.set_value(dest, vc);
}

void rtl_li(rtlreg_t *dest, uint32_t imm);

void rtl_j(uint32_t imm);



void functor() {
	rtlreg_t init;
	rtl_li(&init, 100);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_li(&init, 1);
	rtl_add(&init, &init, &init); // 2
	rtl_add(&init, &init, &init); // 4
	rtl_add(&init, &init, &init); // 8
	rtl_add(cpu.val + 3, cpu.val + 3, &init);
	rtl_add(cpu.val + 3, cpu.val + 3, &init);
	rtl_add(cpu.val + 3, cpu.val + 3, &init);
	rtl_add(cpu.val + 3, cpu.val + 3, &init);
	rtl_add(cpu.val + 3, cpu.val + 3, &init);
}

void jump(){
	rtl_j(1000);
	decoding.state = JITState::Terminate;
}

void exec(void (*inst)()) {
	auto& state = decoding.state;
	auto& ori_eip = decoding.ori_eip;
	auto& executor = decoding.executor;
	
	switch(state){
		case JITState::Init:{
			
			if(auto query = executor.fetchFunction(0, ori_eip)){
				auto [func, expected_inst]  = query.value();
				auto real_inst = func((uint32_t*)&cpu, nullptr);
				return;
			}
			
			executor.begin_block(0, ori_eip);
			state = JITState::Compiling;
			break;
		}
		case JITState::Compiling:{
			break;
		}
		case JITState::Terminate:{
			assert(0);
		}
	}
	inst();
	if(state == JITState::Terminate){
		state = JITState::Init;
	}
}

int main() {
	using llvm::CodeExecutor;
	cpu.val[3] = 100;
	cpu.fuck[0] = 2;
	uint32_t cr3 = 0;
	uint32_t addr = 1000;
	decoding.ori_eip = 1000;
	CodeExecutor::initEnvironment();
	
	for(int i = 0; i < 100; ++i){
		exec(functor);
		exec(functor);
		exec(functor);	
		exec(jump);	
	}
	
	CodeExecutor::initEnvironment();
	CodeExecutor ce;
	(void) ce.getFunctorTy();
	return 0;
}
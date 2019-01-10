//
// Created by mike on 1/10/19.
//

#include "workflow.h"
#include <iostream>

CPUState cpu;

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
	*dest = *a + *b;
	
	auto& eng = decoding.executor;
	auto va = eng.get_value(a);
	auto vb = eng.get_value(b);
	auto vc = eng().CreateAdd(va, vb);
	eng.set_value(dest, vc);
}

void rtl_li(rtlreg_t *dest, uint32_t imm){
	*dest = imm;
	
	auto& eng = decoding.executor;
	auto vimm = eng().getInt32(imm);
	eng.set_value(dest, vimm);
}

void rtl_j(uint32_t imm){
	
	decoding.ori_eip = imm;
	decoding.state = JITState::Terminate;
}

void functor() {
	rtlreg_t init;
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
	assert(decoding.ori_eip == 1000 + 2);
	rtl_j(1000);
}

void exec(void (*inst)()) {
	auto state = decoding.state;
	auto& ori_eip = decoding.ori_eip;
	auto& eng = decoding.executor;
	
	switch(state){
		case JITState::Init:{
			assert(ori_eip == 1000);
			if(auto query = eng.fetchFunction(0, ori_eip)){
				auto [func, expected_inst]  = query.value();
				auto real_inst = func((uint32_t*)&cpu, nullptr);
				assert(expected_inst == real_inst);
				decoding.ori_eip = 1000;
				return;
			}
			
			eng.begin_block(0, ori_eip);
			decoding.state = JITState::Compiling;
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
	decoding.ori_eip += 1;
	eng.finish_inst();
	
	if(decoding.state == JITState::Terminate){
		state = JITState::Init;
		eng.finish_block();
	}
}

int main() {
	using llvm::CodeExecutor;
	CodeExecutor::InitEnvironment();
	decoding.executor.init();
	cpu.val[3] = 1000;
	cpu.fuck[0] = 2;
	uint32_t cr3 = 0;
	uint32_t addr = 1000;
	decoding.ori_eip = 1000;
	
	for(int i = 0; i < 100; ++i){
		assert(addr == 1000);
		exec(functor);
		exec(functor);
		exec(jump);
	}
	
	std::cout << cpu.val[3];
	return 0;
}
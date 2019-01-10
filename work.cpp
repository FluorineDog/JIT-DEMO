//
// Created by mike on 1/10/19.
//

#include "workflow.h"
#include <iostream>

using std::cout;

struct ExecState{
	int good;
	int seq_eip;
};

void rtl_add(rtlreg_t *dest, rtlreg_t *a, rtlreg_t *c);

void rtl_li(rtlreg_t *dest, uint32_t imm);

void rtl_j(uint32_t imm);



void functor() {
	rtlreg_t init;
	rtl_li(&init, 20);
	rtl_add(cpu.val + 3, &init, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
	rtl_add(cpu.val + 3, cpu.val + 3, cpu.fuck);
}

void jump(){
	rtl_j(1000);
}

void exec(void(*inst)()){
	
}

int main() {
	using llvm::CodeExecutor;
	cpu.val[3] = 100;
	cpu.fuck[0] = 2;
	uint32_t cr3 = 2333;
	uint32_t addr = 1000;
	for(int i = 0; i < 100; ++i){
	
	}
	
	
	CodeExecutor::initEnvironment();
	CodeExecutor ce;
	(void) ce.getFunctorTy();
	return 0;
}
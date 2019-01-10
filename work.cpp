//
// Created by mike on 1/10/19.
//

#include "workflow.h"
int main() {
	using llvm::CodeExecutor;
	CodeExecutor::initEnvironment();
	CodeExecutor ce;
	(void)ce.getFunctorTy();
	return 0;
}
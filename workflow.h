#pragma once

#include <optional>
#include "kjit.h"
#include <map>
#include <memory>
#include <unordered_map>
#include <set>
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Constant.h>

using rtlreg_t = uint32_t;

namespace llvm {

struct CPUState {
	uint32_t val[20];
	uint32_t fuck[20];
} cpu;

class CodeExecutor {
private:
	// cpu and memory
	using RawFT = int (*)(uint32_t *, char *);

public:
	CodeExecutor() : builder_(ctx_) {
		this->jit_ = cantFail(orc::KaleidoscopeJIT::Create());
	};
	
	static void initEnvironment() {
		LLVMInitializeNativeTarget();
		LLVMInitializeNativeAsmPrinter();
	}
	
	void prepareFunctionAt(uint32_t cr3, uint32_t vaddr) {
		// clear state;
		s_ = State();
		
		auto uid = get_uid(cr3, vaddr);
		auto name = get_name(uid);
		s_.mod = std::make_unique<Module>(name, ctx_);
		s_.func = cast<Function>(s_.mod->getOrInsertFunction(name, getFunctorTy()));
		s_.bb = BasicBlock::Create(ctx_, "entry", s_.func);
		builder_.SetInsertPoint(s_.bb);
		assert(s_.func->arg_size() == 2);
		auto iter = s_.func->arg_begin();
		s_.regfile = &*iter;
		++iter;
		s_.memory = &*iter;
	}
	
	LLVMContext &get_ctx() {
		return ctx_;
	}
	
	static std::optional<int> is_cpu(rtlreg_t *reg) {
		if ((rtlreg_t *) &cpu <= reg && reg < (rtlreg_t *) (&cpu + 1)) {
			return reg - (rtlreg_t *) &cpu;
		} else {
			return std::nullopt;
		}
	}
	
	
	void set_value(rtlreg_t *reg, Value *) {
		
	}
	
	Value *get_value(rtlreg_t *reg) {
		if (auto reg_id = is_cpu(reg)) {
			int id = reg_id.value();
			if (s_.reg_cache[id].first == nullptr) {
				auto reg_ptr = builder_.CreateConstGEP1_32(s_.regfile, id);
				// not dirty yet
				s_.reg_cache[id] = std::make_pair(builder_.CreateLoad(reg_ptr), false);
			}
			return s_.reg_cache[id].first;
		} else {
			assert(s_.value_cache.count(reg));
			return s_.value_cache[reg];
			
		}
	}
	
	void fetchFunctionAt(uint32_t cr3, uint32_t vaddr) {
		
	}
	
	Type *getRegTy() {
		return Type::getInt32Ty(ctx_);
	}
	
	
	FunctionType *getFunctorTy() {
		static FunctionType *functor_type = nullptr;
		if (!functor_type) {
//			auto&& params_type = ;
			functor_type = FunctionType::get(Type::getInt32Ty(ctx_),
					{getRegTy()->getPointerTo(), Type::getInt8PtrTy(ctx_)}, false);
		}
		return functor_type;
	}

private:
	static inline std::uint64_t get_uid(uint32_t cr3, uint32_t vaddr) {
		return ((uint64_t) cr3 << 32) | vaddr;
	}
	
	static inline std::string get_name(uint64_t uid) {
		return "functor_" + std::to_string(uid);
	}
	
	static inline std::string get_name(uint32_t cr3, uint32_t vaddr) {
		return get_name(get_uid(cr3, vaddr));
	}

private:
	std::unique_ptr<orc::KaleidoscopeJIT> jit_;
	LLVMContext ctx_;
	std::unordered_map<uint64_t, std::pair<RawFT *, int>> icache;
	
	struct State {
		std::unique_ptr<Module> mod;
		std::unordered_map<rtlreg_t *, Value *> value_cache;
		BasicBlock *bb;
		std::array<std::pair<Value *, bool>, sizeof(CPUState) / sizeof(rtlreg_t)> reg_cache;
		Function *func;
		Value *regfile;
		Value *memory;
	} s_;
	
	IRBuilder<> builder_;
};
	
}

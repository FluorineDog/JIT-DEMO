//===-- examples/HowToUseJIT/HowToUseJIT.cpp - An example use of the JIT --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This small program provides an example of how to quickly build a small
//  module with two functions and execute it with the JIT.
//
// Goal:
//  The goal of this snippet is to create in the memory
//  the LLVM module consisting of two functions as follow:
//
// int add1(int x) {
//   return x+1;
// }
//
// int foo() {
//   return add1(10);
// }
//
// then compile the module via JIT, then execute the `foo'
// function and return result to a driver, i.e. to a "host program".
//
// Some remarks and questions:
//
// - could we invoke some code using noname functions too?
//   e.g. evaluate "foo()+foo()" without fears to introduce
//   conflict of temporary function name with some real
//   existing function name?
//
//===----------------------------------------------------------------------===//
#include "kjit.h"
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

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

int fuck() {
    return 1000;
}

using namespace llvm;

int main() {
	LLVMContext Context;
    
    IRBuilder<> builder(Context);
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();


    // Create some module to put our function into it.
    std::unique_ptr<Module> Owner = make_unique<Module>("test", Context);
    Module *M = Owner.get();

    // Create the add1 function entry and insert this entry into module M.  The
    // function will have a return type of "int" and take an argument of "int".
    Function *Add1F = cast<Function>(M->getOrInsertFunction(
        "add1", Type::getInt32Ty(Context), Type::getInt32Ty(Context)));

    // Add a basic block to the function. As before, it automatically inserts
    // because of the last argument.
    BasicBlock *BB = BasicBlock::Create(Context, "EntryBlock", Add1F);
    builder.SetInsertPoint(BB);

    // Create a basic block builder with default parameters.  The builder will
    // automatically append instructions to the basic block `BB'.

    // Get pointers to the constant `1'.
    Value *One = builder.getInt32(1);

    // Get pointers to the integer argument of the add1 function...
    assert(Add1F->arg_begin() != Add1F->arg_end());    // Make sure there's an arg
    Argument *ArgX = &*Add1F->arg_begin();             // Get the arg
    ArgX->setName("AnArg");    // Give it a nice symbolic name for fun.

    // Create the add instruction, inserting it into the end of BB.
    Value *Add = builder.CreateAdd(One, ArgX);

    // Create the return instruction and add it to the basic block
    builder.CreateRet(Add);

    // Now, function add1 is ready.

    // Now we're going to create function `foo', which returns an int and takes no
    // arguments.
    auto FooF = cast<Function>(M->getOrInsertFunction("foo", Type::getInt32Ty(Context)));

    // Add a basic block to the FooF function.
    BB = BasicBlock::Create(Context, "EntryBlock", FooF);

    // Tell the basic block builder to attach itself to the new basic block
    builder.SetInsertPoint(BB);

    // Get pointer to the constant `10'.
    //	Value *Ten = builder.getInt32(10);
    {
        auto FT = FunctionType::get(builder.getInt32Ty(), {}, false);
        auto FuncAddr = builder.getInt64((size_t)fuck);
        auto F = builder.CreateIntToPtr(FuncAddr, FT->getPointerTo());
        auto val = builder.CreateCall(F);
        CallInst *Add1CallRes = builder.CreateCall(Add1F, val);
        Add1CallRes->setTailCall(true);
        builder.CreateRet(Add1CallRes);
    }
    outs() << *Owner;

    // Pass Ten to the call to Add1F

    // Create the return instruction and add it to the basic block.

    ///////////////////
    auto kjit_ = orc::KaleidoscopeJIT::Create();
    assert(kjit_);
    auto kjit = std::move(kjit_.get());
    assert(kjit);
    auto err = kjit->addModule(std::move(Owner));
    outs() << err;
    assert(!err);
    auto es = kjit->lookup("foo");
    assert(es);
    auto f = reinterpret_cast<int (*)()>(es->getAddress());
    outs() << "result = " << f();

    {
        auto mod = make_unique<Module>("wtf", Context);
        auto fb = cast<Function>(mod->getOrInsertFunction(
            "exec", Type::getInt32PtrTy(Context), Type::getInt32PtrTy(Context)));
        assert(fb);
        auto bb = BasicBlock::Create(Context, "body", fb);
        builder.SetInsertPoint(bb);
        auto arg1 = &*fb->arg_begin();
        auto valptr = builder.CreateConstGEP1_32(arg1, 0);
        auto val = builder.CreateLoad(valptr);
        builder.CreateStore(builder.CreateAdd(val, builder.getInt32(1)), valptr);
        builder.CreateRet(val);
        outs() << "---------------------";
        outs() << *mod;
        auto err = kjit->addModule(std::move(mod));
        assert(!err);
    }

    auto func =
        reinterpret_cast<int (*)(int *)>(cantFail(kjit->lookup("exec")).getAddress());
    int hhh[32] = {};
    outs() << func(hhh);
    outs() << func(hhh);
    outs() << func(hhh);
    outs() << func(hhh);
    outs() << func(hhh);
    outs() << hhh[0];
    return 0;
}

#ifndef PMGIO_H
#define PMGIO_H

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Assembly/Parser.h"
#include "llvm/Support/IRReader.h"

using namespace llvm;

Module *parseModule(std::string InputFileName,LLVMContext &Context);

#endif

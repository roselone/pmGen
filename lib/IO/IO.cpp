#include "IO.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/IRReader.h"
using namespace std;

Module *parseModule(std::string InputFileName,LLVMContext &Context){
	SMDiagnostic Err;
	Module *m=ParseIRFile(InputFileName,Err,Context);
	return m;
}


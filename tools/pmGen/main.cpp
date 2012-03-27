#include "test.h"
#include "IO.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Assembly/AssemblyAnnotationWriter.h"
#include "llvm/LLVMContext.h"
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/InlineAsm.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Operator.h"
#include "llvm/Module.h"
#include "llvm/ValueSymbolTable.h"
#include "llvm/TypeSymbolTable.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/FormattedStream.h"
int main (int argc, char ** argv)
{
	LLVMContext Context;
	std::string InputFileName;
	if (argc>1){
		InputFileName=argv[1];
	}
	else {
		errs() << "Please input the .bc file.\n";
		exit(1);
	}

	Module *m=parseModule(InputFileName,Context);
	outs()<<*m<<"\n-----------------------\n\n";

	Module::global_iterator begin=m->global_begin();
	Module::global_iterator end=m->global_end();
	TypePrinting TypePrinter;
	std::vector<const Type*> numberedTypes;
	//AddModuleTypesToPrinter(TypePrinter,numberedTypes,m);
	for (Module::global_iterator i=begin;i!=end;i++){
		outs()<<i->getName().str()<<' '<<i->getType()<<'\n';
		//TypePrinter.print(i->getType()->getElementType(),outs());
		i->print(outs(),0);
		outs()<<'\n';
	}

	const TypeSymbolTable &ST=m->getTypeSymbolTable();
	for (TypeSymbolTable::const_iterator TI=ST.begin(),E=ST.end();
			TI!=E;++TI){
		outs()<<TI->first<<' '<<TI->second<<'\n';
	}
	return 0;
}


#include "test.h"
#include "IO.h"
#include "TypeFinder.h"
#include "Helper.h"
#include "SlotTracker.h"
#include "FunctionGen.h"

#include "llvm/Support/FormattedStream.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/TypeSymbolTable.h"
/*
#include "llvm/DerivedTypes.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/ErrorHandling.h"
#include <cctype>

#include "llvm/ADT/DenseSet.h"
*/
using namespace llvm;

int main (int argc, char ** argv)
{
	LLVMContext Context;
	std::string InputFileName;
	std::string InitProc;
	raw_string_ostream initProc(InitProc);
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
	TypeGen TypeGener;
	SlotTracker SlotTable(m);
	ConStr conStr;
	std::vector<const Type*> numberedTypes;
	TypeFinder typeFinder(TypeGener,numberedTypes);
	typeFinder.Run(*m);
	formatted_raw_ostream OS(outs());

	TypeGener.gen(numberedTypes,m->getTypeSymbolTable(),outs());
	if (!m->global_empty()) outs()<<'\n';
	Helper::InitBE(initProc,true);
//	Helper::conStr=new std::map<StringRef,std::string>();	
	for (Module::const_global_iterator GI=m->global_begin(),GE=m->global_end();
			GI!=GE;++GI){
		if (!conStr.isConStr(GI)){
			TypeGener.print(GI->getType()->getElementType(),outs());
			outs()<<' ';	
			Helper::WriteAsOperandInternal(outs(),GI,&TypeGener,&SlotTable,GI->getParent());
			outs()<<";\n";
			if (GI->hasInitializer())
				Helper::InitGValue(initProc,GI,&TypeGener,&SlotTable,GI->getParent());
		}
		//TODO 初始化 init
	}

	FunctionGen functionGener(TypeGener,SlotTable,OS,m);
	for (Module::const_iterator FI=m->begin(),FE=m->end();FI!=FE;++FI){
		functionGener.printFunction(FI);
	}
	Helper::InitBE(initProc,false);
	outs()<<initProc.str();
	return 0;
}


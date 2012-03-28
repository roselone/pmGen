#include "test.h"
#include "IO.h"
#include "TypeFinder.h"

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
	std::vector<const Type*> numberedTypes;
	TypeFinder typeFinder(TypeGener,numberedTypes);
	typeFinder.Run(*m);
	for (Module::global_iterator i=begin;i!=end;i++){
		outs()<<i->getName().str()<<' '<<i->getType()<<'\n';
		//TypeGener.print(i->getType()->getElementType(),outs());
		i->print(outs(),0);
		i->getType()->getElementType()->print(outs());
		outs()<<'\n';
		TypeGener.print(i->getType()->getElementType(),outs(),0);
		outs()<<'\n';
	}

	const TypeSymbolTable &ST=m->getTypeSymbolTable();
	for (TypeSymbolTable::const_iterator TI=ST.begin(),E=ST.end();
			TI!=E;++TI){
		outs()<<TI->first<<' '<<TI->second<<'\n';
	}

	const Type *type;
	for (int i=0,e=numberedTypes.size();i!=e;++i){
		type=numberedTypes[i];
		if (type->isStructTy()) {
			TypeGener.printAtLeastOneLevel(type,outs());
		}
		outs()<<'\n';
	}
	return 0;
}


#ifndef PMG_HELPER_H
#define PMG_HELPER_H

#include <map>
#include <string>

namespace llvm {

	
enum PrefixType {
	GlobalPrefix,
	LabelPrefix,
	LocalPrefix,
	NoPrefix
};


class SlotTracker;
class Value;
class raw_ostream;
class TypeGen;
class Module;
class User;
class Constant;
class MDNode;
class StringRef;
class GlobalVariable;

class ConStr {
	std::map<StringRef,std::string> conStr;

public:
	ConStr(){conStr.empty();}	
	static ConStr *pConStr;
	bool isConStr(const GlobalVariable *V);
	bool isExist(const StringRef name);
	std::string getString(const StringRef name);
	ConStr *get();
};


class Helper {

public:
	static void test(raw_ostream &OS,StringRef Name);

	static SlotTracker *createSlotTracker(const Value *V);

	static const Module *getModuleFromVal(const Value *V);
	static void PrintLLVMName(raw_ostream &OS, StringRef Name, PrefixType Prefix); 

	static void PrintLLVMName(raw_ostream &OS, const Value *V);

	static void PrintEscapedString(StringRef Name,raw_ostream &Out);

	static void WriteAsOperandInternal(
			raw_ostream &Out, const Value *V,
			TypeGen *TypePrinter,
			SlotTracker *Machine,
			const Module *Context);

	static const char *getPredicateText(unsigned predicate);
	
	static void WriteOptimizationInfo(raw_ostream &Out, const User *U);	

	static void WriteConstantInternal(
			raw_ostream &Out,const Constant *CV,
			TypeGen &TypePrinter,
			SlotTracker *Machine,
			const Module *Context);

	static void WriteMDNodeBodyInternal(
			raw_ostream &Out,const MDNode *Node,
			TypeGen *TypePrinter,
			SlotTracker *Machine,
			const Module *Context);

	static void WriteAsOperand(
			raw_ostream &Out,const Value *V,
			bool PrintType,const Module *Context);

	static void InitBE(raw_ostream &Out,bool BorE);

	static void InitGValue(
			raw_ostream &Out,const GlobalVariable *GV,
			TypeGen *TypePrinter,
			SlotTracker *Machine,
			const Module *Context);
	
	static void Formatting(std::string &s);
};

}

#endif

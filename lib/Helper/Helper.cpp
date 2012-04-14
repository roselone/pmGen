#include "Helper.h"
#include "SlotTracker.h"
#include "TypeGen.h"
#include "TypeFinder.h"
#include "Define.h"

#include "llvm/Assembly/Writer.h"
#include "llvm/Module.h"
#include "llvm/DerivedTypes.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/InlineAsm.h"
#include "llvm/Operator.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;


// PrintEscapedString - Print each character of the specified string, escaping
// it if it is not printable or if it is an escape char.
void Helper::PrintEscapedString(StringRef Name, raw_ostream &Out) {
  for (unsigned i = 0, e = Name.size(); i != e; ++i) {
    unsigned char C = Name[i];
    if (isprint(C) && C != '\\' && C != '"')
      Out << C;
    else
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

void Helper::test(raw_ostream &OS,StringRef Name){
	OS<<Name.str();
}

/// PrintLLVMName - Turn the specified name into an 'LLVM name', which is either
/// prefixed with % (if the string only contains simple characters) or is
/// surrounded with ""'s (if it has special chars in it).  Print it out.
void Helper::PrintLLVMName(raw_ostream &OS, StringRef Name, PrefixType Prefix) {
	assert(Name.data() && "Cannot get empty name!");
	switch (Prefix) {
		default: llvm_unreachable("Bad prefix!");
		case NoPrefix: break;
		case GlobalPrefix: OS << '_'; break;
		case LabelPrefix:  break;
		case LocalPrefix:  break;
	}

	if (Name[0]>='0' && Name[0]<='9') OS << 'v';
	for (unsigned i=0,e=Name.size();i!=e;++i){
		char C=Name[i];
		if (!(C>='0' && C<='9') && !(C>='A' && C<='Z') && !(C>='a' && C<='z')){
			OS<<'_';
		}else OS<<C;
	}
	// Scan the name to see if it needs quotes first.
/*	bool NeedsQuotes = isdigit(Name[0]);
	if (!NeedsQuotes) {
		for (unsigned i = 0, e = Name.size(); i != e; ++i) {
			char C = Name[i];
			if (!isalnum(C) && C != '-' && C != '.' && C != '_') {
				NeedsQuotes = true;
				break;
			}
		}
	}
*/
	// If we didn't need any quotes, just write out the name in one blast.
//	if (!NeedsQuotes) {
//		OS << Name;
//		return;
//	}

	// Okay, we need quotes.  Output the quotes and escape any scary characters as
	// needed.
//	OS << '"';
//	PrintEscapedString(Name, OS);
//	OS << '"';
}

/// PrintLLVMName - Turn the specified name into an 'LLVM name', which is either
/// prefixed with % (if the string only contains simple characters) or is
/// surrounded with ""'s (if it has special chars in it).  Print it out.
void Helper::PrintLLVMName(raw_ostream &OS, const Value *V) {
	PrintLLVMName(OS, V->getName(),
                isa<GlobalValue>(V) ? GlobalPrefix : LocalPrefix);
}


const Module *Helper::getModuleFromVal(const Value *V) {
  if (const Argument *MA = dyn_cast<Argument>(V))
    return MA->getParent() ? MA->getParent()->getParent() : 0;

  if (const BasicBlock *BB = dyn_cast<BasicBlock>(V))
    return BB->getParent() ? BB->getParent()->getParent() : 0;

  if (const Instruction *I = dyn_cast<Instruction>(V)) {
    const Function *M = I->getParent() ? I->getParent()->getParent() : 0;
    return M ? M->getParent() : 0;
  }
  
  if (const GlobalValue *GV = dyn_cast<GlobalValue>(V))
    return GV->getParent();
  return 0;
}



SlotTracker *Helper::createSlotTracker(const Value *V) {
  if (const Argument *FA = dyn_cast<Argument>(V))
    return new SlotTracker(FA->getParent());

  if (const Instruction *I = dyn_cast<Instruction>(V))
    return new SlotTracker(I->getParent()->getParent());

  if (const BasicBlock *BB = dyn_cast<BasicBlock>(V))
    return new SlotTracker(BB->getParent());

  if (const GlobalVariable *GV = dyn_cast<GlobalVariable>(V))
    return new SlotTracker(GV->getParent());

  if (const GlobalAlias *GA = dyn_cast<GlobalAlias>(V))
    return new SlotTracker(GA->getParent());

  if (const Function *Func = dyn_cast<Function>(V))
    return new SlotTracker(Func);

  if (const MDNode *MD = dyn_cast<MDNode>(V)) {
    if (!MD->isFunctionLocal())
      return new SlotTracker(MD->getFunction());

    return new SlotTracker((Function *)0);
  }

  return 0;
}

const char *Helper::getPredicateText(unsigned predicate) {
  const char * pred = "unknown";
  switch (predicate) {
  case FCmpInst::FCMP_FALSE: pred = "false"; break;
  case FCmpInst::FCMP_OEQ:   pred = "=="; break;
  case FCmpInst::FCMP_OGT:   pred = ">"; break;
  case FCmpInst::FCMP_OGE:   pred = ">="; break;
  case FCmpInst::FCMP_OLT:   pred = "<"; break;
  case FCmpInst::FCMP_OLE:   pred = "<="; break;
  case FCmpInst::FCMP_ONE:   pred = "!="; break;
  case FCmpInst::FCMP_ORD:   pred = "ord"; break;
  case FCmpInst::FCMP_UNO:   pred = "uno"; break;
  case FCmpInst::FCMP_UEQ:   pred = "=="; break;
  case FCmpInst::FCMP_UGT:   pred = ">"; break;
  case FCmpInst::FCMP_UGE:   pred = ">="; break;
  case FCmpInst::FCMP_ULT:   pred = "<"; break;
  case FCmpInst::FCMP_ULE:   pred = "<="; break;
  case FCmpInst::FCMP_UNE:   pred = "!="; break;
  case FCmpInst::FCMP_TRUE:  pred = "true"; break;
  case ICmpInst::ICMP_EQ:    pred = "=="; break;
  case ICmpInst::ICMP_NE:    pred = "!="; break;
  case ICmpInst::ICMP_SGT:   pred = ">"; break;
  case ICmpInst::ICMP_SGE:   pred = ">="; break;
  case ICmpInst::ICMP_SLT:   pred = "<"; break;
  case ICmpInst::ICMP_SLE:   pred = "<="; break;
  case ICmpInst::ICMP_UGT:   pred = ">"; break;
  case ICmpInst::ICMP_UGE:   pred = ">="; break;
  case ICmpInst::ICMP_ULT:   pred = "<"; break;
  case ICmpInst::ICMP_ULE:   pred = "<="; break;
  }
  return pred;
}


void Helper::WriteOptimizationInfo(raw_ostream &Out, const User *U) {
  if (const OverflowingBinaryOperator *OBO =
        dyn_cast<OverflowingBinaryOperator>(U)) {
    if (OBO->hasNoUnsignedWrap())
      Out << " nuw";
    if (OBO->hasNoSignedWrap())
      Out << " nsw";
  } else if (const PossiblyExactOperator *Div =
               dyn_cast<PossiblyExactOperator>(U)) {
    if (Div->isExact())
      Out << " exact";
  } else if (const GEPOperator *GEP = dyn_cast<GEPOperator>(U)) {
    if (GEP->isInBounds())
      Out << " inbounds";
  }
}

void Helper::WriteConstantInternal(raw_ostream &Out, const Constant *CV,
                                  TypeGen &TypePrinter,
                                  SlotTracker *Machine,
                                  const Module *Context) {
  if (const ConstantInt *CI = dyn_cast<ConstantInt>(CV)) {
    if (CI->getType()->isIntegerTy(1)) {
      Out << (CI->getZExtValue() ? "true" : "false");
      return;
    }
    Out << CI->getValue();
    return;
  }

  if (const ConstantFP *CFP = dyn_cast<ConstantFP>(CV)) {
    if (&CFP->getValueAPF().getSemantics() == &APFloat::IEEEdouble ||
        &CFP->getValueAPF().getSemantics() == &APFloat::IEEEsingle) {
      // We would like to output the FP constant value in exponential notation,
      // but we cannot do this if doing so will lose precision.  Check here to
      // make sure that we only output it in exponential format if we can parse
      // the value back and get the same value.
      //
      bool ignored;
      bool isDouble = &CFP->getValueAPF().getSemantics()==&APFloat::IEEEdouble;
      double Val = isDouble ? CFP->getValueAPF().convertToDouble() :
                              CFP->getValueAPF().convertToFloat();
      SmallString<128> StrVal;
      raw_svector_ostream(StrVal) << Val;

      // Check to make sure that the stringized number is not some string like
      // "Inf" or NaN, that atof will accept, but the lexer will not.  Check
      // that the string matches the "[-+]?[0-9]" regex.
      //
      if ((StrVal[0] >= '0' && StrVal[0] <= '9') ||
          ((StrVal[0] == '-' || StrVal[0] == '+') &&
           (StrVal[1] >= '0' && StrVal[1] <= '9'))) {
        // Reparse stringized version!
        if (atof(StrVal.c_str()) == Val) {
          Out << StrVal.str();
          return;
        }
      }
      // Otherwise we could not reparse it to exactly the same value, so we must
      // output the string in hexadecimal format!  Note that loading and storing
      // floating point types changes the bits of NaNs on some hosts, notably
      // x86, so we must not use these types.
      assert(sizeof(double) == sizeof(uint64_t) &&
             "assuming that double is 64 bits!");
      char Buffer[40];
      APFloat apf = CFP->getValueAPF();
      // Floats are represented in ASCII IR as double, convert.
      if (!isDouble)
        apf.convert(APFloat::IEEEdouble, APFloat::rmNearestTiesToEven,
                          &ignored);
      Out << "0x" <<
              utohex_buffer(uint64_t(apf.bitcastToAPInt().getZExtValue()),
                            Buffer+40);
      return;
    }

    // Some form of long double.  These appear as a magic letter identifying
    // the type, then a fixed number of hex digits.
    Out << "0x";
    if (&CFP->getValueAPF().getSemantics() == &APFloat::x87DoubleExtended) {
      Out << 'K';
      // api needed to prevent premature destruction
      APInt api = CFP->getValueAPF().bitcastToAPInt();
      const uint64_t* p = api.getRawData();
      uint64_t word = p[1];
      int shiftcount=12;
      int width = api.getBitWidth();
      for (int j=0; j<width; j+=4, shiftcount-=4) {
        unsigned int nibble = (word>>shiftcount) & 15;
        if (nibble < 10)
          Out << (unsigned char)(nibble + '0');
        else
          Out << (unsigned char)(nibble - 10 + 'A');
        if (shiftcount == 0 && j+4 < width) {
          word = *p;
          shiftcount = 64;
          if (width-j-4 < 64)
            shiftcount = width-j-4;
        }
      }
      return;
    } else if (&CFP->getValueAPF().getSemantics() == &APFloat::IEEEquad)
      Out << 'L';
    else if (&CFP->getValueAPF().getSemantics() == &APFloat::PPCDoubleDouble)
      Out << 'M';
    else
      llvm_unreachable("Unsupported floating point type");
    // api needed to prevent premature destruction
    APInt api = CFP->getValueAPF().bitcastToAPInt();
    const uint64_t* p = api.getRawData();
    uint64_t word = *p;
    int shiftcount=60;
    int width = api.getBitWidth();
    for (int j=0; j<width; j+=4, shiftcount-=4) {
      unsigned int nibble = (word>>shiftcount) & 15;
      if (nibble < 10)
        Out << (unsigned char)(nibble + '0');
      else
        Out << (unsigned char)(nibble - 10 + 'A');
      if (shiftcount == 0 && j+4 < width) {
        word = *(++p);
        shiftcount = 64;
        if (width-j-4 < 64)
          shiftcount = width-j-4;
      }
    }
    return;
  }

  if (isa<ConstantAggregateZero>(CV)) {
    Out << "zeroinitializer";
    return;
  }
  
  if (const BlockAddress *BA = dyn_cast<BlockAddress>(CV)) {
    Out << "blockaddress(";
    WriteAsOperandInternal(Out, BA->getFunction(), &TypePrinter, Machine,
                           Context);
    Out << ", ";
    WriteAsOperandInternal(Out, BA->getBasicBlock(), &TypePrinter, Machine,
                           Context);
    Out << ")";
    return;
  }

  if (const ConstantArray *CA = dyn_cast<ConstantArray>(CV)) {
    // As a special case, print the array as a string if it is an array of
    // i8 with ConstantInt values.
    //
    const Type *ETy = CA->getType()->getElementType();
    if (CA->isString()) {
      Out << "c\"";
      PrintEscapedString(CA->getAsString(), Out);
      Out << '"';
    } else {                // Cannot output in string format...
      Out << '[';
      if (CA->getNumOperands()) {
        TypePrinter.print(ETy, Out);
        Out << ' ';
        WriteAsOperandInternal(Out, CA->getOperand(0),
                               &TypePrinter, Machine,
                               Context);
        for (unsigned i = 1, e = CA->getNumOperands(); i != e; ++i) {
          Out << ", ";
          TypePrinter.print(ETy, Out);
          Out << ' ';
          WriteAsOperandInternal(Out, CA->getOperand(i), &TypePrinter, Machine,
                                 Context);
        }
      }
      Out << ']';
    }
    return;
  }

  if (const ConstantStruct *CS = dyn_cast<ConstantStruct>(CV)) {
    if (CS->getType()->isPacked())
      Out << '<';
    Out << '{';
    unsigned N = CS->getNumOperands();
    if (N) {
      Out << ' ';
      TypePrinter.print(CS->getOperand(0)->getType(), Out);
      Out << ' ';

      WriteAsOperandInternal(Out, CS->getOperand(0), &TypePrinter, Machine,
                             Context);

      for (unsigned i = 1; i < N; i++) {
        Out << ", ";
        TypePrinter.print(CS->getOperand(i)->getType(), Out);
        Out << ' ';

        WriteAsOperandInternal(Out, CS->getOperand(i), &TypePrinter, Machine,
                               Context);
      }
      Out << ' ';
    }

    Out << '}';
    if (CS->getType()->isPacked())
      Out << '>';
    return;
  }

  if (const ConstantVector *CP = dyn_cast<ConstantVector>(CV)) {
    const Type *ETy = CP->getType()->getElementType();
    assert(CP->getNumOperands() > 0 &&
           "Number of operands for a PackedConst must be > 0");
    Out << '<';
    TypePrinter.print(ETy, Out);
    Out << ' ';
    WriteAsOperandInternal(Out, CP->getOperand(0), &TypePrinter, Machine,
                           Context);
    for (unsigned i = 1, e = CP->getNumOperands(); i != e; ++i) {
      Out << ", ";
      TypePrinter.print(ETy, Out);
      Out << ' ';
      WriteAsOperandInternal(Out, CP->getOperand(i), &TypePrinter, Machine,
                             Context);
    }
    Out << '>';
    return;
  }

  if (isa<ConstantPointerNull>(CV)) {
    Out << "null";
    return;
  }

  if (isa<UndefValue>(CV)) {
    Out << "undef";
    return;
  }

  if (const ConstantExpr *CE = dyn_cast<ConstantExpr>(CV)) {
    //Out << CE->getOpcodeName();
	Out << ' ';
    //WriteOptimizationInfo(Out, CE);
    //if (CE->isCompare())
    //  Out << ' ' << getPredicateText(CE->getPredicate());
	switch (CE->getOpcode()){
		case GetElementPtr:
			ConStr conStr;
			User::const_op_iterator OI=CE->op_begin();
			User::const_op_iterator OE=CE->op_end();
			bool isStruct=false;
			if (conStr.isExist(dyn_cast<Value>(*OI)->getName())){
				Out<<'"';
				std::string name=conStr.getString(dyn_cast<Value>(*OI)->getName());
				Helper::PrintEscapedString(name,Out);
				Out<<'"'<<' ';
				return ;
			}
			if (cast<PointerType>((*OI)->getType())->getElementType()->getTypeID()==Type::StructTyID) isStruct=true;
			WriteAsOperandInternal(Out,*OI,&TypePrinter,Machine,Context);
			const Type *type=cast<PointerType>((*OI)->getType())->getElementType();
			OI++;
			if (isStruct){
				if (dyn_cast<ConstantInt>(*OI)->getZExtValue()!=0){
					Out <<'[';
					WriteAsOperandInternal(Out,*OI,&TypePrinter,Machine,Context);
					Out <<']';
				}
			}else{
//				Out<<'[';
//				WriteAsOperandInternal(Out,*OI,&TypePrinter,Machine,Context);
//				Out<<']';
			}
			if (OI==OE) return ;
			for (++OI;OI!=OE;++OI){
				if (isStruct){
					const StructType *STY=cast<StructType>(type);
					unsigned num=dyn_cast<ConstantInt>(*OI)->getZExtValue();
					type=STY->getElementType(num);
					if (type->getTypeID()!=Type::StructTyID) isStruct=false;
					Out<<".u"<<num;				
				}else{
					const ArrayType *ATY = cast<ArrayType>(type);
					type=ATY->getElementType();
					if (type->getTypeID()==Type::StructTyID) isStruct=true;
					Out<<'[';
					WriteAsOperandInternal(Out,*OI,&TypePrinter,Machine,Context);
					Out<<']';
				}
			}

			return;
	}
	Out << CE->getOpcodeName();
    Out << " (";

    for (User::const_op_iterator OI=CE->op_begin(); OI != CE->op_end(); ++OI) {
      TypePrinter.print((*OI)->getType(), Out);
      Out << ' ';
      WriteAsOperandInternal(Out, *OI, &TypePrinter, Machine, Context);
      if (OI+1 != CE->op_end())
        Out << ", ";
    }

    if (CE->hasIndices()) {
      const SmallVector<unsigned, 4> &Indices = CE->getIndices();
      for (unsigned i = 0, e = Indices.size(); i != e; ++i)
        Out << ", " << Indices[i];
    }

    if (CE->isCast()) {
      Out << " to ";
      TypePrinter.print(CE->getType(), Out);
    }

    Out << ')';
    return;
  }

  Out << "<placeholder or erroneous Constant>";
}

void Helper::WriteMDNodeBodyInternal(raw_ostream &Out, const MDNode *Node,
                                    TypeGen *TypePrinter,
                                    SlotTracker *Machine,
                                    const Module *Context) {
  Out << "!{";
  for (unsigned mi = 0, me = Node->getNumOperands(); mi != me; ++mi) {
    const Value *V = Node->getOperand(mi);
    if (V == 0)
      Out << "null";
    else {
      TypePrinter->print(V->getType(), Out);
      Out << ' ';
      WriteAsOperandInternal(Out, Node->getOperand(mi), 
                             TypePrinter, Machine, Context);
    }
    if (mi + 1 != me)
      Out << ", ";
  }
  
  Out << "}";
}


/// WriteAsOperand - Write the name of the specified value out to the specified
/// ostream.  This can be useful when you just want to print int %reg126, not
/// the whole instruction that generated it.
///
void Helper::WriteAsOperandInternal(raw_ostream &Out, const Value *V,
                                   TypeGen *TypePrinter,
                                   SlotTracker *Machine,
                                   const Module *Context) {
  if (V->hasName()) {
    PrintLLVMName(Out, V);
    return;
  }

  const Constant *CV = dyn_cast<Constant>(V);
  if (CV && !isa<GlobalValue>(CV)) {
    assert(TypePrinter && "Constants require TypeGen!");
    WriteConstantInternal(Out, CV, *TypePrinter, Machine, Context);
    return;
  }

  if (const InlineAsm *IA = dyn_cast<InlineAsm>(V)) {
    Out << "asm ";
    if (IA->hasSideEffects())
      Out << "sideeffect ";
    if (IA->isAlignStack())
      Out << "alignstack ";
    Out << '"';
    PrintEscapedString(IA->getAsmString(), Out);
    Out << "\", \"";
    PrintEscapedString(IA->getConstraintString(), Out);
    Out << '"';
    return;
  }

  if (const MDNode *N = dyn_cast<MDNode>(V)) {
    if (N->isFunctionLocal()) {
      // Print metadata inline, not via slot reference number.
      WriteMDNodeBodyInternal(Out, N, TypePrinter, Machine, Context);
      return;
    }
  
    if (!Machine) {
      if (N->isFunctionLocal())
        Machine = new SlotTracker(N->getFunction());
      else
        Machine = new SlotTracker(Context);
    }
    int Slot = Machine->getMetadataSlot(N);
    if (Slot == -1)
      Out << "<badref>";
    else
      Out << '!' << Slot;
    return;
  }

  if (const MDString *MDS = dyn_cast<MDString>(V)) {
    Out << "!\"";
    PrintEscapedString(MDS->getString(), Out);
    Out << '"';
    return;
  }

  if (V->getValueID() == Value::PseudoSourceValueVal ||
      V->getValueID() == Value::FixedStackPseudoSourceValueVal) {
    V->print(Out);
    return;
  }

  char Prefix = 'v';
  int Slot;
  if (Machine) {
    if (const GlobalValue *GV = dyn_cast<GlobalValue>(V)) {
      Slot = Machine->getGlobalSlot(GV);
      Prefix = '_';
    } else {
      Slot = Machine->getLocalSlot(V);
    }
  } else {
    Machine = createSlotTracker(V);
    if (Machine) {
      if (const GlobalValue *GV = dyn_cast<GlobalValue>(V)) {
        Slot = Machine->getGlobalSlot(GV);
        Prefix = '@';
      } else {
        Slot = Machine->getLocalSlot(V);
      }
      delete Machine;
    } else {
      Slot = -1;
    }
  }

  if (Slot != -1)
    Out << Prefix << Slot;
  else
    Out << "<badref>";
}

void Helper::WriteAsOperand(raw_ostream &Out, const Value *V,
                          bool PrintType, const Module *Context) {

  // Fast path: Don't construct and populate a TypeGen object if we
  // won't be needing any types printed.
  if (!PrintType &&
      ((!isa<Constant>(V) && !isa<MDNode>(V)) ||
       V->hasName() || isa<GlobalValue>(V))) {
    WriteAsOperandInternal(Out, V, 0, 0, Context);
    return;
  }

  if (Context == 0) Context = getModuleFromVal(V);

  TypeGen TypePrinter;
  std::vector<const Type*> NumberedTypes;
  TypeFinder typeFinder(TypePrinter,NumberedTypes);
  typeFinder.Run(*Context);
  if (PrintType) {
    TypePrinter.print(V->getType(), Out);
    Out << ' ';
  }

  WriteAsOperandInternal(Out, V, &TypePrinter, 0, Context);
}

void Helper::InitBE(raw_ostream &Out,bool BorE){
	if (BorE){
		Out<<"init {\nchan _syn = [0] of { int };\n";
	}else Out << "  run _main(_syn);\n}\n";
	return ;
}

void Helper::InitGValue(raw_ostream &Out,const GlobalVariable *GV,TypeGen *TypePrinter,
		SlotTracker *Machine,const Module *Context){

	const Value *ini=GV->getInitializer();
	const Constant *CV=dyn_cast<Constant>(ini);
	std::string name=GV->getName();
	name="_"+name;

	if (const ConstantInt *CI=dyn_cast<ConstantInt>(CV)){
		Out<<"  "<<name<<" = "<<CI->getValue();
		Out << ";\n";
	}else if (const ConstantArray *CA=dyn_cast<ConstantArray>(CV)){
		for (unsigned i=0,e=CA->getNumOperands();i!=e;++i){
			const ConstantInt *T=dyn_cast<ConstantInt>(CA->getOperand(i));
			if (T){
				Out <<"  "<<name<<'['<<i<<']'<<" = "<<T->getValue();
				Out << ";\n";
			}
		}
	}else if (const ConstantStruct *CS=dyn_cast<ConstantStruct>(CV)){
		for (unsigned i=0,e=CS->getNumOperands();i!=e;++i){
			const ConstantInt *T=dyn_cast<ConstantInt>(CS->getOperand(i));
			if (T){
				Out <<"  "<<name<<".u"<<i<<" = "<<T->getValue();
				Out << ";\n";
			}
		}
	}
}

ConStr *ConStr::pConStr=new ConStr();

bool ConStr::isConStr(const GlobalVariable *V){
	if (V->hasInitializer()){
		const ConstantArray *CA=dyn_cast<ConstantArray>(V->getInitializer());
		if (CA && CA->isString()){
			get()->conStr.insert(std::pair<StringRef,std::string>(V->getName(),CA->getAsString()));
			return true;
		}
	}
	return false;
}

bool ConStr::isExist(const StringRef name){
	if (get()->conStr.find(name)!=get()->conStr.end()) return true;
	else return false;
}

std::string ConStr::getString(const StringRef name){
	return get()->conStr.find(name)->second;
}

ConStr *ConStr::get(){
	return ConStr::pConStr;
}

// Module level constructor. Causes the contents of the Module (sans functions)
// to be added to the slot table.
SlotTracker::SlotTracker(const Module *M)
  : TheModule(M), TheFunction(0), FunctionProcessed(false), 
    mNext(0), fNext(0),  mdnNext(0) {
}

// Function level constructor. Causes the contents of the Module and the one
// function provided to be added to the slot table.
SlotTracker::SlotTracker(const Function *F)
  : TheModule(F ? F->getParent() : 0), TheFunction(F), FunctionProcessed(false),
    mNext(0), fNext(0), mdnNext(0) {
}

inline void SlotTracker::initialize() {
  if (TheModule) {
    processModule();
    TheModule = 0; ///< Prevent re-processing next time we're called.
  }

  if (TheFunction && !FunctionProcessed)
    processFunction();
}

// Iterate through all the global variables, functions, and global
// variable initializers and create slots for them.
void SlotTracker::processModule() {

  // Add all of the unnamed global variables to the value table.
  for (Module::const_global_iterator I = TheModule->global_begin(),
         E = TheModule->global_end(); I != E; ++I) {
    if (!I->hasName())
      CreateModuleSlot(I);
  }

  // Add metadata used by named metadata.
  for (Module::const_named_metadata_iterator
         I = TheModule->named_metadata_begin(),
         E = TheModule->named_metadata_end(); I != E; ++I) {
    const NamedMDNode *NMD = I;
    for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i)
      CreateMetadataSlot(NMD->getOperand(i));
  }

  // Add all the unnamed functions to the table.
  for (Module::const_iterator I = TheModule->begin(), E = TheModule->end();
       I != E; ++I)
    if (!I->hasName())
      CreateModuleSlot(I);

}

// Process the arguments, basic blocks, and instructions  of a function.
void SlotTracker::processFunction() {
  fNext = 0;

  // Add all the function arguments with no names.
  for(Function::const_arg_iterator AI = TheFunction->arg_begin(),
      AE = TheFunction->arg_end(); AI != AE; ++AI)
    if (!AI->hasName())
      CreateFunctionSlot(AI);


  SmallVector<std::pair<unsigned, MDNode*>, 4> MDForInst;

  // Add all of the basic blocks and instructions with no names.
  for (Function::const_iterator BB = TheFunction->begin(),
       E = TheFunction->end(); BB != E; ++BB) {
    if (!BB->hasName())
      CreateFunctionSlot(BB);
    
    for (BasicBlock::const_iterator I = BB->begin(), E = BB->end(); I != E;
         ++I) {
      if (!I->getType()->isVoidTy() && !I->hasName())
        CreateFunctionSlot(I);
      
      // Intrinsics can directly use metadata.  We allow direct calls to any
      // llvm.foo function here, because the target may not be linked into the
      // optimizer.
      if (const CallInst *CI = dyn_cast<CallInst>(I)) {
        if (Function *F = CI->getCalledFunction())
          if (F->getName().startswith("llvm."))
            for (unsigned i = 0, e = I->getNumOperands(); i != e; ++i)
              if (MDNode *N = dyn_cast_or_null<MDNode>(I->getOperand(i)))
                CreateMetadataSlot(N);
      }

      // Process metadata attached with this instruction.
      I->getAllMetadata(MDForInst);
      for (unsigned i = 0, e = MDForInst.size(); i != e; ++i)
        CreateMetadataSlot(MDForInst[i].second);
      MDForInst.clear();
    }
  }

  FunctionProcessed = true;

}

/// Clean up after incorporating a function. This is the only way to get out of
/// the function incorporation state that affects get*Slot/Create*Slot. Function
/// incorporation state is indicated by TheFunction != 0.
void SlotTracker::purgeFunction() {
  fMap.clear(); // Simply discard the function level map
  TheFunction = 0;
  FunctionProcessed = false;
}

/// getGlobalSlot - Get the slot number of a global value.
int SlotTracker::getGlobalSlot(const GlobalValue *V) {
  // Check for uninitialized state and do lazy initialization.
  initialize();

  // Find the type plane in the module map
  ValueMap::iterator MI = mMap.find(V);
  return MI == mMap.end() ? -1 : (int)MI->second;
}

/// getMetadataSlot - Get the slot number of a MDNode.
int SlotTracker::getMetadataSlot(const MDNode *N) {
  // Check for uninitialized state and do lazy initialization.
  initialize();

  // Find the type plane in the module map
  mdn_iterator MI = mdnMap.find(N);
  return MI == mdnMap.end() ? -1 : (int)MI->second;
}


/// getLocalSlot - Get the slot number for a value that is local to a function.
int SlotTracker::getLocalSlot(const Value *V) {
  assert(!isa<Constant>(V) && "Can't get a constant or global slot with this!");

  // Check for uninitialized state and do lazy initialization.
  initialize();

  ValueMap::iterator FI = fMap.find(V);
  return FI == fMap.end() ? -1 : (int)FI->second;
}


/// CreateModuleSlot - Insert the specified GlobalValue* into the slot table.
void SlotTracker::CreateModuleSlot(const GlobalValue *V) {
  assert(V && "Can't insert a null Value into SlotTracker!");
  assert(!V->getType()->isVoidTy() && "Doesn't need a slot!");
  assert(!V->hasName() && "Doesn't need a slot!");

  unsigned DestSlot = mNext++;
  mMap[V] = DestSlot;

}

/// CreateSlot - Create a new slot for the specified value if it has no name.
void SlotTracker::CreateFunctionSlot(const Value *V) {
  assert(!V->getType()->isVoidTy() && !V->hasName() && "Doesn't need a slot!");

  unsigned DestSlot = fNext++;
  fMap[V] = DestSlot;

  // G = Global, F = Function, o = other
}

/// CreateModuleSlot - Insert the specified MDNode* into the slot table.
void SlotTracker::CreateMetadataSlot(const MDNode *N) {
  assert(N && "Can't insert a null Value into SlotTracker!");

  // Don't insert if N is a function-local metadata, these are always printed
  // inline.
  if (!N->isFunctionLocal()) {
    mdn_iterator I = mdnMap.find(N);
    if (I != mdnMap.end())
      return;

    unsigned DestSlot = mdnNext++;
    mdnMap[N] = DestSlot;
  }

  // Recursively add any MDNodes referenced by operands.
  for (unsigned i = 0, e = N->getNumOperands(); i != e; ++i)
    if (const MDNode *Op = dyn_cast_or_null<MDNode>(N->getOperand(i)))
      CreateMetadataSlot(Op);
}



#include "FunctionGen.h"
#include "Helper.h"
#include "TypeGen.h"
#include "SlotTracker.h"
#include "Define.h"

#include "llvm/Assembly/Writer.h"
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/LLVMContext.h"
#include "llvm/Operator.h"
#include "llvm/Module.h"
#include "llvm/ValueSymbolTable.h"
#include "llvm/TypeSymbolTable.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/CFG.h"
#include <algorithm>
#include <cctype>
#include <map>
#include <stdlib.h>

using namespace llvm;

/// printFunction - Print all aspects of a function.
///
void FunctionGen::printFunction(const Function *F) {
  // Print out the return type and name.
  if (F->isDeclaration()) return ;
  Out << "\nproctype ";
  gos.empty();
  count=0;
/*
  if (AnnotationWriter) AnnotationWriter->emitFunctionAnnot(F, Out);

  if (F->isMaterializable())
    Out << "; Materializable\n";

  if (F->isDeclaration())
    Out << "declare ";
  else
    Out << "define ";

  PrintLinkage(F->getLinkage(), Out);
  PrintVisibility(F->getVisibility(), Out);

  // Print the calling convention.
  switch (F->getCallingConv()) {
  case CallingConv::C: break;   // default
  case CallingConv::Fast:         Out << "fastcc "; break;
  case CallingConv::Cold:         Out << "coldcc "; break;
  case CallingConv::X86_StdCall:  Out << "x86_stdcallcc "; break;
  case CallingConv::X86_FastCall: Out << "x86_fastcallcc "; break;
  case CallingConv::X86_ThisCall: Out << "x86_thiscallcc "; break;
  case CallingConv::ARM_APCS:     Out << "arm_apcscc "; break;
  case CallingConv::ARM_AAPCS:    Out << "arm_aapcscc "; break;
  case CallingConv::ARM_AAPCS_VFP:Out << "arm_aapcs_vfpcc "; break;
  case CallingConv::MSP430_INTR:  Out << "msp430_intrcc "; break;
  case CallingConv::PTX_Kernel:   Out << "ptx_kernel "; break;
  case CallingConv::PTX_Device:   Out << "ptx_device "; break;
  default: Out << "cc" << F->getCallingConv() << " "; break;
  }
*/
  const FunctionType *FT = F->getFunctionType();
  const AttrListPtr &Attrs = F->getAttributes();
  Attributes RetAttrs = Attrs.getRetAttributes();
 // if (RetAttrs != Attribute::None)
 //   Out <<  Attribute::getAsString(Attrs.getRetAttributes()) << ' ';
  const Type *returnType=F->getReturnType();
 // TypeGener.print(F->getReturnType(), Out);
 // Out << ' ';
  Helper::WriteAsOperandInternal(Out, F, &TypeGener, &Machine, F->getParent());
  Out << '(';
  Machine.incorporateFunction(F);

  // Loop over the arguments, printing them...

  unsigned Idx = 1;
  if (!F->isDeclaration()) {
    // If this isn't a declaration, print the argument names as well.
    for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
         I != E; ++I) {
      // Insert commas as we go... the first arg doesn't get a comma
      if (I != F->arg_begin()) Out << "; ";
      printArgument(I, Attrs.getParamAttributes(Idx));
      Idx++;
    }
  } else {
    // Otherwise, print the types from the function type.
    for (unsigned i = 0, e = FT->getNumParams(); i != e; ++i) {
      // Insert commas as we go... the first arg doesn't get a comma
      if (i) Out << ", ";

      // Output type...
      TypeGener.print(FT->getParamType(i), Out);

      Attributes ArgAttrs = Attrs.getParamAttributes(i+1);
      if (ArgAttrs != Attribute::None)
        Out << ' ' << Attribute::getAsString(ArgAttrs);
    }
  }
  if (FT->getNumParams()>0) Out <<", ";
  if (returnType->isVoidTy()){
	Out <<"chan __syn";
  }else{
	  Out <<"chan __return";
  }

  // Finish printing arguments...
  if (FT->isVarArg()) {
    if (FT->getNumParams()) Out << ", ";
    Out << "...";  // Output varargs portion of signature!
  }
  Out << ')';
  /*
  if (F->hasUnnamedAddr())
    Out << " unnamed_addr";
  Attributes FnAttrs = Attrs.getFnAttributes();
  if (FnAttrs != Attribute::None)
    Out << ' ' << Attribute::getAsString(Attrs.getFnAttributes());
  if (F->hasSection()) {
    Out << " section \"";
	Helper::PrintEscapedString(F->getSection(), Out);
    Out << '"';
  }
  if (F->getAlignment())
    Out << " align " << F->getAlignment();
  if (F->hasGC())
    Out << " gc \"" << F->getGC() << '"';
	*/
  if (F->isDeclaration()) {
    Out << '\n';
  } else {
    Out << " {\n";
    // Output all of the function's basic blocks.
	
	for (Function::const_iterator I=F->begin(),E=F->end();I!=E;++I){
		if (I->hasName()){
			gos.insert(std::pair<StringRef,int>(I->getName(),count));
			count++;
		}
		for (BasicBlock::const_iterator BI = I->begin(),BE=I->end();BI!=BE;++BI){
			localValueDeclare(*BI);
		}
	}
	Out<<"int currentLabel;\n";
	
	Out<<"chan _syn = [0] of { int };\n";
//TODO map 优化
//
//
	retCount=0;
	for (Function::const_iterator I=F->begin(),E=F->end();I!=E;++I){
		for (BasicBlock::const_iterator BI=I->begin(),BE=I->end();BI!=BE;++BI){
			if (const CallInst *CI=dyn_cast<CallInst>(BI)){
			    const Value *Operand = CI->getCalledValue();
				const PointerType    *PTy = cast<PointerType>(Operand->getType());
				const FunctionType   *FTy = cast<FunctionType>(PTy->getElementType());
				const Type         *RTY = FTy->getReturnType();
				if (!RTY->isVoidTy() && Operand->getName()!="printf"){
					Out<<"chan _return"<<retCount<<" = [0] of { ";
					TypeGener.print(RTY,Out);
					Out<<" };\n";
					retCount++;
				}
			}
		}
	}
	retCount=0;

    for (Function::const_iterator I = F->begin(), E = F->end(); I != E; ++I){
		flag=false;
		printBasicBlock(I);
	}
	if (returnType->isVoidTy())
		Out <<"__syn!0\n";

	Out<<"LableSkip:skip\n";

    Out << "}\n";
  }

  Machine.purgeFunction();
}

/// printArgument - This member is called for every argument that is passed into
/// the function.  Simply print it out
///
void FunctionGen::printArgument(const Argument *Arg,
                                   Attributes Attrs) {
  // Output type...
  TypeGener.print(Arg->getType(), Out);

  // Output parameter attributes list
  if (Attrs != Attribute::None)
    Out << ' ' << Attribute::getAsString(Attrs);

  // Output name, if available...
  if (Arg->hasName()) {
    Out << ' ';
	Helper::PrintLLVMName(Out, Arg);
  }
}

/// printBasicBlock - This member is called for each basic block in a method.
///
void FunctionGen::printBasicBlock(const BasicBlock *BB) {
  if (BB->hasName()) {              // Print out the label if it exists...
    Out << "\n";
	Out << "Label"<<gos.find(BB->getName())->second<<':';
	//Helper::PrintLLVMName(Out, BB->getName(), LabelPrefix);
    //Out << ':';
  } else if (!BB->use_empty()) {      // Don't print block # of no uses...
    Out << "\n; <label>:";
    int Slot = Machine.getLocalSlot(BB);
    if (Slot != -1)
      Out << Slot;
    else
      Out << "<badref>";
  }

  if (BB->getParent() == 0) {
    Out.PadToColumn(50);
    Out << "; Error: Block without parent!";
  } else if (BB != &BB->getParent()->getEntryBlock()) {  // Not the entry block?
    // Output predecessors for the block.
    Out.PadToColumn(50);
    Out << ";";
    const_pred_iterator PI = pred_begin(BB), PE = pred_end(BB);

    if (PI == PE) {
      Out << " No predecessors!";
    } else {
      Out << " preds = ";
      writeOperand(*PI, false);
      for (++PI; PI != PE; ++PI) {
        Out << ", ";
        writeOperand(*PI, false);
      }
    }
  }

  Out << "\n";

  //if (AnnotationWriter) AnnotationWriter->emitBasicBlockStartAnnot(BB, Out);


  // Output all of the instructions in the basic block...
  for (BasicBlock::const_iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
    printInstruction(*I);
   // Out << '\n';
  }

  //if (AnnotationWriter) AnnotationWriter->emitBasicBlockEndAnnot(BB, Out);
}

void FunctionGen::localValueDeclare(const Instruction &I){
	if (!I.getType()->isVoidTy()){
		TypeGener.print(I.getType(),Out);
		Out << ' ';
		if (I.hasName()) {
			//Out <<I.getName();
			Helper::PrintLLVMName(Out, &I);
		} else if (!I.getType()->isVoidTy()) {
			// Print out the def slot taken.
			int SlotNum = Machine.getLocalSlot(&I);
			if (SlotNum == -1)
			Out << "<badref>";
		else
			Out << 'v' << SlotNum;
		}	
		Out<<";\n";	
	}
}

/// printInfoComment - Print a little comment after the instruction indicating
/// which slot it occupies.
///
/*
void FunctionGen::printInfoComment(const Value &V) {
  if (AnnotationWriter) {
    AnnotationWriter->printInfoComment(V, Out);
    return;
  }
}
*/
// This member is called for each Instruction in a function..
void FunctionGen::printInstruction(const Instruction &I) {
  //if (AnnotationWriter) AnnotationWriter->emitInstructionAnnot(&I, Out);

	std::string name;
	if (I.hasName()){
		name=I.getName().str();
		if (name[0]=='.') name[0]='_';
	}else if(!I.getType()->isVoidTy()){
		int SlotNum=Machine.getLocalSlot(&I);
		if (SlotNum==-1){
			name="<badref>";
		}else{
			char tmp[100];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"v%d",SlotNum);
			name=tmp;
		}
	}

	if (isa<PHINode>(I)) {
		Out << "if\n";

		for (unsigned op = 0, Eop = I.getNumOperands(); op < Eop; op += 2) {
			Out << "::";
			Out <<"(currentLabel == ";
			Out<<gos.find(I.getOperand(op+1)->getName())->second<<")->";
			Out<<name<<" = ";
			writeOperand(I.getOperand(op),false);
			Out<<"\n";
		}
		Out<<"fi\n";
		return ;
	}
	if (!flag){
		Out<<"currentLabel = "<<gos.find(I.getParent()->getName())->second<<'\n';
		flag=true;
	}

	if (isa<AllocaInst>(I)) return;
  // Print out indentation for an instruction.
  Out << "  ";
	if (const CallInst *CI = dyn_cast<CallInst>(&I)) {
	//	if (CI->getCalledValue()->getName()=="printf")
	Out <<"run ";
			goto flag1;
	}
  if (I.hasName() || !I.getType()->isVoidTy())
	Out <<name<<" = ";
flag1:
  // Print out name if it exists...
/*
  if (I.hasName()) {
	  Helper::PrintLLVMName(Out, &I);
    Out << " = ";
  } else if (!I.getType()->isVoidTy()) {
    // Print out the def slot taken.
    int SlotNum = Machine.getLocalSlot(&I);
    if (SlotNum == -1)
      Out << "<badref> = ";
    else
      Out << '%' << SlotNum << " = ";
  }
  */
/*
  // If this is a volatile load or store, print out the volatile marker.
  if ((isa<LoadInst>(I)  && cast<LoadInst>(I).isVolatile()) ||
      (isa<StoreInst>(I) && cast<StoreInst>(I).isVolatile())) {
      Out << "volatile ";
  } else if (isa<CallInst>(I) && cast<CallInst>(I).isTailCall()) {
    // If this is a call, check if it's a tail call.
    Out << "tail ";
  }
*/
  // Print out the opcode...
//  Out << I.getOpcodeName();
  std::string opcodeName = I.getOpcodeName();

  // Print out optimization information.
  //WriteOptimizationInfo(Out, &I);

  // Print out the compare instruction predicates
  // Print out the type of the operands...
  const Value *Operand = I.getNumOperands() ? I.getOperand(0) : 0;

  // Special case conditional branches to swizzle the condition out to the front
  if (isa<BranchInst>(I) && cast<BranchInst>(I).isConditional()) {
    BranchInst &BI(cast<BranchInst>(I));
    Out << "if\n::(";
    writeOperand(BI.getCondition(), false);
    Out << "!= 0) -> goto Label";
	Out << gos.find(BI.getSuccessor(0)->getName())->second;
    //writeOperand(BI.getSuccessor(0), false);
    Out << "\n::(";
	writeOperand(BI.getCondition(), false);
	Out << "==0) -> goto Label";
	Out << gos.find(BI.getSuccessor(1)->getName())->second;
    //writeOperand(BI.getSuccessor(1), false);
	Out << "\nfi";

  } else if (isa<SwitchInst>(I)) {
    // Special case switch instruction to get formatting nice and correct.
    Out << ' ';
    writeOperand(Operand        , true);
    Out << ", ";
    writeOperand(I.getOperand(1), true);
    Out << " [";

    for (unsigned op = 2, Eop = I.getNumOperands(); op < Eop; op += 2) {
      Out << "\n    ";
      writeOperand(I.getOperand(op  ), true);
      Out << ", ";
      writeOperand(I.getOperand(op+1), true);
    }
    Out << "\n  ]";
  } else if (isa<IndirectBrInst>(I)) {
    // Special case indirectbr instruction to get formatting nice and correct.
    Out << ' ';
    writeOperand(Operand, true);
    Out << ", [";
    
    for (unsigned i = 1, e = I.getNumOperands(); i != e; ++i) {
      if (i != 1)
        Out << ", ";
      writeOperand(I.getOperand(i), true);
    }
    Out << ']';
  } else if (isa<PHINode>(I)) {
    Out << ' ';
    TypeGener.print(I.getType(), Out);
    Out << ' ';

    for (unsigned op = 0, Eop = I.getNumOperands(); op < Eop; op += 2) {
      if (op) Out << ", ";
      Out << "[ ";
      writeOperand(I.getOperand(op  ), false); Out << ", ";
      writeOperand(I.getOperand(op+1), false); Out << " ]";
    }
  } else if (const ExtractValueInst *EVI = dyn_cast<ExtractValueInst>(&I)) {
    Out << ' ';
    writeOperand(I.getOperand(0), true);
    for (const unsigned *i = EVI->idx_begin(), *e = EVI->idx_end(); i != e; ++i)
      Out << ", " << *i;
  } else if (const InsertValueInst *IVI = dyn_cast<InsertValueInst>(&I)) {
    Out << ' ';
    writeOperand(I.getOperand(0), true); Out << ", ";
    writeOperand(I.getOperand(1), true);
    for (const unsigned *i = IVI->idx_begin(), *e = IVI->idx_end(); i != e; ++i)
      Out << ", " << *i;
  } else if (isa<ReturnInst>(I) && !Operand) {
    Out << " void";
  } else if (const CallInst *CI = dyn_cast<CallInst>(&I)) {
    // Print the calling convention being used.
    switch (CI->getCallingConv()) {
    case CallingConv::C: break;   // default
    case CallingConv::Fast:  Out << " fastcc"; break;
    case CallingConv::Cold:  Out << " coldcc"; break;
    case CallingConv::X86_StdCall:  Out << " x86_stdcallcc"; break;
    case CallingConv::X86_FastCall: Out << " x86_fastcallcc"; break;
    case CallingConv::X86_ThisCall: Out << " x86_thiscallcc"; break;
    case CallingConv::ARM_APCS:     Out << " arm_apcscc "; break;
    case CallingConv::ARM_AAPCS:    Out << " arm_aapcscc "; break;
    case CallingConv::ARM_AAPCS_VFP:Out << " arm_aapcs_vfpcc "; break;
    case CallingConv::MSP430_INTR:  Out << " msp430_intrcc "; break;
    case CallingConv::PTX_Kernel:   Out << " ptx_kernel"; break;
    case CallingConv::PTX_Device:   Out << " ptx_device"; break;
    default: Out << " cc" << CI->getCallingConv(); break;
    }

    Operand = CI->getCalledValue();
    const PointerType    *PTy = cast<PointerType>(Operand->getType());
    const FunctionType   *FTy = cast<FunctionType>(PTy->getElementType());
    const Type         *RetTy = FTy->getReturnType();
    const AttrListPtr &PAL = CI->getAttributes();

//    if (PAL.getRetAttributes() != Attribute::None)
//      Out << ' ' << Attribute::getAsString(PAL.getRetAttributes());

    // If possible, print out the short form of the call instruction.  We can
    // only do this if the first argument is a pointer to a nonvararg function,
    // and if the return type is not a pointer to a function.
    //
	std::string name2=Operand->getName();
	if (name2=="printf"){
		Out<<' '<<name2;
	}else{
    Out << ' ';
    if (!FTy->isVarArg() &&
        (!RetTy->isPointerTy() ||
         !cast<PointerType>(RetTy)->getElementType()->isFunctionTy())) {
     // TypeGener.print(RetTy, Out);
     // Out << ' ';
      writeOperand(Operand, false);
    } else {
      writeOperand(Operand, false);
    }
	}
    Out << '(';
    for (unsigned op = 0, Eop = CI->getNumArgOperands(); op < Eop; ++op) {
      if (op > 0)
        Out << ", ";
	  writeParamOperand(CI->getArgOperand(op), PAL.getParamAttributes(op + 1));
    }
	if (name2!="printf"){
		Out<<", ";
		if (RetTy->isVoidTy())
			Out<<"_syn";
		else Out <<"_return"<<retCount;
	}
    Out << ')';
	if (name2!="printf") {
		if (RetTy->isVoidTy()){
			Out << "\n_syn?0;";
		}else{
			Out << "\n_return"<<retCount<<"?"<<name<<';';
			retCount++;
		}
	}
//   if (PAL.getFnAttributes() != Attribute::None)
//     Out << ' ' << Attribute::getAsString(PAL.getFnAttributes());
  } else if (const InvokeInst *II = dyn_cast<InvokeInst>(&I)) {
    Operand = II->getCalledValue();
    const PointerType    *PTy = cast<PointerType>(Operand->getType());
    const FunctionType   *FTy = cast<FunctionType>(PTy->getElementType());
    const Type         *RetTy = FTy->getReturnType();
    const AttrListPtr &PAL = II->getAttributes();

    // Print the calling convention being used.
    switch (II->getCallingConv()) {
    case CallingConv::C: break;   // default
    case CallingConv::Fast:  Out << " fastcc"; break;
    case CallingConv::Cold:  Out << " coldcc"; break;
    case CallingConv::X86_StdCall:  Out << " x86_stdcallcc"; break;
    case CallingConv::X86_FastCall: Out << " x86_fastcallcc"; break;
    case CallingConv::X86_ThisCall: Out << " x86_thiscallcc"; break;
    case CallingConv::ARM_APCS:     Out << " arm_apcscc "; break;
    case CallingConv::ARM_AAPCS:    Out << " arm_aapcscc "; break;
    case CallingConv::ARM_AAPCS_VFP:Out << " arm_aapcs_vfpcc "; break;
    case CallingConv::MSP430_INTR:  Out << " msp430_intrcc "; break;
    case CallingConv::PTX_Kernel:   Out << " ptx_kernel"; break;
    case CallingConv::PTX_Device:   Out << " ptx_device"; break;
    default: Out << " cc" << II->getCallingConv(); break;
    }

    if (PAL.getRetAttributes() != Attribute::None)
      Out << ' ' << Attribute::getAsString(PAL.getRetAttributes());

    // If possible, print out the short form of the invoke instruction. We can
    // only do this if the first argument is a pointer to a nonvararg function,
    // and if the return type is not a pointer to a function.
    //
    Out << ' ';
    if (!FTy->isVarArg() &&
        (!RetTy->isPointerTy() ||
         !cast<PointerType>(RetTy)->getElementType()->isFunctionTy())) {
      TypeGener.print(RetTy, Out);
      Out << ' ';
      writeOperand(Operand, false);
    } else {
      writeOperand(Operand, true);
    }
    Out << '(';
    for (unsigned op = 0, Eop = II->getNumArgOperands(); op < Eop; ++op) {
      if (op)
        Out << ", ";
	  writeParamOperand(II->getArgOperand(op), PAL.getParamAttributes(op + 1));
    }

    Out << ')';
    if (PAL.getFnAttributes() != Attribute::None)
      Out << ' ' << Attribute::getAsString(PAL.getFnAttributes());

    Out << "\n          to ";
    writeOperand(II->getNormalDest(), true);
    Out << " unwind ";
    writeOperand(II->getUnwindDest(), true);

  } else if (isa<CastInst>(I)) {
		if (Operand){
			Out << ' ';
			writeOperand(Operand,false);
		}
	/*  
    if (Operand) {
      Out << ' ';
      writeOperand(Operand, true);   // Work with broken code
    }
    Out << " to ";
    TypeGener.print(I.getType(), Out);
	*/
  } else if (isa<VAArgInst>(I)) {
    if (Operand) {
      Out << ' ';
      writeOperand(Operand, true);   // Work with broken code
    }
    Out << ", ";
    TypeGener.print(I.getType(), Out);
  } else if (Operand) {   // Print the normal way.

    // PrintAllTypes - Instructions who have operands of all the same type
    // omit the type from all but the first operand.  If the instruction has
    // different type operands (for example br), then they are all printed.
/*
	bool PrintAllTypes = false;
    const Type *TheType = Operand->getType();

    // Select, Store and ShuffleVector always print all types.
    if (isa<SelectInst>(I) || isa<StoreInst>(I) || isa<ShuffleVectorInst>(I)
        || isa<ReturnInst>(I)) {
      PrintAllTypes = true;
    } else {
      for (unsigned i = 1, E = I.getNumOperands(); i != E; ++i) {
        Operand = I.getOperand(i);
        // note that Operand shouldn't be null, but the test helps make dump()
        // more tolerant of malformed IR
        if (Operand && Operand->getType() != TheType) {
          PrintAllTypes = true;    // We have differing types!  Print them all!
          break;
        }
      }
    }

    if (!PrintAllTypes) {
      Out << ' ';
      TypeGener.print(TheType, Out);
    }
*/
	Out << ' ';
	if (isa<SelectInst>(I)){
		Out << '(';
		writeOperand(I.getOperand(0),false);
		Out << "!=0 -> ";
		writeOperand(I.getOperand(1),false);
		Out << " : ";
		writeOperand(I.getOperand(2),false);
		Out << ")";
	}else if (isa<StoreInst>(I)){
		writeOperand(I.getOperand(1),false);
		Out << " = ";
		writeOperand(I.getOperand(0),false);
	}else  if (const CmpInst *CI = dyn_cast<CmpInst>(&I)){
		Out << '(';
		writeOperand(I.getOperand(0),false);
		Out << ' ' << Helper::getPredicateText(CI->getPredicate())<<' ';
		writeOperand(I.getOperand(1),false);
		Out << ')';
	}else if (const BinaryOperator *BI=dyn_cast<BinaryOperator>(&I)){
		writeOperand(I.getOperand(0),false);
		switch (BI->getOpcode()){
			case Add:
			case FAdd:Out<<" + ";break;
			case Sub:
			case FSub:Out << " - ";break;
			case Mul:
			case FMul:Out << " * "; break;
			case UDiv:
			case SDiv:
			case FDiv:Out << " / ";break;
			case URem:
			case SRem:
			case FRem:Out << " % ";break;
			case Shl:Out << " << ";break;
			case LShr:
			case AShr:Out << " >> ";break;
			case And:Out << " & ";break;
			case Or:Out << " | ";break;
			case Xor:Out<< " ^ ";break;
		}
		writeOperand(I.getOperand(1),false);
	}else if (isa<ReturnInst>(I)){
		Out <<"__return!";
		writeOperand(I.getOperand(0),false);
		Out << "\ngoto LabelSkip;";
	}else{
		for (unsigned i = 0, E = I.getNumOperands(); i != E; ++i) {
			if (i) Out << ",[other] ";
			writeOperand(I.getOperand(i), false);
		}	
	}
  }
	Out << '\n';
  // Print post operand alignment for load/store.
/*
  if (isa<LoadInst>(I) && cast<LoadInst>(I).getAlignment()) {
    Out << ", align " << cast<LoadInst>(I).getAlignment();
  } else if (isa<StoreInst>(I) && cast<StoreInst>(I).getAlignment()) {
    Out << ", align " << cast<StoreInst>(I).getAlignment();
  }

  // Print Metadata info.
  SmallVector<std::pair<unsigned, MDNode*>, 4> InstMD;
  I.getAllMetadata(InstMD);
  if (!InstMD.empty()) {
    SmallVector<StringRef, 8> MDNames;
    I.getType()->getContext().getMDKindNames(MDNames);
    for (unsigned i = 0, e = InstMD.size(); i != e; ++i) {
      unsigned Kind = InstMD[i].first;
       if (Kind < MDNames.size()) {
         Out << ", !" << MDNames[Kind];
      } else {
        Out << ", !<unknown kind #" << Kind << ">";
      }
      Out << ' ';
	  Helper::WriteAsOperandInternal(Out, InstMD[i].second, &TypeGener, &Machine,
                             TheModule);
    }
  }
*/
  //printInfoComment(I);
}

void FunctionGen::writeOperand(const Value *Operand, bool PrintType) {
  if (Operand == 0) {
    Out << "<null operand!>";
    return;
  }
  if (PrintType) {
    TypeGener.print(Operand->getType(), Out);
    Out << ' ';
  }
  Helper::WriteAsOperandInternal(Out, Operand, &TypeGener, &Machine, TheModule);
}

void FunctionGen::writeParamOperand(const Value *Operand,
                                       Attributes Attrs) {
  if (Operand == 0) {
    Out << "<null operand!>";
    return;
  }

  // Print the type
//  TypeGener.print(Operand->getType(), Out);
  // Print parameter attributes list
//  if (Attrs != Attribute::None)
//    Out << ' ' << Attribute::getAsString(Attrs);
//  Out << ' ';
  // Print the operand
  Helper::WriteAsOperandInternal(Out, Operand, &TypeGener, &Machine, TheModule);
}



#include "Helper.h"
#include "TypeFinder.h"
#include "TypeGen.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/TypeSymbolTable.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
#include <cctype>

using namespace llvm;

void TypeFinder::Run(const Module &M) {

	AddModuleTypesToPrinter(TP,&M);

    // Get types from the type symbol table.  This gets opaque types referened
    // only through derived named types.
    const TypeSymbolTable &ST = M.getTypeSymbolTable();
    for (TypeSymbolTable::const_iterator TI = ST.begin(), E = ST.end();
           TI != E; ++TI)
		IncorporateType(TI->second);

    // Get types from global variables.
	for (Module::const_global_iterator I = M.global_begin(),
           E = M.global_end(); I != E; ++I) {
        IncorporateType(I->getType());
        if (I->hasInitializer())
          IncorporateValue(I->getInitializer());
    }

    // Get types from aliases.
    for (Module::const_alias_iterator I = M.alias_begin(),
         E = M.alias_end(); I != E; ++I) {
		IncorporateType(I->getType());
        IncorporateValue(I->getAliasee());
    }

    // Get types from functions.
    for (Module::const_iterator FI = M.begin(), E = M.end(); FI != E; ++FI) {
        IncorporateType(FI->getType());

		for (Function::const_iterator BB = FI->begin(), E = FI->end();
             BB != E;++BB)
			for (BasicBlock::const_iterator II = BB->begin(),
               E = BB->end(); II != E; ++II) {
				const Instruction &I = *II;
				// Incorporate the type of the instruction and all its operands.
				IncorporateType(I.getType());
				for (User::const_op_iterator OI = I.op_begin(), OE = I.op_end();
					OI != OE; ++OI)
					IncorporateValue(*OI);
			}
      }
}

void TypeFinder::IncorporateType(const Type *Ty) {
    // Check to see if we're already visited this type.
	if (!VisitedTypes.insert(Ty).second)
		return;

    // If this is a structure or opaque type, add a name for the type.
    if (((Ty->isStructTy() && cast<StructType>(Ty)->getNumElements())
        || Ty->isOpaqueTy()) && !TP.hasTypeName(Ty)) {
		TP.addTypeName(Ty, "%"+utostr(unsigned(NumberedTypes.size())));
		NumberedTypes.push_back(Ty);
    }

    // Recursively walk all contained types.
    for (Type::subtype_iterator I = Ty->subtype_begin(),
        E = Ty->subtype_end(); I != E; ++I)
        IncorporateType(*I);
}

/// IncorporateValue - This method is used to walk operand lists finding
/// types hiding in constant expressions and other operands that won't be
/// walked in other ways.  GlobalValues, basic blocks, instructions, and
/// inst operands are all explicitly enumerated.
void TypeFinder::IncorporateValue(const Value *V) {
    if (V == 0 || !isa<Constant>(V) || isa<GlobalValue>(V)) return;

    // Already visited?
    if (!VisitedConstants.insert(V).second)
		return;

    // Check this type.
    IncorporateType(V->getType());

    // Look in operands for types.
    const Constant *C = cast<Constant>(V);
    for (Constant::const_op_iterator I = C->op_begin(),
         E = C->op_end(); I != E;++I)
		IncorporateValue(*I);
}

void TypeFinder::AddModuleTypesToPrinter(TypeGen &TP,
                             const Module *M) {
	if (M == 0) return;

	// If the module has a symbol table, take all global types and stuff their
	// names into the TypeNames map.
	const TypeSymbolTable &ST = M->getTypeSymbolTable();
	for (TypeSymbolTable::const_iterator TI = ST.begin(), E = ST.end();
       TI != E; ++TI) {
		const Type *Ty = cast<Type>(TI->second);

		// As a heuristic, don't insert pointer to primitive types, because
		// they are used too often to have a single useful name.
		if (const PointerType *PTy = dyn_cast<PointerType>(Ty)) {
			const Type *PETy = PTy->getElementType();
		if ((PETy->isPrimitiveType() || PETy->isIntegerTy()) &&
			  !PETy->isOpaqueTy())
			continue;
		}

		// Likewise don't insert primitives either.
		if (Ty->isIntegerTy() || Ty->isPrimitiveType())
			continue;

		// Get the name as a string and insert it into TypeNames.
		std::string NameStr;
		raw_string_ostream NameROS(NameStr);
		formatted_raw_ostream NameOS(NameROS);
		Helper::PrintLLVMName(NameOS, TI->first, LocalPrefix);
		NameOS.flush();
		TP.addTypeName(Ty, NameStr);
	}
}

static DenseMap<const Type *, std::string> &getTypeNamesMap(void *M) {
	return *static_cast<DenseMap<const Type *, std::string>*>(M);
}

void TypeGen::clear() {
	getTypeNamesMap(TypeNames).clear();
}

bool TypeGen::hasTypeName(const Type *Ty) const {
	return getTypeNamesMap(TypeNames).count(Ty);
}

void TypeGen::addTypeName(const Type *Ty, const std::string &N) {
	getTypeNamesMap(TypeNames).insert(std::make_pair(Ty, N));
}


TypeGen::TypeGen() {
	TypeNames = new DenseMap<const Type *, std::string>();
}

TypeGen::~TypeGen() {
	delete &getTypeNamesMap(TypeNames);
}

/// CalcTypeName - Write the specified type to the specified raw_ostream, making
/// use of type names or up references to shorten the type name where possible.
void TypeGen::CalcTypeName(
		const Type *Ty,
		SmallVectorImpl<const Type *> &TypeStack,
		raw_ostream &OS, bool IgnoreTopLevelName) {
	// Check to see if the type is named.
	if (!IgnoreTopLevelName) {
		DenseMap<const Type *, std::string> &TM = getTypeNamesMap(TypeNames);
		DenseMap<const Type *, std::string>::iterator I = TM.find(Ty);
		if (I != TM.end()) {
			OS << I->second;
			return;
		}
	}

	// Check to see if the Type is already on the stack...
	unsigned Slot = 0, CurSize = TypeStack.size();
	while (Slot < CurSize && TypeStack[Slot] != Ty) ++Slot; // Scan for type

	// This is another base case for the recursion.  In this case, we know
	// that we have looped back to a type that we have previously visited.
	// Generate the appropriate upreference to handle this.
	if (Slot < CurSize) {
		OS << '\\' << unsigned(CurSize-Slot);     // Here's the upreference
		return;
	}

	TypeStack.push_back(Ty);    // Recursive case: Add us to the stack..

	switch (Ty->getTypeID()) {
		case Type::VoidTyID:      OS << "void"; break;
		case Type::FloatTyID:					break;
		case Type::DoubleTyID:					break;
		case Type::X86_FP80TyID:				break;
		case Type::FP128TyID:					break;
		case Type::PPC_FP128TyID:				break;
		case Type::LabelTyID:     OS << "label"; break;
		case Type::MetadataTyID:  OS << "metadata"; break;
		case Type::X86_MMXTyID:					break;
		case Type::IntegerTyID: {
			unsigned bitWidth=cast<IntegerType>(Ty)->getBitWidth();
			switch (bitWidth){
				case 1:			  OS << "bit";  break;
				case 8:			  OS << "byte"; break;
				case 16:		  OS << "short";break;
				case 32:		  OS << "int";  break;
				default:						break;
			}
			break;
		}
		case Type::FunctionTyID: {
	const FunctionType *FTy = cast<FunctionType>(Ty);
    CalcTypeName(FTy->getReturnType(), TypeStack, OS);
    OS << " (";
    for (FunctionType::param_iterator I = FTy->param_begin(),
         E = FTy->param_end(); I != E; ++I) {
      if (I != FTy->param_begin())
        OS << ", ";
      CalcTypeName(*I, TypeStack, OS);
    }
    if (FTy->isVarArg()) {
      if (FTy->getNumParams()) OS << ", ";
      OS << "...";
    }
    OS << ')';
    break;
  }
		case Type::StructTyID: {
			const StructType *STy = cast<StructType>(Ty);
			OS << "TYPEDEF ";
			CalcTypeName(Ty,TypeStack,OS);
			OS << " {";
			int i=0;
			for (StructType::element_iterator I = STy->element_begin(),
					E = STy->element_end(); I != E; ++I) {
				OS << ' ';
				CalcTypeName(*I, TypeStack, OS);
				OS << " u"<<i;
				i++;
				if (llvm::next(I) == STy->element_end())
				OS << ' ';
				else
				OS << ';';
			}
			OS << '}';
			break;
		}
  case Type::PointerTyID: {
    const PointerType *PTy = cast<PointerType>(Ty);
    CalcTypeName(PTy->getElementType(), TypeStack, OS);
    if (unsigned AddressSpace = PTy->getAddressSpace())
      OS << " addrspace(" << AddressSpace << ')';
    OS << '*';
    break;
  }
		case Type::ArrayTyID: {
			const ArrayType *ATy = cast<ArrayType>(Ty);
			CalcTypeName(ATy->getElementType(),TypeStack,OS);
			OS << '[' << ATy->getNumElements() << ']';
			break;
		}

  case Type::VectorTyID: {
    const VectorType *PTy = cast<VectorType>(Ty);
    OS << "<" << PTy->getNumElements() << " x ";
    CalcTypeName(PTy->getElementType(), TypeStack, OS);
    OS << '>';
    break;
  }
  case Type::OpaqueTyID:
    OS << "opaque";
    break;
  default:
    OS << "<unrecognized-type>";
    break;
  }

  TypeStack.pop_back();       // Remove self from stack.
}

/// printTypeInt - The internal guts of printing out a type that has a
/// potentially named portion.
///
void TypeGen::print(const Type *Ty, raw_ostream &OS,
                         bool IgnoreTopLevelName) {
  // Check to see if the type is named.
  DenseMap<const Type*, std::string> &TM = getTypeNamesMap(TypeNames);
  if (!IgnoreTopLevelName) {
    DenseMap<const Type*, std::string>::iterator I = TM.find(Ty);
    if (I != TM.end()) {
      OS << I->second;
      return;
    }
  }

  // Otherwise we have a type that has not been named but is a derived type.
  // Carefully recurse the type hierarchy to print out any contained symbolic
  // names.
  SmallVector<const Type *, 16> TypeStack;
  std::string TypeName;

  raw_string_ostream TypeOS(TypeName);
  CalcTypeName(Ty, TypeStack, TypeOS, IgnoreTopLevelName);
  OS << TypeOS.str();

  // Cache type name for later use.
  if (!IgnoreTopLevelName)
    TM.insert(std::make_pair(Ty, TypeOS.str()));
}

void TypeGen::gen(std::vector<const Type*> numberedTypes,const TypeSymbolTable &ST,raw_ostream &OS){
	const Type *type;
	//Emit all numbered types.
	for (int NI=0,NE=numberedTypes.size();NI!=NE;++NI){
		type=numberedTypes[NI];
		this->printAtLeastOneLevel(type,OS);
		OS<<'\n';
	}

	//Print the named types.
	for (TypeSymbolTable::const_iterator TI=ST.begin(),TE=ST.end();TI!=TE;++TI){
		this->printAtLeastOneLevel(TI->second,OS);
		OS<<'\n';
	}
}

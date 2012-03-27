#include "TypeFinder.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/TypeSymbolTable.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
#include <cctype>

using namespace llvm;
// PrintEscapedString - Print each character of the specified string, escaping
// it if it is not printable or if it is an escape char.
void PrintEscapedString(StringRef Name, raw_ostream &Out) {
	for (unsigned i = 0, e = Name.size(); i != e; ++i) {
    unsigned char C = Name[i];
    if (isprint(C) && C != '\\' && C != '"')
      Out << C;
    else
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
	}
}

enum PrefixType {
	GlobalPrefix,
	LabelPrefix,
	LocalPrefix,
	NoPrefix
};

/// PrintLLVMName - Turn the specified name into an 'LLVM name', which is either
/// prefixed with % (if the string only contains simple characters) or is
/// surrounded with ""'s (if it has special chars in it).  Print it out.
void PrintLLVMName(raw_ostream &OS, StringRef Name, PrefixType Prefix) {
	assert(Name.data() && "Cannot get empty name!");
	switch (Prefix) {
		default: llvm_unreachable("Bad prefix!");
		case NoPrefix: break;
		case GlobalPrefix: OS << '@'; break;
		case LabelPrefix:  break;
		case LocalPrefix:  OS << '%'; break;
	}

	// Scan the name to see if it needs quotes first.
	bool NeedsQuotes = isdigit(Name[0]);
	if (!NeedsQuotes) {
		for (unsigned i = 0, e = Name.size(); i != e; ++i) {
			char C = Name[i];
			if (!isalnum(C) && C != '-' && C != '.' && C != '_') {
				NeedsQuotes = true;
				break;
			}
		}
	}

	// If we didn't need any quotes, just write out the name in one blast.
	if (!NeedsQuotes) {
		OS << Name;
		return;
	}

	// Okay, we need quotes.  Output the quotes and escape any scary characters as
	// needed.
	OS << '"';
	PrintEscapedString(Name, OS);
	OS << '"';
}

/// PrintLLVMName - Turn the specified name into an 'LLVM name', which is either
/// prefixed with % (if the string only contains simple characters) or is
/// surrounded with ""'s (if it has special chars in it).  Print it out.
void PrintLLVMName(raw_ostream &OS, const Value *V) {
	PrintLLVMName(OS, V->getName(),
                isa<GlobalValue>(V) ? GlobalPrefix : LocalPrefix);
}

void TypeFinder::Run(const Module &M) {

	AddModuleTypesToPrinter(TP,NumberedTypes,&M);

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

void TypeFinder::AddModuleTypesToPrinter(TypePrinting &TP,
							 std::vector<const Type*> &NumberedTypes,
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
		PrintLLVMName(NameOS, TI->first, LocalPrefix);
		NameOS.flush();
		TP.addTypeName(Ty, NameStr);
	}
}

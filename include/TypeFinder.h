#ifndef PMG_TYPEFINDER_H
#define PMG_TYPEFINDER_H

#include "llvm/Assembly/Writer.h"
#include "llvm/Module.h"
#include "llvm/DerivedTypes.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

class TypeFinder {
    // To avoid walking constant expressions multiple times and other IR
    // objects, we keep several helper maps.
    DenseSet<const Value*> VisitedConstants;
    DenseSet<const Type*> VisitedTypes;

    TypePrinting &TP;
    std::vector<const Type*> &NumberedTypes;
  public:
    TypeFinder(TypePrinting &tp, std::vector<const Type*> &numberedTypes)
      : TP(tp), NumberedTypes(numberedTypes) {}

    void Run(const Module &M);
  private:
    void IncorporateType(const Type *Ty);

    /// IncorporateValue - This method is used to walk operand lists finding
    /// types hiding in constant expressions and other operands that won't be
    /// walked in other ways.  GlobalValues, basic blocks, instructions, and
    /// inst operands are all explicitly enumerated.
    void IncorporateValue(const Value *V);
	void AddModuleTypesToPrinter(TypePrinting &TP,
								 std::vector<const Type *> &NumberedTypes,
								 const Module *M);
};

}

#endif

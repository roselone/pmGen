#ifndef PMG_SLOTTRACKER_H
#define PMG_SLOTTRACKER_H

#include "llvm/ADT/DenseSet.h"

namespace llvm {

class Module;
class Function;
class Value;
class GlobalValue;
class MDNode;

/// This class provides computation of slot numbers for LLVM Assembly writing.
///
class SlotTracker {
public:
  /// ValueMap - A mapping of Values to slot numbers.
  typedef DenseMap<const Value*, unsigned> ValueMap;

private:
  /// TheModule - The module for which we are holding slot numbers.
  const Module* TheModule;

  /// TheFunction - The function for which we are holding slot numbers.
  const Function* TheFunction;
  bool FunctionProcessed;

  /// mMap - The TypePlanes map for the module level data.
  ValueMap mMap;
  unsigned mNext;

  /// fMap - The TypePlanes map for the function level data.
  ValueMap fMap;
  unsigned fNext;

  /// mdnMap - Map for MDNodes.
  DenseMap<const MDNode*, unsigned> mdnMap;
  unsigned mdnNext;
public:
  /// Construct from a module
  explicit SlotTracker(const Module *M);
  /// Construct from a function, starting out in incorp state.
  explicit SlotTracker(const Function *F);

  /// Return the slot number of the specified value in it's type
  /// plane.  If something is not in the SlotTracker, return -1.
  int getLocalSlot(const Value *V);
  int getGlobalSlot(const GlobalValue *V);
  int getMetadataSlot(const MDNode *N);

  /// If you'd like to deal with a function instead of just a module, use
  /// this method to get its data into the SlotTracker.
  void incorporateFunction(const Function *F) {
    TheFunction = F;
    FunctionProcessed = false;
  }

  /// After calling incorporateFunction, use this method to remove the
  /// most recently incorporated function from the SlotTracker. This
  /// will reset the state of the machine back to just the module contents.
  void purgeFunction();

  /// MDNode map iterators.
  typedef DenseMap<const MDNode*, unsigned>::iterator mdn_iterator;
  mdn_iterator mdn_begin() { return mdnMap.begin(); }
  mdn_iterator mdn_end() { return mdnMap.end(); }
  unsigned mdn_size() const { return mdnMap.size(); }
  bool mdn_empty() const { return mdnMap.empty(); }

  /// This function does the actual initialization.
  inline void initialize();

  // Implementation Details
private:
  /// CreateModuleSlot - Insert the specified GlobalValue* into the slot table.
  void CreateModuleSlot(const GlobalValue *V);

  /// CreateMetadataSlot - Insert the specified MDNode* into the slot table.
  void CreateMetadataSlot(const MDNode *N);

  /// CreateFunctionSlot - Insert the specified Value* into the slot table.
  void CreateFunctionSlot(const Value *V);

  /// Add all of the module level global variables (and their initializers)
  /// and function declarations, but not the contents of those functions.
  void processModule();

  /// Add all of the functions arguments, basic blocks, and instructions.
  void processFunction();

  SlotTracker(const SlotTracker &);  // DO NOT IMPLEMENT
  void operator=(const SlotTracker &);  // DO NOT IMPLEMENT
};

}

#endif

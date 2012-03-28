#ifndef PMG_TYPEGEN_H
#define PMG_TYPEGEN_H

#include <string>

namespace llvm {

class Type;
class Module;
class Value;
class raw_ostream;
template <typename T> class SmallVectorImpl;

class TypeGen {
	void *TypeNames;
public:
	TypeGen();
	~TypeGen();
	
	void clear();
	void print(const Type *Ty, raw_ostream &OS, bool IgnoreTopLevelName = false);
  
	void printAtLeastOneLevel(const Type *Ty, raw_ostream &OS) {
		print(Ty, OS, true);
	}
  
	/// hasTypeName - Return true if the type has a name in TypeNames, false
	/// otherwise.
	bool hasTypeName(const Type *Ty) const;
  
	/// addTypeName - Add a name for the specified type if it doesn't already have
	/// one.  This name will be printed instead of the structural version of the
	/// type in order to make the output more concise.
	void addTypeName(const Type *Ty, const std::string &N);
  
private:
	void CalcTypeName(const Type *Ty, SmallVectorImpl<const Type *> &TypeStack,
                    raw_ostream &OS, bool IgnoreTopLevelName = false);

};

}

#endif

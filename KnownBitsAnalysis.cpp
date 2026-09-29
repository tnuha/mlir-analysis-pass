// KnownBitsAnalysis.cpp - Transfer functions
//

#include "KnownBitsAnalysis.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Matchers.h"

using namespace mlir;

namespace knownbits {

// TODO: update BitState
void KnownBitsAnalysis::setToEntryState(BitLattice *lattice) {
  propagateIfChanged(lattice, lattice->join(BitState::top()));
}

LogicalResult
KnownBitsAnalysis::visitOperation(Operation *op,
                                  ArrayRef<const BitLattice *> operands,
                                  ArrayRef<BitLattice *> results) {
  // Raising a result to top says "this operation could produce anything",
  // which is always a sound answer and is what every unhandled case does.
  auto unknown = [&] {
    setAllToEntryStates(results);
    return success();
  };

  // Only single-result integer operations are interesting here.  Calls, loads,
  // floats, and vectors all land in `unknown`.
  // if (op->getNumResults() != 1 || !op->getResult(0).getType().isIntOrIndex())
  //   return unknown();
  // ZeroLattice *result = results[0];

  // Rule 1: a constant is zero or nonzero according to what it says.
  // This is the only rule that does not consult its operands, and without some
  // rule of this kind the analysis would have no facts to propagate at all.
  // IntegerAttr value;
  // if (matchPattern(op, m_Constant(&value))) {
  //   ZeroState state = value.getValue().isZero() ? Kind::Zero : Kind::NonZero;
  //   propagateIfChanged(result, result->join(state));
  //   return success();
  // }

  // Rule 2: `x & y` is zero if either operand is zero, since a zero operand
  // clears every bit.  Note what this rule does *not* say: two nonzero
  // operands tell us nothing, because 1 & 2 is 0.
  // Zeros win `&`.
  // if (isa<LLVM::AndOp>(op)) {
  //   ZeroState lhs = operands[0]->getValue();
  //   ZeroState rhs = operands[1]->getValue();

  // Bottom means the solver has not yet proved anything reaches this
  // operand.  Leaving the result alone keeps the analysis optimistic; the
  // solver will call back here once the operand moves up the lattice.
  //   if (lhs.isBottom() || rhs.isBottom())
  //     return success();

  //   if (lhs.kind == Kind::Zero || rhs.kind == Kind::Zero) {
  //     propagateIfChanged(result, result->join(ZeroState(Kind::Zero)));
  //     return success();
  //   }
  // }

  // Rule 3: `x | y` is nonzero if either operand is nonzero.
  // Zeros lose `|`.
  // if (isa<LLVM::OrOp>(op)) {
  //   ZeroState lhs = operands[0]->getValue();
  //   ZeroState rhs = operands[1]->getValue();

  //   // unreachable, as above
  //   if (lhs.isBottom() || rhs.isBottom())
  //     return success();

  //   if (lhs.kind == Kind::NonZero || rhs.kind == Kind::NonZero) {
  //     propagateIfChanged(result, result->join(ZeroState(Kind::NonZero)));
  //   }
  // }

  return unknown();
}

} // namespace knownbits

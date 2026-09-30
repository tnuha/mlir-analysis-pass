// KnownBitsAnalysis.cpp - Transfer functions
//

#include "KnownBitsAnalysis.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Matchers.h"
#include "llvm/IR/Constants.h"

using namespace mlir;

namespace knownbits {

// TODO: update BitState
void KnownBitsAnalysis::setToEntryState(BitLattice *lattice) {
  propagateIfChanged(lattice, lattice->join(BitFlagsState::top()));
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

  // currently only supports signless 8b integer operations
  if (op->getNumResults() != 1 ||
      !op->getResult(0).getType().isSignlessInteger(NBITS))
    return unknown();
  BitLattice *result = results[0];

  // Rule 1: a constant reflects its literal bit pattern.
  // This is the only rule that does not consult its operands, and without some
  // rule of this kind the analysis would have no facts to propagate at all.
  IntegerAttr value;
  if (matchPattern(op, m_Constant(&value))) {
    // should be 8b due to result check above
    // TODO: generalize to other sizes
    uint8_t raw = static_cast<uint8_t>(value.getValue().getZExtValue());
    BitFlagsState state(raw);
    propagateIfChanged(result, result->join(state));
    return success();
  }

  // Rule 2: If all bits are known, operations are identical
  // to those carried out by the raw bits.
  if (op->getNumOperands() == 2) {
    BitFlagsState lhs = operands[0]->getValue();
    BitFlagsState rhs = operands[1]->getValue();
    if (lhs.fullyKnown() && rhs.fullyKnown()) {
      // TODO: switch depending on op
      return success();
    }
  }

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

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

  if (op->getNumOperands() == 2) {
    BitFlagsState lhs = operands[0]->getValue();
    BitFlagsState rhs = operands[1]->getValue();
    // Rule 2: Bitwise operations are super easy.
    if (isa<LLVM::AndOp>(op)) {
      propagateIfChanged(result, result->join(lhs & rhs));
      return success();
    } else if (isa<LLVM::OrOp>(op)) {
      propagateIfChanged(result, result->join(lhs | rhs));
      return success();
    } else if (isa<LLVM::XOrOp>(op)) {
      propagateIfChanged(result, result->join(lhs ^ rhs));
      return success();
    } else if (isa<LLVM::ShlOp>(op)) {
      propagateIfChanged(result, result->join(lhs << rhs));
      return success();
    } else if (isa<LLVM::AddOp>(op)) {
      propagateIfChanged(result, result->join(lhs + rhs));
      return success();
    }
    // Rule 3: If all bits are known, operations are identical
    // to those carried out by the raw bits.
    if (lhs.fullyKnown() && rhs.fullyKnown()) {
      // TODO: switch depending on op
      // TODO: add more?
      auto lhs_raw = lhs.asRaw();
      auto rhs_raw = rhs.asRaw();
      BitFlagsState state = BitFlagsState::top();
      if (isa<LLVM::SubOp>(op))
        state = BitFlagsState(lhs_raw - rhs_raw);

      propagateIfChanged(result, result->join(state));
      return success();
    }
  }

  return unknown();
}

} // namespace knownbits

//===- ZeroAnalysis.h - Sparse forward analysis over ZeroState ------------===//

#ifndef KNOWNBITS_ANALYSIS_H
#define KNOWNBITS_ANALYSIS_H

#include "KnownBitsDomain.h"
#include "mlir/Analysis/DataFlow/SparseAnalysis.h"

namespace knownbits {

using BitLattice = mlir::dataflow::Lattice<BitState>;

class KnownBitsAnalysis
    : public mlir::dataflow::SparseForwardDataFlowAnalysis<BitLattice> {
public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;

  /// Transfer function: given the states of `op`'s operands, set the states of
  /// its results.  Must be monotone in the operand states.
  mlir::LogicalResult
  visitOperation(mlir::Operation *op,
                 llvm::ArrayRef<const BitLattice *> operands,
                 llvm::ArrayRef<BitLattice *> results) override;

  /// The state of anything entering the analysis from outside: function
  /// arguments, and results the transfer function declines to reason about.
  void setToEntryState(BitLattice *lattice) override;
};

} // namespace knownbits

#endif

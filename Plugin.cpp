//===- Plugin.cpp - Pass definition and plugin entry point ----------------===//
//
// Scaffolding: wires the analysis into a pass and exposes it to mlir-opt.
//
//===----------------------------------------------------------------------===//

#include "Annotate.h"
#include "KnownBitsAnalysis.h"

#include "KnownBitsDomain.h"
#include "mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"
#include "mlir/Analysis/DataFlow/DeadCodeAnalysis.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"
#include <mlir/IR/BuiltinTypes.h>
#include <utility>

using namespace mlir;

namespace {

void addState(
    llvm::SmallVector<std::pair<Value, knownbits::BitFlagsState>> states,
    DataFlowSolver &solver, Value &val) {
  auto type = llvm::dyn_cast<IntegerType>(val.getType());
  if (!type || type.getWidth() != NBITS)
    return;
  const auto *element = solver.lookupState<knownbits::BitLattice>(val);
  if (!element)
    return;
  knownbits::BitFlagsState state = element->getValue();
  if (state != knownbits::BitFlagsState::top())
    states.emplace_back(val, state);
}

void instrument(Value val, knownbits::BitFlagsState state) {
  // TODO
  assert(false);
}

struct KnownBitsAnalysisPass
    : PassWrapper<KnownBitsAnalysisPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(KnownBitsAnalysisPass)

  KnownBitsAnalysisPass() = default;
  KnownBitsAnalysisPass(const KnownBitsAnalysisPass &pass) {}

  Option<int> debugOption{
      *this, "debug", llvm::cl::desc("enable instrumented debug assertions")};

  StringRef getArgument() const final { return "knownbits-analysis"; }

  StringRef getDescription() const final {
    return "Find interesting dataflow facts about bits in a program";
  }

  void runOnOperation() override {

    DataFlowConfig config;
    config.setInterprocedural(false);

    DataFlowSolver solver(config);
    // DeadCodeAnalysis supplies reachability, without which the solver must
    // assume every branch is taken; SparseConstantPropagation resolves branch
    // conditions for it.  Both are prerequisites, not extras.
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    solver.load<knownbits::KnownBitsAnalysis>();

    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation().emitError(
          "knownbits analysis failed to reach a fixed point");
      return signalPassFailure();
    }

    llvm::SmallVector<std::pair<Value, knownbits::BitFlagsState>> states;
    if (this->debugOption.hasValue()) {
      getOperation().walk([&](LLVM::LLVMFuncOp F) {
        for (auto &B : F.getBody()) {
          for (Value arg : B.getArguments()) {
            addState(states, solver, arg);
          }
          for (auto &op : B) {
            for (Value res : op.getResults())
              addState(states, solver, res);
          }
        }
      });
    }

    // Query states only now that the solver has converged.
    auto describe = [&](Value value, AsmState &asmState) -> std::string {
      const auto *lattice = solver.lookupState<knownbits::BitLattice>(value);
      if (!lattice)
        return {};
      auto val = lattice->getValue();
      // remove unapplicable facts
      if (val == knownbits::BitFlagsState::bottom() ||
          val == knownbits::BitFlagsState::top())
        return {};
      std::string description;
      llvm::raw_string_ostream os(description);
      value.printAsOperand(os, asmState);
      os << " is " << val;
      return description;
    };

    // stderr, so that mlir-opt's stdout stays the unmodified IR and the two
    // can be redirected independently.
    knownbits::printAnnotated(getOperation(), describe, llvm::errs());

    for (const auto &[val, state] : states)
      instrument(val, state);

    // This pass only reads.
    markAllAnalysesPreserved();
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo mlirGetPassPluginInfo() {
  // LLVM_VERSION_STRING is baked in at compile time and checked by mlir-opt at
  // load time, which is what turns an ABI mismatch into a clear diagnostic.
  return {MLIR_PLUGIN_API_VERSION, "KnownBitsAnalysis", LLVM_VERSION_STRING,
          []() { PassRegistration<KnownBitsAnalysisPass>(); }};
}

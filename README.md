# MLIR out-of-tree dataflow analysis for known-bits

The included analysis, `knownbits-analysis`, decides
on known facts about giving bits in 8b variables
in a program.

## Building

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

That is the whole procedure on Linux, macOS, and WSL2. There is no platform
flag to set and no path to edit. `CMakeLists.txt` finds MLIR by asking
whichever `llvm-config` is on your `PATH` where its CMake package lives, so if
`mlir-opt` runs, the build should configure.

To build against a specific MLIR instead:

```sh
cmake -S . -B build -DMLIR_DIR=/path/to/prefix/lib/cmake/mlir
```

You need an LLVM built with MLIR enabled and plugins enabled
(`-DLLVM_ENABLE_PROJECTS=mlir -DLLVM_ENABLE_PLUGINS=ON`; both are ordinary on
Linux and macOS). Distribution packages work: on Debian and Ubuntu that is
`libmlir-dev` alongside `llvm-dev`. On macOS, Homebrew's `llvm` is the easy
route if it ships `mlir-opt` for your version; otherwise build LLVM yourself.
The configure step diagnoses the cases it can detect — no MLIR
found, plugins disabled in the host LLVM, or an `mlir-opt` on `PATH` whose
version does not match what you are building against.

## Running

```sh
./run.sh input.mlir
```

`run.sh` locates the plugin whatever it is called on your platform and puts the
annotated listing on stdout. Or invoke `mlir-opt` yourself:

```sh
mlir-opt --load-pass-plugin=build/KnownBitsAnalysis.so \
         --pass-pipeline='builtin.module(knownbits-analysis)' \
         input.mlir -o /dev/null
```

using `build/KnownBitsAnalysis.dylib` on macOS. The pass leaves the IR unchanged and
writes it to stdout as usual; the annotated view goes to stderr, so the two
streams can be redirected independently. Annotations are comments, so the
annotated listing is still valid MLIR. Values at top or bottom are left
unannotated, so that what prints is exactly what was proved.

Get input in the LLVM dialect from C with:

```sh
clang -S -emit-llvm -o - input.c | mlir-translate --import-llvm
```

## What is where

This is a template following
[John Regehr's MLIR dataflow analysis template](<https://github.com/regehr/mlir-analysis-pass>).
Aside from the obvious change from zero-analysis to known bits analysis,
the structure is much the same.

## Tests

`./test` contains MLIR and relevant C files
for this analysis. An easy run after building the analysis plugin
is `./run.sh test/foo.mlir`.

Much like the handout, comments are appended to relevant lines
carrying information about the result abstract values.
This is a vector of abstract bit states, those being:

- `T`: top, whose concretization set is {0, 1}
- `1`: one, whose concretization set is {1}
- `0`: zero, whose concretization set is {0}
- `B`: bottom, whose concretization set is {}. Due to dead code elimination, this is unexpected.

Example output for `test/foo.mlir`:

```mlir
%0 = llvm.mlir.constant(5 : i8) : i8 // %0 is 00000101
%1 = llvm.mlir.constant(10 : i8) : i8 // %1 is 00001010
%2 = llvm.add %0, %1 : i8 // %2 is 00001111
%3 = llvm.xor %0, %1 : i8 // %3 is 00001111
%4 = llvm.sub %0, %1 : i8 // %4 is 11111011
%5 = llvm.and %0, %1 : i8 // %5 is 00000000
%6 = llvm.or %0, %1 : i8 // %6 is 00001111
%7 = llvm.add %6, %arg1 : i8
%8 = llvm.mlir.constant(-16 : i8) : i8 // %8 is 11110000
%9 = llvm.and %8, %arg0 : i8 // %9 is TTTT0000
%10 = llvm.or %1, %9 : i8 // %10 is TTTT1010
%11 = llvm.mlir.constant(1 : i8) : i8 // %11 is 00000001
%12 = llvm.shl %10, %11 : i8 // %12 is TTT10100
%13 = llvm.mlir.constant(2 : i8) : i8 // %13 is 00000010
%14 = llvm.shl %10, %13 : i8 // %14 is TT101000
%15 = llvm.add %14, %11 : i8 // %15 is TT101001
%16 = llvm.add %9, %8 : i8
llvm.return %10 : i8
```

## Notes on portability

Most of the platform-specific knowledge lives in `CMakeLists.txt`, next to the
code it affects. The parts worth knowing about:

**The plugin's file name differs.** It is `KnownBitsAnalysis.dylib` on macOS and
`KnownBitsAnalysis.so` on Linux and WSL2. Nothing in this project spells that out:
CMake is asked via `$<TARGET_FILE:KnownBitsAnalysis>`, and `run.sh` probes for both.

**Linking a plugin on macOS needs special flags.** The plugin deliberately
leaves its MLIR symbols undefined, to be resolved from the `mlir-opt` process
that loads it. On macOS that requires `-undefined dynamic_lookup`, which
`include(HandleLLVMOptions)` supplies. The same include also matches LLVM's
RTTI and exception settings, which differ between distribution packages and
local builds and cause link errors or silent ODR violations when they are
wrong. That is also why `project()` enables C: `HandleLLVMOptions` probes flags
with the C compiler and fails if none is configured.

**A plugin only loads into the LLVM it was built against.** The version is
recorded at compile time and checked at load time, so a mismatch is a clear
error rather than a crash. The configure step warns about it earlier still, by
comparing against the `mlir-opt` it finds.

**The test suite needs no shell.** `cmake/RunTest.cmake` is a CMake script
rather than a shell script, so `ctest` depends on nothing the build did not
already require.

**Under WSL2, build on the Linux filesystem.** A tree under `/mnt/c` is
slow enough to be noticeable and does not reliably carry execute bits.
`.gitattributes` forces LF endings, which keeps `run.sh` working when a
repository is cloned by a Windows git and built inside WSL2.

## How the analysis works

`Plugin.cpp` loads three analyses into one solver. `DeadCodeAnalysis` supplies
reachability — without it the solver must assume every branch is taken — and
`SparseConstantPropagation` resolves branch conditions on its behalf. These are
prerequisites for a precise result, not optional extras. `KnownBitsAnalysis` then
propagates known bits through operations and block arguments until the solver
reaches a fixed point, which is when the pass queries it.

8b bitflags are analyzed bitwise; given an array of N "BitKind"s.
With a few special exceptions,
zeroness of the individual bit is computed depending on its individual join.
This behaves much like zero analysis.

Specifically, transfer functions consider the following cases:

- **Constants** are interpreted by their exact bits. This is the only rule that does
  not consult its operands, and without some rule of this kind there would be
  no facts to propagate at all.
- **bitwise logical operators** like `&`, `|`, `^`, are each computed bitwise
  between two different operands.
- **left shift** is considered due to its ubiquity in C programs when considering
  bitmasking. Naturally, assuming that the right hand side of this operation is comprised
  of only known bits, this simply carries abstract bitvalues "leftward".
  If any bit is unknown, precision is fully lost, so the result is entirely `Top` bits.
- **Abstract add with unknown bits** is possible to perform via composition with bitwise operators.
  With the special case of a carry bit with a top value making the whole vector top values,
  the operation is a set of a few shifts, xors, and ands.
- **Constant operations** are easy to perform by translating an abstract value
  comprised only of known bits to a concrete C++ type, performing the operation,
  and then translating back.

The analysis is intraprocedural.

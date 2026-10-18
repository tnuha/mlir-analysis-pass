# bugfinder fuzzer and harness

Given the small cardinality of this domain,
finding a given bug is rather straightforward and cheap.

Simply instrument a piece of MLIR with the parent directory's
`./run.sh -d`, compile to a piece of object code,
and link against this `harness.c`.
Assumes the input is a single 8-bit integer
and that the procedure's name is `foo`.

## Example usage

```sh
# from the root directory of this repository
./run.sh -d bugfinder/example.mlir                 \
  | mlir-translate --mlir-to-llvmir | llc          \
  | clang -x assembler - -c -o bugfinder/example.o

clang bugfinder/harness.c bugfinder/example.o -o bugfinder/harness

./bugfinder/harness
```

Changing `bugfinder/example.mlir` to another function
for more interesting behavior is trivial.
As usual, `./run.sh` without the debug flag still provides
relevant insights for generated code.

## Example caught bug

Currently, no known bugs have been caught by this approach,
although commit `538645bc` was a case where some light was shed
on some additional precision that could be gathered with this approach.

Exposing a caught bug by neglecting a carried bit
can be done with the following git diff and the approach described above:

```diff
index c5fcac3..429849f 100644
--- a/KnownBitsDomain.h
+++ b/KnownBitsDomain.h
@@ -204,11 +204,11 @@ struct BitFlagsState {
     BitFlagsState result = a ^ b;
     BitFlagsState one(0x1);
     BitFlagsState zero(0x0);
-    while (carry != zero) {
-      BitFlagsState shiftedcarry = carry << 1;
-      carry = result & shiftedcarry;
-      result = result ^ shiftedcarry;
-    }
+    // while (carry != zero) {
+    BitFlagsState shiftedcarry = carry << 1;
+    carry = result & shiftedcarry;
+    result = result ^ shiftedcarry;
+    // }
     return result;
   }
   BitFlagsState operator+(const BitFlagsState &other) {
```

On x86 + Linux, this triggers an illegal instruction exception,
as the following analyzed results are semantically false w.r.t.
8-bit arithmetic (specifically the last line):

```mlir
%0 = llvm.mlir.constant(-16 : i8) : i8 // %0 is 11110000
%1 = llvm.and %arg0, %0 : i8 // %1 is TTTT0000
%2 = llvm.add %0, %1 : i8 // %2 is TTTT0000
%3 = llvm.mlir.constant(1 : i8) : i8 // %3 is 00000001
%4 = llvm.mlir.constant(3 : i8) : i8 // %4 is 00000011
%5 = llvm.add %2, %3 : i8 // %5 is TTTT0001
%6 = llvm.add %4, %5 : i8 // %6 is TTTT0000
```


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
  | mlir-translate --mlir-to-llvm-ir | llc         \
  | clang -x assembler - -c -o bugfinder/example.o

clang bugfinder/harness.c bugfinder/example.o -o bugfinder/harness

./bugfinder/harness
```

Changing `bugfinder/example.mlir` to another function
for more interesting behavior is trivial.
As usual, `./run.sh` without the debug flag still provides
relevant insights for generated code.


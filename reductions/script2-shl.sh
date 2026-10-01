#!/bin/env bash

# Script generates smallest case of interesting results
# from a `shl`.

case $1 in
  /*) arg=$1 ;;         # arg is absolute
  *) arg="$PWD"/"$1" ;; # arg is relative
esac

cd "$(dirname $0)"/..

# saving to tmp file as entire buffer may not fit in memory
mlir-translate --import-llvm $arg > tmp.mlir

# actual check
# add instructions with non-top results
./run.sh ./tmp.mlir          \
  | grep 'shl'               \
  | grep 'is'                \
  | grep -E -i '[01]T|T[01]' \
  > /dev/null
res="$?"

exit $res


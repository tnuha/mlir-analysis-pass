#!/bin/env bash

# Script generates smallest case of analysis results
# where any known bit precedes a known Top bit.

case $1 in
  /*) arg=$1 ;;         # arg is absolute
  *) arg="$PWD"/"$1" ;; # arg is relative
esac

cd "$(dirname $0)"/..

# saving to tmp file as entire buffer may not fit in memory
mlir-translate --import-llvm $arg > tmp.mlir

# actual check
./run.sh ./tmp.mlir | grep 'is' | grep -E -i '[01]T' > /dev/null
res="$?"

exit $res


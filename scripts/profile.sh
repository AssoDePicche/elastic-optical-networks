#!/bin/bash

set -e

if ! command -v "gprof" &> /dev/null; then
  echo "gprof not found"

  exit 1
fi

if [[ ! -f "./App-linux-x86_64" ]]; then
  scripts/build.sh Debug
fi

if [[ ! -e "gmon.out" ]]; then
  ./App-linux-x86_64
fi

echo "Running Valgrind..."

valgrind ./App-linux-x86_64 > valgrind.txt

echo "Running Gprof..."

gprof App-linux-x86_64 gmon.out > gprof.txt

echo "Gprof report saved to gprof.txt"

echo "Generating Flame Graph..."

flamegraph --open --cmd "record -g" -- ./App-linux-x86_64

echo "Flame graph saved to flamegraph.svg"

gprof2dot gprof.txt > gprof.dot

dot -Tpng -o gprof.png gprof.dot

echo "Gprof graph saved to gprof.png"

echo "Cleaning up..."

rm gmon.out gprof.dot

echo "Good to go"

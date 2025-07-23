#!/bin/bash

set -e

if ! command -v "gprof" &> /dev/null; then
  echo "gprof not found"

  exit 1
fi

if [[ ! -d "./build/" ]]; then
  scripts/build.sh
fi

if [[ ! -e "gmon.out" ]]; then
  ./build/App
fi

echo "Running Gprof..."

gprof build/App gmon.out > gprof.txt

echo "Gprof report saved to gprof.txt"

rm gmon.out

gprof2dot gprof.txt > gprof.dot

dot -Tpng -o gprof.png gprof.dot

rm gprof.dot

echo "Gprof graph saved to gprof.png"

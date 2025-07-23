#!/bin/bash

set -e

if [[ -d "./build/" ]]; then
  echo "Testing..."
else
  ./scripts/build.sh && echo "Testing..."
fi

ctest -j$(fgrep 'processor' /proc/cpuinfo | wc -l) --output-on-failure --test-dir build

#!/bin/bash

if [[ -d "./build/" ]]; then
  echo "Testing..."
else
  ./scripts/build.sh && echo "Testing..."
fi

ctest -j12 --output-on-failure --test-dir build

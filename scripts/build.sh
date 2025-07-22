#!/bin/bash

if [[ -e "./vendor/googletest/CMakeLists.txt" && -e "./vendor/json/CMakeLists.txt" ]]; then
  echo "Building..."
else
  echo "Cloning submodules" && git submodule update --init --recursive
fi

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -j$(fgrep 'processor' /proc/cpuinfo | wc -l)

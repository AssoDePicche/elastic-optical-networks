#!/bin/bash

mkdir -p third_party && cmake -DCMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -j12 && ctest -j12 --output-on-failure --test-dir build

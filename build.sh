#!/bin/bash

mkdir -p third_party && cmake -DCMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build -j12

#!/bin/bash

find application -name "*.cpp" -o -name "*.h" | xargs clang-format -i -style=Google

#!/bin/bash

INPUT=${1-cities.txt}

OUTPUT=${2-matrix.txt}

docker run -v $(pwd):/app geocoding "$INPUT" "$OUTPUT"

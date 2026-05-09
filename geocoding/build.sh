#!/bin/bash

INPUT=${1-cities.txt}

OUTPUT=${2-matrix.txt}

docker build -f Dockerfile -t geocoding .

docker run -v $(pwd):/app geocoding "$INPUT" "$OUTPUT"

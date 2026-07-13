#!/bin/bash

INPUT=${1}

docker run -v $(pwd):/app visualization --plot "$INPUT"

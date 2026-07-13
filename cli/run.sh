#!/bin/bash

MIN_LOAD=${1-0}
MAX_LOAD=${2-90}
OFFSET=${3-10}
CONFIG=${4-"configuration.json"}

docker run --rm -v $(pwd):/app graph-sim "$MIN_LOAD" "$MAX_LOAD" "$OFFSET" "$CONFIG"

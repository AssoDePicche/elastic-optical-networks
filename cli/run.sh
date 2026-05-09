#!/bin/bash

docker build -t graph-sim .

docker run --rm -v $(pwd):/app graph-sim 10 100 10

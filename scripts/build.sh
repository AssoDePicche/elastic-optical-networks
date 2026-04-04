#!/bin/bash

BUILD_TYPE=${1:-Release}

docker build \
  -f application/Dockerfile \
  --build-arg BUILD_TYPE=$BUILD_TYPE \
  --target exporter \
  --output type=local,dest=. \
  .

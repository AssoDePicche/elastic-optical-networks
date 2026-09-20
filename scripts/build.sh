#!/bin/bash
set -euo pipefail

BUILD_TYPE=${1:-Release}

export DOCKER_BUILDKIT=1

docker build \
  -f application/Dockerfile \
  --build-arg BUILD_TYPE="$BUILD_TYPE" \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --target exporter \
  --output type=local,dest=. \
  --progress=plain \
  .

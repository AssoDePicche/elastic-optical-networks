#!/bin/bash

docker build -f application/Dockerfile --target exporter --output type=local,dest=. .

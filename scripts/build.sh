#!/bin/bash

docker build --target exporter --output type=local,dest=. .

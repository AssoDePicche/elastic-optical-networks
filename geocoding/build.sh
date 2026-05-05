#!/bin/bash

docker build -t geocoding .

docker run -v $(pwd):/app geocoding

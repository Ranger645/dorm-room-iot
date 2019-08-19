#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ROOT_DIR=$DIR/..
PORT=10101

echo "Creating command server on port ${PORT}..."

sudo $ROOT_DIR/exe/run $PORT &


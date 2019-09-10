#!/bin/bash

ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Cleaning previous build by default
rm -rf $ROOT/builds/*

# Creating new build directory
BUILD_FOLDER_PREFIX="build_"
BUILD_NAME="${BUILD_FOLDER_PREFIX}$(date +"%Y-%m-%d_%T")"
echo "Building ${BUILD_NAME}..."
BUILD_ROOT="${ROOT}/builds"
BUILD_DIR="${BUILD_ROOT}/${BUILD_NAME}"
mkdir $BUILD_DIR

# Compiling source
SOURCE_CODE_DIR="${ROOT}/src"
gcc -std=gnu99 -pthread -Wall -g -o $BUILD_DIR/run $SOURCE_CODE_DIR/*.c $SOURCE_CODE_DIR/util/*.c $SOURCE_CODE_DIR/daemons/*.c

# Moving scripts into build
cp -r $ROOT/scripts $BUILD_DIR/scripts

# Creating logging folder
mkdir $BUILD_DIR/log

echo "Build Attempt Complete"

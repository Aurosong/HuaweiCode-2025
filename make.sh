#!/bin/bash

BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p $BUILD_DIR
    echo "Created build directory: $BUILD_DIR"
else
    echo "Build directory already exists: $BUILD_DIR"
fi

cd $BUILD_DIR
cmake ..
make
cd ..
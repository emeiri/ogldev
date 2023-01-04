#!/bin/bash

for filename in tutorial* Terrain*; do
    echo "Building " $filename "..."
    cd $filename
    ./build.sh
    cd ..
done

#!/bin/bash

for filename in tutorial*; do
    echo "Building " $filename "..."
    cd $filename
    ./build.sh
    cd ..
done

#!/bin/bash

for dirname in Vulkan/Tutorial* ; do
    echo "Building " $dirname "..."
    cd $dirname
    ./build.sh
    cd ../../
done

for dirname in tutorial* Terrain* ; do
    echo "Building " $dirname "..."
    cd $dirname
    ./build.sh
    cd ..
done

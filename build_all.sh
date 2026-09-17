#!/bin/bash

source ~/Downloads/1.4.328.1/setup-env.sh

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

#!/bin/bash

ROOTDIR="../.."
source ../../build_base.sh

build_demolition

cd ../Source
../build_physics_lib.sh
cd -

$CC *.cpp -I../Include -I../../DemoLITION/Framework/Include/ $OGL_CPPFLAGS ../../Lib/libdemolition.a ../../Lib/libphysics.a  $OGL_LDFLAGS -o physics01

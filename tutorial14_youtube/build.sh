#!/bin/bash

source ../build_base.sh

LDFLAGS=`pkg-config --libs glew`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC tutorial14.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp world_transform.cpp camera.cpp $OGL_CPPFLAGS $LDFLAGS -o tutorial14

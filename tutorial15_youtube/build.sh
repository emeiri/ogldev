#!/bin/bash

source ../build_base.sh
LDFLAGS=`pkg-config --libs glew ImageMagick freetype2 glfw3`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC tutorial15.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp world_transform.cpp camera.cpp $OGL_CPPFLAGS $LDFLAGS -o tutorial15

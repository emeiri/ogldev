#!/bin/bash

source ../build_base.sh
LDFLAGS=`pkg-config --libs glew ImageMagick++`
LDFLAGS="$LDFLAGS -lglut -lX11"

$CC tutorial16.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp world_transform.cpp camera.cpp $OGL_CPPFLAGS $LDFLAGS -o tutorial16

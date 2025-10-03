#!/bin/bash

source ../build_base.sh
LDFLAGS=`pkg-config --libs glew assimp`
LDFLAGS="$LDFLAGS -lglut -lX11 -lmeshoptimizer"

$CC tutorial28.cpp ../Common/ogldev_util.cpp  ../Common/math_3d.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp ../Common/ogldev_world_transform.cpp ../Common/ogldev_basic_mesh.cpp camera.cpp skinning_technique.cpp skinned_mesh.cpp ../Common/technique.cpp $OGL_CPPFLAGS $LDFLAGS -o tutorial28

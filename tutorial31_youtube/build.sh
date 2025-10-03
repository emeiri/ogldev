#!/bin/bash

source ../build_base.sh

$CC tutorial31.cpp picking_texture.cpp picking_technique.cpp simple_color_technique.cpp ../Common/ogldev_util.cpp ../Common/pipeline.cpp ../Common/math_3d.cpp ../Common/ogldev_basic_glfw_camera.cpp ../Common/ogldev_texture.cpp ../Common/ogldev_new_lighting.cpp ../Common/technique.cpp ../Common/ogldev_glfw.cpp ../Common/ogldev_atb.cpp ../Common/ogldev_world_transform.cpp ../Common/3rdparty/stb_image.cpp ../Common/ogldev_basic_mesh.cpp $OGL_CPPFLAGS $OGL_LDFLAGS -o tutorial31

#!/bin/bash

source ../build_base.sh

$CC tutorial36.cpp $ROOTDIR/Common/ogldev_framebuffer_object.cpp $ROOTDIR/Common/ogldev_util.cpp  $ROOTDIR/Common/math_3d.cpp $ROOTDIR/Common/ogldev_texture.cpp $ROOTDIR/Common/3rdparty/stb_image.cpp $ROOTDIR/Common/ogldev_world_transform.cpp $ROOTDIR/Common/ogldev_basic_glfw_camera.cpp $ROOTDIR/Common/ogldev_basic_mesh.cpp  $ROOTDIR/Common/ogldev_new_lighting.cpp $ROOTDIR/Common/ogldev_glfw.cpp $ROOTDIR/Common/ogldev_shadow_mapping_technique.cpp $ROOTDIR/Common/ogldev_shadow_map_fbo.cpp $ROOTDIR/Common/technique.cpp $OGL_CPPFLAGS $OGL_LDFLAGS -o tutorial36

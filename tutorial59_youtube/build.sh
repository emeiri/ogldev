#!/bin/bash

source ../build_base.sh

build_ogldev

$CC tutorial59.cpp bindless_tex_technique.cpp $OGL_CPPFLAGS ../Lib/libogldev.a $OGL_LDFLAGS -o tutorial59

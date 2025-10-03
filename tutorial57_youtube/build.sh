#!/bin/bash

source ../build_base.sh

build_ogldev

$CC tutorial57.cpp particles.cpp particles_technique.cpp $OGL_CPPFLAGS ../Lib/libogldev.a $OGL_LDFLAGS -o tutorial57

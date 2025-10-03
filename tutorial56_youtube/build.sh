#!/bin/bash

source ../build_base.sh

$CC tutorial56.cpp $OGL_CPPFLAGS ../Lib/libogldev.a $OGL_LDFLAGS -o tutorial56

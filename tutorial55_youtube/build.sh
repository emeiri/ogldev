#!/bin/bash

source ../build_base.sh

$CC tutorial55.cpp $OGL_CPPFLAGS ../Lib/libogldev.a $OGL_LDFLAGS -o tutorial55

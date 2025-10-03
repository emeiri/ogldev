#!/bin/bash

source ../build_base.sh

$CC endless_grid.cpp -I../DemoLITION/Framework/Include/ $OGL_CPPFLAGS ../Lib/libdemolition.a  $OGL_LDFLAGS -o endless_grid

#!/bin/bash

g++ -c  `pkg-config --cflags glew ImageMagick++ assimp freetype2 fontconfig gl glfw3 xcb` -DVULKAN -I../Include -I../Include/ImageMagick-6/ -I../Common/FreetypeGL -I/usr/include/freetype2 tutorial54.cpp -o tutorial54.o
g++  tutorial54.o -o tutorial54 -lglut -lxcb -lvulkan `pkg-config --libs glew ImageMagick++ assimp freetype2 fontconfig gl glfw3 xcb`   ../Lib/libAntTweakBar.a -lX11

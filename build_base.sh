CC=g++
OGL_CPPFLAGS="-I../Include -std=c++20"
#CPPFLAGS=`pkg-config --cflags glew ImageMagick++ assimp freetype2 fontconfig gl glfw3 xcb` 
#CPPFLAGS="-c $CPPFLAGS -DVULKAN -I../Include -I../Include/ImageMagick-6/ -I../Common/FreetypeGL -I/usr/include/freetype2"
OGL_LDFLAGS=`pkg-config --libs glew ImageMagick++ assimp freetype2 fontconfig gl glfw3 xcb`
OGL_LDFLAGS="$LDFLAGS -lglut -lxcb -lvulkan    ../Lib/libAntTweakBar.a -lX11"

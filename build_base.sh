if [ -v $ROOTDIR ]; then
    ROOTDIR=".."
fi

CC=g++
OGL_CPPFLAGS="$CPPFLAGS -I$ROOTDIR/Include -I$ROOTDIR/Common/FreetypeGL -I$ROOTDIR/Common/3rdparty/ImGui/GLFW/ -std=c++20"
OGL_LDFLAGS=`pkg-config --libs glew assimp`
OGL_LDFLAGS="$OGL_LDFLAGS -lglfw -lX11 -lmeshoptimizer -lglut"

build_ogldev() {
	cd $ROOTDIR/Common
	./build.sh
	cd -
}

build_demolition() {
	cd $ROOTDIR/Common
	./build.sh
	cd -
}



CC=g++
OGL_CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL -I../Common/3rdparty/ImGui/GLFW/ -std=c++20"
OGL_LDFLAGS=`pkg-config --libs glew assimp`
OGL_LDFLAGS="$OGL_LDFLAGS -lglfw -lX11 -lmeshoptimizer -lglut"
ROOTDIR=".."

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



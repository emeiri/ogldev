/*
    Regression test shim for freeglut (see Testing/README.md).

    Windows\ogldev_vs_2026\Directory.Build.targets puts Testing\shim in front of
    the include path, so <GL/freeglut.h> resolves to this file. It includes the
    real header and then routes a few GLUT calls through the test mode of
    Include/regress_hook.h. That way the tutorials themselves contain no testing
    code. Without --test-frames on the command line every wrapper just calls
    the real function.
*/

#include "../../../Include/GL/freeglut.h"

#if defined(__cplusplus) && !defined(REGRESS_GLUT_SHIM_H)
#define REGRESS_GLUT_SHIM_H

#include "../../../Include/regress_capture_gl.h"

namespace Regress {

// The end of the frame is the buffer swap
inline void GlutSwapBuffers()
{
    if (Active()) {
        if (!GetState().Capture) {
            InitGL();
        }

        GetState().Width = glutGet(GLUT_WINDOW_WIDTH);
        GetState().Height = glutGet(GLUT_WINDOW_HEIGHT);

        if (EndFrame()) {
            glutLeaveMainLoop();
        }

        // Some tutorials draw only when the window needs a repaint
        glutPostRedisplay();
    }

    glutSwapBuffers();
}


// The fixed 1/60 clock
inline int GlutGet(GLenum Query)
{
    if (Active() && (Query == GLUT_ELAPSED_TIME)) {
        return (int)TimeMillis(0);
    }

    return glutGet(Query);
}


// Never full screen
inline int GlutEnterGameMode()
{
    return Active() ? glutCreateWindow("Regression test") : glutEnterGameMode();
}


// No input
inline void GlutKeyboardFunc(void (*Callback)(unsigned char, int, int))
{
    if (!Active()) {
        glutKeyboardFunc(Callback);
    }
}


inline void GlutSpecialFunc(void (*Callback)(int, int, int))
{
    if (!Active()) {
        glutSpecialFunc(Callback);
    }
}


inline void GlutMouseFunc(void (*Callback)(int, int, int, int))
{
    if (!Active()) {
        glutMouseFunc(Callback);
    }
}


inline void GlutMotionFunc(void (*Callback)(int, int))
{
    if (!Active()) {
        glutMotionFunc(Callback);
    }
}


inline void GlutPassiveMotionFunc(void (*Callback)(int, int))
{
    if (!Active()) {
        glutPassiveMotionFunc(Callback);
    }
}

}

#define glutSwapBuffers       Regress::GlutSwapBuffers
#define glutGet               Regress::GlutGet
#define glutEnterGameMode     Regress::GlutEnterGameMode
#define glutKeyboardFunc      Regress::GlutKeyboardFunc
#define glutSpecialFunc       Regress::GlutSpecialFunc
#define glutMouseFunc         Regress::GlutMouseFunc
#define glutMotionFunc        Regress::GlutMotionFunc
#define glutPassiveMotionFunc Regress::GlutPassiveMotionFunc

#endif

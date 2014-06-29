/*

	Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_util.h"
#include "ogldev_glut_backend.h"
#include "ogldev_keys.h"

// Points to the object implementing the ICallbacks interface which was delivered to
// GLUTBackendRun(). All events are forwarded to this object.
static ICallbacks* s_pCallbacks = NULL;

static bool sWithDepth = false;
static bool sWithStencil = false;

static OGLDEV_KEY GLUTKeyToOGLDEVKey(uint Key)
{
    switch (Key) {
        case GLUT_KEY_INSERT:         
            return OGLDEV_KEY_INSERT;
        case GLUT_KEY_DELETE:        
            return OGLDEV_KEY_DELETE;
        case GLUT_KEY_RIGHT:         
            return OGLDEV_KEY_RIGHT;
        case GLUT_KEY_LEFT:         
            return OGLDEV_KEY_LEFT;
        case GLUT_KEY_DOWN:        
            return OGLDEV_KEY_DOWN;            
        case GLUT_KEY_UP:         
            return OGLDEV_KEY_UP;
        case GLUT_KEY_PAGE_UP:   
            return OGLDEV_KEY_PAGE_UP;
        case GLUT_KEY_PAGE_DOWN:      
            return OGLDEV_KEY_PAGE_DOWN;
        case GLUT_KEY_HOME:    
            return OGLDEV_KEY_HOME;
        case GLUT_KEY_END:     
            return OGLDEV_KEY_END;
        case GLUT_KEY_F1:        
            return OGLDEV_KEY_F1;
        case GLUT_KEY_F2:        
            return OGLDEV_KEY_F2;
        case GLUT_KEY_F3:       
            return OGLDEV_KEY_F3;
        case GLUT_KEY_F4:   
            return OGLDEV_KEY_F4;
        case GLUT_KEY_F5:      
            return OGLDEV_KEY_F5;
        case GLUT_KEY_F6:     
            return OGLDEV_KEY_F6;
        case GLUT_KEY_F7:     
            return OGLDEV_KEY_F7;
        case GLUT_KEY_F8:     
            return OGLDEV_KEY_F8;
        case GLUT_KEY_F9:     
            return OGLDEV_KEY_F9;
        case GLUT_KEY_F10:    
            return OGLDEV_KEY_F10;
        case GLUT_KEY_F11:   
            return OGLDEV_KEY_F11;
        case GLUT_KEY_F12:    
            return OGLDEV_KEY_F12;
        default:
            OGLDEV_ERROR("Unimplemented OGLDEV key");
            exit(1);
    }
    
    return OGLDEV_KEY_UNDEFINED;
}


static void SpecialKeyboardCB(int Key, int x, int y)
{
    OGLDEV_KEY OgldevKey = GLUTKeyToOGLDEVKey(Key);
    s_pCallbacks->KeyboardCB(OgldevKey);
}


static void KeyboardCB(unsigned char Key, int x, int y)
{
    OGLDEV_KEY OgldevKey = (OGLDEV_KEY)Key;
    s_pCallbacks->KeyboardCB(OgldevKey);
}


static void PassiveMouseCB(int x, int y)
{
    s_pCallbacks->PassiveMouseCB(x, y);
}


static void RenderSceneCB()
{
    s_pCallbacks->RenderSceneCB();
}


static void IdleCB()
{
    s_pCallbacks->IdleCB();
}


static void MouseCB(int Button, int State, int x, int y)
{
    s_pCallbacks->MouseCB(Button, State, x, y);
}


static void InitCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(IdleCB);
    glutSpecialFunc(SpecialKeyboardCB);
    glutPassiveMotionFunc(PassiveMouseCB);
    glutKeyboardFunc(KeyboardCB);
    glutMouseFunc(MouseCB);
}


void GLUTBackendInit(int argc, char** argv, bool WithDepth, bool WithStencil)
{
    sWithDepth = WithDepth;
    sWithStencil = WithStencil;

    glutInit(&argc, argv);
	
    uint DisplayMode = GLUT_DOUBLE|GLUT_RGBA;

    if (WithDepth) {
        DisplayMode |= GLUT_DEPTH;
    }

    if (WithStencil) {
        DisplayMode |= GLUT_STENCIL;
    }

    glutInitDisplayMode(DisplayMode);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
}


bool GLUTBackendCreateWindow(unsigned int Width, unsigned int Height, bool isFullScreen, const char* pTitle)
{
    if (isFullScreen) {
        char ModeString[64] = { 0 };
        int bpp = 32;
        SNPRINTF(ModeString, sizeof(ModeString), "%dx%d@%d", Width, Height, bpp);
        glutGameModeString(ModeString);
        glutEnterGameMode();
    }
    else {
        glutInitWindowSize(Width, Height);
        glutCreateWindow(pTitle);
    }

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return false;
    }

    return true;
}

void GLUTBackendRun(ICallbacks* pCallbacks)
{
    if (!pCallbacks) {
        fprintf(stderr, "%s : callbacks not specified!\n", __FUNCTION__);
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    if (sWithDepth) {
            glEnable(GL_DEPTH_TEST);
    }

    s_pCallbacks = pCallbacks;
    InitCallbacks();
    glutMainLoop();
}
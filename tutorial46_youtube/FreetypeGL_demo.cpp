/*

        Copyright 2023 Etay Meiri

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

    FreetypeGL demo
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>


#include "ogldev_util.h"
#include "ogldev_glfw.h"
#include "../Common/FreetypeGL/freetypeGL.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class FreetypeGLDemo
{
public:

    FreetypeGLDemo()
    {
    }


    virtual ~FreetypeGLDemo()
    {
    }


    void Init()
    {
        _CreateWindow();

        InitCallbacks();

        m_fontRenderer.InitFontRenderer(WINDOW_WIDTH, WINDOW_HEIGHT);
	}


    void Run()
    {
        while (!glfwWindowShouldClose(window)) {
            RenderSceneCB();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }


    void RenderSceneCB()
    {
        glClearColor(0.35f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static int x = 0;
        int y = x;

        m_fontRenderer.RenderText(FONT_TYPE_AMIRI, blue, red, 50, 30, "FreetypeGL!");
        m_fontRenderer.RenderText(FONT_TYPE_LIBERASTIKA, white, blue, 50, 150, "FreetypeGL!");
        m_fontRenderer.RenderText(FONT_TYPE_LOBSTER, black, white, 50, 280, "FreetypeGL!");
        m_fontRenderer.RenderText(FONT_TYPE_LUCKIEST_GUY, yellow, black, 50, 390, "FreetypeGL!");
        m_fontRenderer.RenderText(FONT_TYPE_OLD_STANDARD, orange1, yellow, 50, 500, "FreetypeGL!");
        m_fontRenderer.RenderText(FONT_TYPE_SOURCE_CODE_PRO, orange2, orange1, 50, 610, "FreetypeGL!");
        m_fontRenderer.RenderText(FONT_TYPE_SOURCE_SANS_PRO, green, orange2, 50, 720, "FreetypeGL!");
        m_fontRenderer.RenderText(FONT_TYPE_VERA, gray, green, 50, 840, "FreetypeGL!");
        m_fontRenderer.RenderText(FONT_TYPE_VERA_MOBD, purple, gray, 50, 950, "FreetypeGL!");
        m_fontRenderer.RenderText(FONT_TYPE_VERA_MOBI, cyan, purple, 50, 1090, "FreetypeGL!");
    }


    void PassiveMouseCB(int x, int y)
    {
    }

    void KeyboardCB(uint key, int state)
    {
        if (state == GLFW_PRESS) {
            switch (key) {
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                glfwDestroyWindow(window);
                glfwTerminate();
                exit(0);
            }
        }
    }


    void MouseCB(int button, int action, int x, int y)
    {
    }


private:

    void _CreateWindow()
    {
        int major_ver = 0;
        int minor_ver = 0;
        bool is_full_screen = false;
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "FreetypeGL demo");

        glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    }


    void InitCallbacks()
    {
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
    }

    GLFWwindow* window = NULL;
    FontRenderer m_fontRenderer;
};


static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
}


static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
}


static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode)
{
}


int main(int argc, char** argv)
{
    FreetypeGLDemo app;

    app.Init();

    app.Run();

    return 0;
}

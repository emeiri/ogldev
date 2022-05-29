/*

        Copyright 2021 Etay Meiri

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

    FreeGLUT sample - creating multiple windows
*/

#include <GL/freeglut.h>
#include <stdio.h>


static void RenderSceneCB_win1()
{
    GLclampf Red = 1.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}


static void RenderSceneCB_win2()
{
    GLclampf Red = 0.0f, Green = 1.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);

    int width = 1920;
    int height = 1080;
    glutInitWindowSize(width, height);

    int x = 0;
    int y = 0;
    glutInitWindowPosition(x, y);
    int win1 = glutCreateWindow("Window #1");
    printf("Window #1 id: %d\n", win1);

    width = 1024;
    height = 768;
    glutInitWindowSize(width, height);

    x = 200;
    y = 300;
    glutInitWindowPosition(x, y);
    int win2 = glutCreateWindow("Window #2");
    printf("Window #2 id: %d\n", win2);

    glutSetWindow(win1);
    glutDisplayFunc(RenderSceneCB_win1);

    glutSetWindow(win2);
    glutDisplayFunc(RenderSceneCB_win2);

    glutMainLoop();

    return 0;
}

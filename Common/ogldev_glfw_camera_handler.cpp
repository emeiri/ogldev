/*

        Copyright 2024 Etay Meiri

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

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "ogldev_glfw_camera_handler.h"

bool GLFWCameraHandler(CameraMovement& Movement, int Key, int Action, int Mods)
{
    bool Press = Action != GLFW_RELEASE;

    bool Handled = true;

    switch (Key) {

    case GLFW_KEY_W:
        Movement.Forward = Press;
        break;

    case GLFW_KEY_S:
        Movement.Backward = Press;
        break;

    case GLFW_KEY_A:
        Movement.StrafeLeft = Press;
        break;

    case GLFW_KEY_D:
        Movement.StrafeRight = Press;
        break;

    case GLFW_KEY_PAGE_UP:
        Movement.Up = Press;
        break;

    case GLFW_KEY_PAGE_DOWN:
        Movement.Down = Press;
        break;

    case GLFW_KEY_KP_ADD:
        Movement.Plus = Press;
        break;

    case GLFW_KEY_KP_SUBTRACT:
        Movement.Minus = Press;
        break;

    default:
        Handled = false;
    }

    if (Mods & GLFW_MOD_SHIFT) {
        Movement.FastSpeed = Press;
    }

    return Handled;
}
/*

        Copyright 2022 Etay Meiri

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

#pragma once

struct CameraMovement {
    bool Forward = false;
    bool Backward = false;
    bool StrafeLeft = false;
    bool StrafeRight = false;
    bool Up = false;
    bool Down = false;
    bool FastSpeed = false;
    bool Plus = false;
    bool Minus = false;
};


bool GLFWCameraHandler(CameraMovement& Movement, int Key, int Action, int Mods);

//#endif

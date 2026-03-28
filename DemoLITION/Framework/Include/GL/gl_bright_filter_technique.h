/*

        Copyright 2026 Etay Meiri

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

#include "gl_full_screen_technique.h"

#define DEF_LOC(name) GLuint m_##name##Loc = -1

class BrightFilterTechnique : public FullScreenTechnique
{
public:
    BrightFilterTechnique() {};

    bool Init();

private:

    DEF_LOC(gSampler);
    DEF_LOC(gLuminanceThreshold);
};

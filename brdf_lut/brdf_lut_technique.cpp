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

#include "brdf_lut_technique.h"


BRDF_LUT_Technique::BRDF_LUT_Technique()
{
}

bool BRDF_LUT_Technique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_COMPUTE_SHADER, "brdf_lut.cs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return true;
}

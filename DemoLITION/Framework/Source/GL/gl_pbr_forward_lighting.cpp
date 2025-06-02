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

//#define FAIL_ON_MISSING_LOC


#include "GL/gl_pbr_forward_lighting.h"

PBRForwardLightingTechnique::PBRForwardLightingTechnique()
{
}

bool PBRForwardLightingTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/pbr_forward_lighting.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/pbr_forward_lighting.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    if (!BaseLightingTechnique::Init()) {
        return false;
    }

    return true;
}


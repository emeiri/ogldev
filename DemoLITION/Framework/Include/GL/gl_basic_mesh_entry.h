/*

        Copyright 2025 Etay Meiri

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

#include <string>

#include "ogldev_math_3d.h"

struct BasicMeshEntry {
    uint NumIndices = 0;
    uint NumVertices = 0;
    uint BaseVertex = 0;
    uint BaseIndex = 0;
    uint ValidFaces = 0;
    int MaterialIndex = -1;
    // Moved to the node hierarchy. This is still here to ease the transition.
    Matrix4f TransformationDeprecated;
    std::string Name;
    glm::vec3 Size = glm::vec3(0.0f);
    glm::vec3 BboxMin = glm::vec3(0.0f);
    glm::vec3 BboxMax = glm::vec3(0.0f);
};

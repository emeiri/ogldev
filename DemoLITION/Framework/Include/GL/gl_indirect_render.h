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

#include <vector>

#include "ogldev_math_3d.h"
#include "ogldev_material.h"
#include "GL/gl_basic_mesh_entry.h"

class IndirectRender {

public:
    
    IndirectRender() {}

    ~IndirectRender() {}

    void Init(const std::vector<BasicMeshEntry>& Meshes, const std::vector<Material>& Materials);

    void Render(const Matrix4f& ObjectMatrix);

    void RefreshMaterials(const std::vector<Material>& Materials);

private:

    void InitMeshes(const std::vector<BasicMeshEntry>& Meshes);

    void InitDrawCmdsBuffer(const std::vector<BasicMeshEntry>& Meshes);

    void AllocPerObjectBuffer(const std::vector<BasicMeshEntry>& Meshes);

    void UpdatePerObjectData(const Matrix4f& ObjectMatrix);

    void PrepareIndirectRenderMaterials(const std::vector<Material>& Materials);

    struct MaterialColorIndirect {
        Vector4f AmbientColor;
        Vector4f DiffuseColor;
        Vector4f SpecularColor;
    };

    std::vector<MaterialColorIndirect> m_colors;
    std::vector<GLuint64> m_diffuseMaps;
    std::vector<GLuint64> m_normalMaps;

    GLuint m_drawCmdBuffer = 0;
    GLuint m_perObjectBuffer = 0;
    GLuint m_colorsBuffer = 0;
    GLuint m_diffuseMapBuffer = 0;
    GLuint m_normalMapBuffer = 0;

    struct Mesh {
        Matrix4f m_transformation;
        int m_materialIndex = 0;
    };

    std::vector<Mesh> m_meshes;
};
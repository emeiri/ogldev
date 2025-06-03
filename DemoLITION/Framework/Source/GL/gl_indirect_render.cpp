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

#include "GL/gl_ssbo_db.h"
#include "GL/gl_indirect_render.h"

struct PerObjectData {
    Matrix4f WorldMatrix;
    Matrix4f NormalMatrix;
    glm::ivec4 MaterialIndex = glm::ivec4(0);
};

struct DrawElementsIndirectCommand {
    unsigned int  Count = 0;
    unsigned int  InstanceCount = 0;
    unsigned int  FirstIndex = 0;
    int           BaseVertex = 0;
    unsigned int  BaseInstance = 0;
};


void IndirectRender::Init(const std::vector<BasicMeshEntry>& Meshes, const std::vector<Material>& Materials)
{
    assert(Meshes.size() > 0);

    InitMeshes(Meshes);

    InitDrawCmdsBuffer(Meshes);

    AllocPerObjectBuffer(Meshes);

    PrepareIndirectRenderMaterials(Materials);
}


void IndirectRender::InitMeshes(const std::vector<BasicMeshEntry>& Meshes)
{
    m_meshes.resize(Meshes.size());

    for (int i = 0; i < Meshes.size(); i++) {
        m_meshes[i].m_transformation = Meshes[i].Transformation;
        m_meshes[i].m_materialIndex = Meshes[i].MaterialIndex;
    }
}


void IndirectRender::InitDrawCmdsBuffer(const std::vector<BasicMeshEntry>& Meshes)
{
    std::vector<DrawElementsIndirectCommand> DrawCommands;
    DrawCommands.resize(Meshes.size());

    for (int i = 0; i < Meshes.size(); i++) {
        DrawElementsIndirectCommand Cmd;
        Cmd.Count = Meshes[i].NumIndices;
        Cmd.InstanceCount = 1;
        Cmd.FirstIndex = Meshes[i].BaseIndex;
        Cmd.BaseVertex = Meshes[i].BaseVertex;
        Cmd.BaseInstance = 0;

        DrawCommands[i] = Cmd;
    }

    glCreateBuffers(1, &m_drawCmdBuffer);
    glNamedBufferStorage(m_drawCmdBuffer, sizeof(DrawCommands[0]) * DrawCommands.size(),
                         (const void*)DrawCommands.data(), 0);
}


void IndirectRender::AllocPerObjectBuffer(const std::vector<BasicMeshEntry>& Meshes)
{
    glCreateBuffers(1, &m_perObjectBuffer);
    glNamedBufferStorage(m_perObjectBuffer, sizeof(PerObjectData) * Meshes.size(), 
                         NULL, GL_DYNAMIC_STORAGE_BIT);
}


void IndirectRender::PrepareIndirectRenderMaterials(const std::vector<Material>& Materials)
{
    int NumMaterials = (int)Materials.size();

    m_materials.resize(NumMaterials);

    for (int i = 0; i < NumMaterials; i++) {
        m_materials[i].Color.AmbientColor = Materials[i].AmbientColor;
        m_materials[i].Color.DiffuseColor = Materials[i].DiffuseColor;
        m_materials[i].Color.SpecularColor = Materials[i].SpecularColor;

        if (Materials[i].pTextures[TEX_TYPE_BASE] && (Materials[i].pTextures[TEX_TYPE_BASE]->GetBindlessHandle() == -1)) {
            printf("Diffuse texture exists but bindless handle is missing\n");
            exit(1);
        }
        GLuint64 DiffuseMapBindlessHandle = Materials[i].pTextures[TEX_TYPE_BASE] ? Materials[i].pTextures[TEX_TYPE_BASE]->GetBindlessHandle() : -1;
        m_materials[i].DiffuseMap = DiffuseMapBindlessHandle;
        GLuint64 NormalMapBindlessHandle = Materials[i].pTextures[TEX_TYPE_NORMAL] ? Materials[i].pTextures[TEX_TYPE_NORMAL]->GetBindlessHandle() : -1;
        m_materials[i].NormalMap = NormalMapBindlessHandle;
    }

    glCreateBuffers(1, &m_materialsBuffer);
    glNamedBufferStorage(m_materialsBuffer, ARRAY_SIZE_IN_BYTES(m_materials), m_materials.data(), GL_DYNAMIC_STORAGE_BIT);
}



void IndirectRender::Render(const Matrix4f& ObjectMatrix)
{
    UpdatePerObjectData(ObjectMatrix);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_drawCmdBuffer);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, 
                                (GLsizei)m_meshes.size(), 0);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void IndirectRender::RefreshMaterials(const std::vector<Material>& Materials)
{
    int NumMaterials = (int)Materials.size();

    for (int i = 0; i < NumMaterials; i++) {
        m_materials[i].Color.AmbientColor = Materials[i].AmbientColor;
        m_materials[i].Color.DiffuseColor = Materials[i].DiffuseColor;
        m_materials[i].Color.SpecularColor = Materials[i].SpecularColor;
        if (Materials[i].pTextures[TEX_TYPE_BASE] && (Materials[i].pTextures[TEX_TYPE_BASE]->GetBindlessHandle() == -1)) {
            printf("Diffuse texture exists but bindless handle is missing\n");
            exit(1);
        }
        GLuint64 DiffuseMapBindlessHandle = Materials[i].pTextures[TEX_TYPE_BASE] ? Materials[i].pTextures[TEX_TYPE_BASE]->GetBindlessHandle() : -1;
        m_materials[i].DiffuseMap = DiffuseMapBindlessHandle;
        GLuint64 NormalMapBindlessHandle = Materials[i].pTextures[TEX_TYPE_NORMAL] ? Materials[i].pTextures[TEX_TYPE_NORMAL]->GetBindlessHandle() : -1;
        m_materials[i].NormalMap = NormalMapBindlessHandle;
    }

    glNamedBufferSubData(m_materialsBuffer, 0, ARRAY_SIZE_IN_BYTES(m_materials), m_materials.data());
}


void IndirectRender::UpdatePerObjectData(const Matrix4f& ObjectMatrix)
{
    std::vector<PerObjectData> PerObjectDataVector;
    PerObjectDataVector.resize(m_meshes.size());

    for (int i = 0; i < m_meshes.size(); i++) {
        // TODO: move to math3d
        Matrix4f FinalWorldMatrix = ObjectMatrix * m_meshes[i].m_transformation;
        Matrix4f WorldInverse = FinalWorldMatrix.Inverse();
        Matrix4f WorldInverseTranspose = WorldInverse.Transpose();

        PerObjectDataVector[i].WorldMatrix = FinalWorldMatrix;
        PerObjectDataVector[i].NormalMatrix = WorldInverseTranspose;
        PerObjectDataVector[i].MaterialIndex.x = m_meshes[i].m_materialIndex;
    }

    glNamedBufferSubData(m_perObjectBuffer, 0, ARRAY_SIZE_IN_BYTES(PerObjectDataVector), 
                         PerObjectDataVector.data());

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_PER_OBJ_DATA, m_perObjectBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_MATERIALS, m_materialsBuffer);
}

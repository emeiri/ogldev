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

#include "Int/core_model.h"
#include "GL/gl_indirect_render.h"

class GLModel : public CoreModel
{
public:
    GLModel();

    GLModel(CoreRenderingSystem* pCoreRenderingSystem) : CoreModel(pCoreRenderingSystem) {}

    ~GLModel();

    virtual void ConvertToMesh();

    void Render(DemolitionRenderCallbacks* pRenderCallbacks);

    void Render(uint DrawIndex, uint PrimID);

    void Render(uint NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats);

    virtual void SetColorTexture(int TextureHandle);

    virtual void SetNormalMap(int TextureHandle);

    virtual void SetHeightMap(int TextureHandle);

    virtual Texture* AllocTexture2D();

    virtual void DestroyTexture(Texture* pTexture);

    void RenderIndirect(const Matrix4f& ObjectMatrix);

    Texture* GetNormalMap() const { return m_pNormalMap; }

    Texture* GetHeightMap() const { return m_pHeightMap; }

protected:

    virtual void AllocBuffers();

    virtual void PopulateBuffersSkinned(vector<SkinnedVertex>& Vertices);

    virtual void PopulateBuffers(vector<Vertex>& Vertices);

    virtual void InitGeometryPost();

private:

    template<typename VertexType>
    void PopulateBuffersInternal(vector<VertexType>& Vertices);

    template<typename VertexType>
    void PopulateBuffersPVP(vector<VertexType>& Vertices);

    template<typename VertexType>
    void PopulateBuffersNonDSA(vector<VertexType>& Vertices);

    template<typename VertexType>
    void PopulateBuffersDSA(vector<VertexType>& Vertices);

    void SetupRenderMaterialsPBR();

    void RenderMesh(int MeshIndex, DemolitionRenderCallbacks* pRenderCallbacks = NULL);

    GLuint m_VAO = 0;

    GLuint m_Buffers[NUM_BUFFERS] = { 0 };

    size_t m_vertexBufferSizeBytes = 0;
    size_t m_indexBufferSizeBytes = 0;

    IndirectRender m_indirectRender;

    Texture* m_pNormalMap = NULL;
    Texture* m_pHeightMap = NULL;
};

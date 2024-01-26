/*

        Copyright 2023 Etay Meiri

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


#ifndef GEOMIP_GRID_H
#define GEOMIP_GRID_H

#include <GL/glew.h>
#include <vector>

#include "ogldev_math_3d.h"
#include "lod_manager.h"

// this header is included by terrain.h so we have a forward 
// declaration for BaseTerrain.
class BaseTerrain;

class GeomipGrid {
 public:
    GeomipGrid();

    ~GeomipGrid();

    void CreateGeomipGrid(int Width, int Depth, int PatchSize, const BaseTerrain* pTerrain);

    void Destroy();

    void Render(const Vector3f& CameraPos, const Matrix4f& ViewProj);

 private:

    struct Vertex {
        Vector3f Pos;
        Vector2f Tex;
        Vector3f Normal = Vector3f(0.0f, 0.0f, 0.0f);

        void InitVertex(const BaseTerrain* pTerrain, int x, int z);
    };

    void CreateGLState();
	
    void PopulateBuffers(const BaseTerrain* pTerrain);
    
    void InitVertices(const BaseTerrain* pTerrain, std::vector<Vertex>& Vertices);
   
    int InitIndices(std::vector<uint>& Indices);
    
    int InitIndicesLOD(int Index, std::vector<uint>& Indices, int lod);
    
    int InitIndicesLODSingle(int Index, std::vector<uint>& Indices, int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom);
    
    void CalcNormals(std::vector<Vertex>& Vertices, std::vector<uint>& Indices);
    
    uint AddTriangle(uint Index, std::vector<uint>& Indices, uint v1, uint v2, uint v3);
    
    uint CreateTriangleFan(int Index, std::vector<uint>& Indices, int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom, int x, int z);

    int CalcNumIndices();

    bool IsPatchInsideViewFrustum_ViewSpace(int X, int Z, const Matrix4f& ViewProj);

    bool IsPatchInsideViewFrustum_WorldSpace(int X, int Z, const FrustumCulling& FC);

    int m_width = 0;
    int m_depth = 0;
    int m_patchSize = 0;
    int m_maxLOD = 0;
    GLuint m_vao = 0;
    GLuint m_vb = 0;
    GLuint m_ib = 0;
    float m_worldScale = 1.0f;

    struct SingleLodInfo {
        int Start = 0;
        int Count = 0;
    };
	
    #define LEFT   2
    #define RIGHT  2
    #define TOP    2
    #define BOTTOM 2
	
    struct LodInfo {
        SingleLodInfo info[LEFT][RIGHT][TOP][BOTTOM];
    };
	
    std::vector<LodInfo> m_lodInfo;
    int m_numPatchesX = 0;
    int m_numPatchesZ = 0;
    LodManager m_lodManager;
    const BaseTerrain* m_pTerrain = NULL;
};

#endif

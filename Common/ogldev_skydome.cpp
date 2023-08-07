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

#include <vector>

#include "ogldev_skydome.h"


Skydome::Skydome(int NumRows, int NumCols, float Radius, const char* pTextureFilename, GLenum TextureUnit, int TextureUnitIndex)  : m_texture(GL_TEXTURE_2D)
{
    m_textureUnit = TextureUnit;
    m_textureUnitIndex = TextureUnitIndex;

    CreateGLState();

    PopulateBuffers(NumRows, NumCols, Radius);

    LoadTexture(pTextureFilename);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!m_skydomeTech.Init()) {
        exit(0);
    }

    m_skydomeTech.Enable();

    m_skydomeTech.SetTextureUnit(TextureUnitIndex);
}



Skydome::Vertex::Vertex(const Vector3f& p)
{
    Pos = p;

    Vector3f pn = p;
    pn.Normalize();

    Tex.x = asinf(pn.x) / (float)M_PI + 0.5f;
    Tex.y = asinf(pn.y) / (float)M_PI + 0.5f;
}


void Skydome::CreateGLState()
{
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vb);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);

    int pos_loc = 0;
    int tex_loc = 1;

    size_t Offset = 0;
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)Offset);
    Offset += 3;

    glEnableVertexAttribArray(tex_loc);
    glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(Offset * sizeof(float)));
    Offset += 2;
}


void Skydome::PopulateBuffers(int NumRows, int NumCols, float Radius)
{
    int NumVerticesTopStrip = 3 * NumCols;
    int NumVerticesRegularStrip = 6 * NumCols;
    m_numVertices = NumVerticesTopStrip + (NumRows - 1) * NumVerticesRegularStrip;

    std::vector<Vertex> Vertices(m_numVertices);

    float PitchAngle = 90.0f / (float)NumRows;
    float HeadingAngle = 360.0f / (float)NumCols;

    Vector3f Apex(0.0f, Radius, 0.0f);

    float Pitch = -90.0f;
    int i = 0;

    for (float Heading = 0.0f ; Heading < 360.0f ; Heading += HeadingAngle) {

        Vertex v0(Apex);
        Vertices[i++] = v0; 

        Vector3f Pos1;
        Pos1.InitBySphericalCoords(Radius, Pitch + PitchAngle, Heading + HeadingAngle);
        Vertex v1(Pos1);
        Vertices[i++] = v1;

        Vector3f Pos2;
        Pos2.InitBySphericalCoords(Radius, Pitch + PitchAngle, Heading);
        Vertex v2(Pos2);
        Vertices[i++] = v2;
    }

    for (Pitch = -90.0f + PitchAngle; Pitch < 0 ; Pitch += PitchAngle) {
        for (float Heading = 0.0f; Heading < 360.0f ; Heading += HeadingAngle) {

            Vector3f Pos0;
            Pos0.InitBySphericalCoords(Radius, Pitch, Heading);
            Vertex v0(Pos0);

            Vector3f Pos1;
            Pos1.InitBySphericalCoords(Radius, Pitch, Heading + HeadingAngle);
            Vertex v1(Pos1);

            Vector3f Pos2;
            Pos2.InitBySphericalCoords(Radius, Pitch + PitchAngle, Heading);
            Vertex v2(Pos2);

            Vector3f Pos3;
            Pos3.InitBySphericalCoords(Radius, Pitch + PitchAngle, Heading + HeadingAngle);
            Vertex v3(Pos3);

            assert(i + 6 <= m_numVertices);

            Vertices[i++] = v0;
            Vertices[i++] = v1;
            Vertices[i++] = v2;

            Vertices[i++] = v1;
            Vertices[i++] = v3;
            Vertices[i++] = v2;
        }
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
}


void Skydome::LoadTexture(const char* pTextureFilename)
{
    m_texture.Load(pTextureFilename);
}


void Skydome::Render(const BasicCamera& Camera)
{
    m_skydomeTech.Enable();

    Matrix4f Rotate;

    static float foo = 0.0f;
    foo += 0.02f;
    Rotate.InitRotateTransform(0.0f, foo, 0.0f);
    Matrix4f World;
    World.InitTranslationTransform(Camera.GetPos() - Vector3f(0.0f, 0.2f, 0.0f));
    Matrix4f View = Camera.GetMatrix();
    Matrix4f Proj = Camera.GetProjectionMat();
    Matrix4f WVP = Proj * View * World * Rotate;
    m_skydomeTech.SetWVP(WVP);

    m_texture.Bind(m_textureUnit);
    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_numVertices);
    glBindVertexArray(0);

    glDepthFunc(OldDepthFuncMode);
}

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


Skydome::Skydome(int NumPitchStripes, int NumHeadingStripes, float Radius, const char* pTextureFilename, GLenum TextureUnit, int TextureUnitIndex)  : m_texture(GL_TEXTURE_2D)
{
    m_textureUnit = TextureUnit;
    m_textureUnitIndex = TextureUnitIndex;

    CreateGLState();

    PopulateBuffers(NumPitchStripes, NumHeadingStripes, Radius);

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

    //Pos.Print();

    Vector3f pn = p;
    pn.Normalize();

    Tex.x = atan2(pn.x, pn.z) / ((float)M_PI * 2.0f) + 0.5f;
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


void Skydome::PopulateBuffers(int NumPitchStripes, int NumHeadingStripes, float Radius)
{
//#define DTOR (float)M_PI / 180.0f

    int NumVerticesTopStripe = 3 * NumHeadingStripes;
    int NumVerticesRegularStripe = 6 * NumHeadingStripes;
    m_numVertices = NumVerticesTopStripe + (NumPitchStripes - 1) * NumVerticesRegularStripe;

    std::vector<Vertex> Vertices(m_numVertices);

    float PitchAngle = 90.0f / (float)NumPitchStripes;
    float HeadingAngle = 360.0f / (float)NumHeadingStripes;

    Vector3f Apex(0.0f, Radius, 0.0f);

    float Pitch = 90.0f;
    int i = 0;

    for (float Heading = 0.0f ; Heading <= (360.0f - HeadingAngle) ; Heading += HeadingAngle) {
        printf("TH %f\n", Heading);

        Vertex v0(Apex);
        Vertices[i++] = v0; 
        Apex.Print();

        Vector3f Pos1;
        Pos1.InitBySphericalCoords(Radius, Pitch - PitchAngle, Heading);
        Vertex v1(Pos1);
        Vertices[i++] = v1;
        Pos1.Print();

        Vector3f Pos2;
        Pos2.InitBySphericalCoords(Radius, Pitch - PitchAngle, Heading + HeadingAngle);
        Vertex v2(Pos2);
        Vertices[i++] = v2;
        Pos2.Print();

        printf("\n");
    }

    for (Pitch = 90.0f - PitchAngle; Pitch >= 0; Pitch -= PitchAngle) {
        for (float Heading = 0; Heading <= (360.0f - HeadingAngle); Heading += HeadingAngle) {
            printf("TH %f\n", Heading);

            Vertex v0(Apex);
            Vertices[i++] = v0;
            Apex.Print();

            Vector3f Pos1;
            Pos1.InitBySphericalCoords(Radius, Pitch - PitchAngle, Heading);
            Vertex v1(Pos1);
            Vertices[i++] = v1;
            Pos1.Print();

            Vector3f Pos2;
            Pos2.InitBySphericalCoords(Radius, Pitch - PitchAngle, Heading + HeadingAngle);
            Vertex v2(Pos2);
            Vertices[i++] = v2;
            Pos2.Print();

            printf("\n");
        }
    }

    
    //fix the texture-seam problem
        #if 0
        for (int i = 0 ; i < m_numVertices - 3 ; i++) {
            int i0 = i;
            int i1 = i + 1;
            int i2 = i + 2;

            if ((Vertices[i0].Tex.u - Vertices[i1].Tex.u) > 0.9f) {
                Vertices[i1].Tex.u += 1.0f;
            }

            if ((Vertices[i1].Tex.u - Vertices[i0].Tex.u) > 0.9f) {
                Vertices[i0].Tex.u += 1.0f;
            }

            if ((Vertices[i0].Tex.u - Vertices[i2].Tex.u) > 0.9f) {
                Vertices[i2].Tex.u += 1.0f;
            }

            if ((Vertices[i2].Tex.u - Vertices[i0].Tex.u) > 0.9f) {
                Vertices[i0].Tex.u += 1.0f;
            }

            if ((Vertices[i1].Tex.u - Vertices[i2].Tex.u) > 0.9f) {
                Vertices[i2].Tex.u += 1.0f;
            }

            if ((Vertices[i2].Tex.u - Vertices[i1].Tex.u) > 0.9f) {
                Vertices[i1].Tex.u += 1.0f;
            }

            if ((Vertices[i0].Tex.v - Vertices[i1].Tex.v ) > 0.8f) {
                Vertices[i1].Tex.v += 1.0f;
            }

            if ((Vertices[i1].Tex.v - Vertices[i0].Tex.v ) > 0.8f) {
                Vertices[i0].Tex.v += 1.0f;
            }

            if ((Vertices[i0].Tex.v - Vertices[i2].Tex.v ) > 0.8f) {
                Vertices[i2].Tex.v += 1.0f;
            }

            if ((Vertices[i2].Tex.v - Vertices[i0].Tex.v ) > 0.8f) {
                Vertices[i0].Tex.v += 1.0f;
            }

            if ((Vertices[i1].Tex.v - Vertices[i2].Tex.v ) > 0.8f) {
                Vertices[i2].Tex.v += 1.0f;
            }

            if ((Vertices[i2].Tex.v - Vertices[i1].Tex.v ) > 0.8f) {
                Vertices[i1].Tex.v += 1.0f;
            }
        }
        #endif

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
    foo += 0.1f;
    Rotate.InitRotateTransform(foo, 0.0f, 0.0f);
    Matrix4f World;
    World.InitTranslationTransform(Vector3f(0.0f, 0.0f, 5.0f));
    Matrix4f View = Camera.GetMatrix();
    Matrix4f Proj = Camera.GetProjectionMat();
    Matrix4f WVP = Proj * View * World;// *Rotate;
    m_skydomeTech.SetWVP(WVP);

    m_texture.Bind(m_textureUnit);

    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
    glDepthFunc(GL_LEQUAL);

    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);

    glCullFace(GL_FRONT);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 24);
    glBindVertexArray(0);

    glDepthFunc(OldDepthFuncMode);
    glCullFace(OldCullFaceMode);
}

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


Skydome::Skydome(float Theta, float Phi, float Radius, const char* pTextureFilename) : m_texture(GL_TEXTURE_2D)
{
    CreateGLState();

    PopulateBuffers(Theta, Phi, Radius);

    LoadTexture(pTextureFilename);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!m_texTech.Init()) {
        exit(0);
    }
    m_texTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
}


Skydome::Vertex::Vertex(const Vector3f& p)
{
    Pos = p;

    Vector3f pn = p;
    pn.Normalize();

    Tex.x = atan2(pn.x, pn.z) / (M_PI * 2) + 0.5f;
    Tex.y = asinf(pn.y) / M_PI + 0.5f;
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

    //    printf("GL state created\n");
}


void Skydome::PopulateBuffers(float Theta, float Phi, float Radius)
{
#define DTOR M_PI / 180.0f

    m_numVertices = (int)((360/Theta) * (90/Phi) * 4);

    std::vector<Vertex> Vertices(m_numVertices);
    int i = 0;

    for (int ph = 0 ; ph <= (90.0f - Phi) ; ph += (int)Phi) {

        for (int th = 0 ; th <= (360.0f - Theta) ; th += (int)Theta) {
            //compute the vertex at phi, theta
            Vector3f Pos0(Radius * sinf(ph * DTOR) * cosf(DTOR * th),
                          Radius * sinf(ph * DTOR) * sinf(DTOR * th),
                          Radius * cosf(ph * DTOR));

            Vertex v0(Pos0);
            Vertices[i++] = v0;

            //compute the vertex at phi+phi, theta
            Vector3f Pos1(Radius * sinf((ph + Phi) * DTOR) * cosf(th * DTOR),
                          Radius * sinf((ph + Phi) * DTOR) * sinf(th * DTOR),
                          Radius * cosf((ph + Phi) * DTOR));

            Vertex v1(Pos1);
            Vertices[i++] = v1;

            //compute the vertex at phi, theta+theta
            Vector3f Pos2(Radius * sinf(DTOR * ph) * cosf(DTOR * (th + Theta)),
                          Radius * sinf(DTOR * ph) * sinf(DTOR * (th + Theta)),
                          Radius * cosf(DTOR * ph));

            Vertex v2(Pos2);
            Vertices[i++] = v2;

            if (ph > -90.0f && ph < 90.0f) {
                Vector3f Pos3(Radius * sinf((ph + Phi) * DTOR) * cosf(DTOR * (th + Theta)),
                              Radius * sinf((ph + Phi) * DTOR) * sinf(DTOR * (th + Theta)),
                              Radius * cosf((ph + Phi) * DTOR));

                Vertex v3(Pos3);
                Vertices[i++] = v3;
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
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
}


void Skydome::LoadTexture(const char* pTextureFilename)
{
    m_texture.Load(pTextureFilename);
}


void Skydome::Render(const BasicCamera& Camera)
{
    m_texTech.Enable();

    Matrix4f Rotate;
    Rotate.InitRotateTransform(90.0f, 0.0f, 0.0f);
    Matrix4f World;
    World.InitTranslationTransform(Camera.GetPos() + Vector3f(0.0f, -0.5f, 0.0f));
    Matrix4f View = Camera.GetMatrix();
    Matrix4f Proj = Camera.GetProjectionMat();
    Matrix4f WVP = Proj * View * World * Rotate;
    m_texTech.SetWVP(WVP);

    m_texture.Bind(COLOR_TEXTURE_UNIT);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_numVertices);
    glBindVertexArray(0);
}

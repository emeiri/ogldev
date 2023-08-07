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

#ifndef OGLDEV_SKYDOME_H
#define OGLDEV_SKYDOME_H

#include <GL/glew.h>

#include "ogldev_skydome_technique.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_texture.h"

class Skydome
{
 public:
    Skydome(int NumPitchStripes, int NumHeadingStripes, float Radius, const char* pTextureFilanem, GLenum TextureUnit, int TextureUnitIndex);

    void Render(const BasicCamera& Camera);

 private:

    struct Vertex {
        Vector3f Pos;
        Vector2f Tex;

        Vertex() {}

        Vertex(const Vector3f& p);
    };

    void CreateGLState();

    void PopulateBuffers(int NumRows, int NumCols, float Radius);

    void LoadTexture(const char* pTextureFilanem);

    int m_numVertices = 0;
    GLuint m_vao;
    GLuint m_vb;
    Texture m_texture;
    SkydomeTechnique m_skydomeTech;
    int m_textureUnit = 0;
    GLenum m_textureUnitIndex = 0;
};


#endif

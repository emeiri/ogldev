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


#ifndef QUAD_ARRAY_H
#define QUAD_ARRAY_H

#include <GL/glew.h>
#include "ogldev.h"

class QuadArray
{
 public:
    QuadArray(uint MaxNumQuads);

    ~QuadArray();

    void Render(uint NumQuads = 0);

 private:

    void CreateVertexBuffer();
    void CreateQuadIDBuffer();

    enum BUFFER_TYPE {
        POS_VB       = 0,
        QUAD_ID_VB   = 1,
        NUM_BUFFERS  = 2
    };

    uint m_maxNumQuads = 0;
    GLuint m_VAO = -1;
    GLuint m_Buffers[NUM_BUFFERS] = { 0 };
};

#endif

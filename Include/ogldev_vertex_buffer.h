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


#pragma once

class VertexBuffer {
public:
    VertexBuffer() {}

    ~VertexBuffer() 
    {
        // TODO...
    }

    void Init(const std::vector<float>& Vertices, int NumVertexElements, GLuint TopologyType)
    {
        m_topologyType = TopologyType;
        m_numVertices = (int)Vertices.size();

        glCreateVertexArrays(1, &m_vao);
        glCreateBuffers(1, &m_vbo);

        glNamedBufferStorage(m_vbo, sizeof(Vertices[0]) * Vertices.size(), Vertices.data(), 0);

        glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, NumVertexElements * sizeof(float));

        int attrib_location = 0;
        glEnableVertexArrayAttrib(m_vao, attrib_location);
        glVertexArrayAttribFormat(m_vao, attrib_location, NumVertexElements, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_vao, attrib_location, 0);
    }


    void Update(const std::vector<float>& Vertices)
    {
        if (m_numVertices == 0) {
            printf("Vertex buffer cannot be updated before it is initialized\n");
            exit(0);
        }

        if (m_numVertices != Vertices.size()) {
            printf("Change in the number of vertex buffer vertices %d --> %zd\n", m_numVertices, Vertices.size());
            exit(0);
        }

        glNamedBufferStorage(m_vbo, sizeof(Vertices[0]) * Vertices.size(), Vertices.data(), 0);
    }


    void Render(int topology_type)
    {
        glBindVertexArray(m_vao);

        if (topology_type == GL_PATCHES) {
            glPatchParameteri(GL_PATCH_VERTICES, m_numVertices);
        }

        glDrawArrays(topology_type, 0, m_numVertices);

        glBindVertexArray(0);
    }


    void Render()
    {
        Render(m_topologyType);
    }

private:
    GLuint m_vbo = -1;
    GLuint m_vao = -1;
    int m_numVertices = 0;
    GLuint m_topologyType = -1;
};


class FullScreenVB : public VertexBuffer {
public:
    FullScreenVB() {}

    ~FullScreenVB() {}

    void Init()
    {
        std::vector<float> Vertices = { -1.0f, -1.0f,     // Bottom left
                                         1.0f,  1.0f,     // Top right
                                        -1.0f,  1.0f,     // Top left                                        
                                        -1.0f, -1.0f,     // Bottom left
                                         1.0f, -1.0f,     // Bottom right
                                         1.0f, 1.0f       // Top right
                                      };                  
                                        

        int NumVertexElements = 2;
        VertexBuffer::Init(Vertices, NumVertexElements, GL_TRIANGLES);
    }
};
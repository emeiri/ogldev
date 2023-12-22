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
    VertexBuffer()
    {

    }

    ~VertexBuffer()
    {

    }

    void Init(const std::vector<float>& Vertices, int NumVertexElements, GLuint TopologyType)
    {
        m_topologyType = TopologyType;
        m_numVertices = (int)Vertices.size();

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, NumVertexElements, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
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

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }


    void Render(int topology_type)
    {
        glBindVertexArray(m_vao);

        if (topology_type == GL_PATCHES) {
            glPatchParameteri(GL_PATCH_VERTICES, m_numVertices);
        }

        glDrawArrays(topology_type, 0, m_numVertices);
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

#pragma once

class VertexBuffer {
public:
    VertexBuffer()
    {

    }

    ~VertexBuffer()
    {

    }

    void Init(const std::vector<float>& Vertices)
    {
        m_numVertices = (int)Vertices.size();

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        glPatchParameteri(GL_PATCH_VERTICES, 4);
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
        if ((topology_type != GL_POINTS) && (topology_type != GL_PATCHES)) {
            printf("Invalid topology type 0x%x\n", topology_type);
            exit(1);
        }

        glBindVertexArray(m_vao);
        glDrawArrays(topology_type, 0, m_numVertices);
    }

private:
    GLuint m_vbo = -1;
    GLuint m_vao = -1;
    int m_numVertices = 0;
};

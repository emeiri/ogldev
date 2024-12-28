#include "ogldev_texture.h"
#include "data.h"

#include <vector>

static constexpr int brdfW = 256;
static constexpr int brdfH = 256;

static const uint32_t bufferSize = 2 * sizeof(float) * brdfW * brdfH;


Particles::Particles()
{
}

void Particles::Init()
{
    m_brdfTech.Init();
    m_brdfTech.Enable();
    InitBuffers();
}

void Particles::Render()
{
    glDispatchCompute(brdfW, brdfH, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void Particles::InitBuffers()
{
    GLuint InputBuf = 0;    

    glGenBuffers(1, &InputBuf);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, InputBuf);
    float f = 0.0f;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), &f, GL_DYNAMIC_DRAW);

    glCreateBuffers(1, &m_outputBuf);
    glNamedBufferStorage(m_outputBuf, bufferSize, m_outputData.data(), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);

    m_outputData.resize(bufferSize);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_outputBuf);
}


const std::vector<char>& Particles::GetOutput()
{
    // Map buffer for reading
    void* p = glMapNamedBufferRange(m_outputBuf, 0, bufferSize, GL_MAP_READ_BIT);

    if (!p) {
        printf("Map error\n");
        exit(1);
    }

    memcpy(m_outputData.data(), p, bufferSize);

    bool result = glUnmapNamedBuffer(m_outputBuf);

    return m_outputData;
}




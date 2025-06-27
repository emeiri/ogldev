#include <stdio.h>
#include <stdlib.h>

#include "GL/gl_buffer.h"

void GLBuffer::InitBuffer(GLsizeiptr size, const void* data, GLbitfield flags)
{
	m_size = size;
	glCreateBuffers(1, &m_buf);
	glNamedBufferStorage(m_buf, size, data, flags);
}


void GLBuffer::BindUBO(int BindIndex)
{
	glBindBufferRange(GL_UNIFORM_BUFFER, BindIndex, m_buf, 0, m_size);
}


void GLBuffer::Update(const void* pData, size_t size)
{
	if (size > m_size) {
		printf("Update size is larger than buffer %d > %d\n", (int)size, (int)m_size);
		exit(1);
	}

	glNamedBufferSubData(m_buf, 0, size, pData);
}

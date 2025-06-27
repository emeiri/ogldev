#pragma once

#include <GL/glew.h>

class GLBuffer
{
public:
	GLBuffer() {}
	
	void InitBuffer(GLsizeiptr size, const void* data, GLbitfield flags);

	void BindUBO(int BindIndex);

	void Update(const void* pData, size_t size);

	~GLBuffer() {};

	//GLuint GetBuffer() const { return m_buf; }

private:
	GLuint m_buf = 0;
	size_t m_size = 0;
};

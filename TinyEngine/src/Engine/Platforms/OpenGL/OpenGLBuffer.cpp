#include "pch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Engine
{
	static GLenum OpenGLVertexBufferUsage(VertexBufferUsage usage)
	{
		switch (usage)
		{
		case Engine::VertexBufferUsage::Static:
			return GL_STATIC_DRAW;
		case Engine::VertexBufferUsage::Dynamic:
			return GL_DYNAMIC_DRAW;
		default:
			ENGINE_ASSERT(false, "Unknown vertex buffer usage!");
			return 0;
		}
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, VertexBufferUsage usage)
		:m_Size(size), m_Usage(usage)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, m_Size, nullptr, OpenGLVertexBufferUsage(m_Usage));
	}

	//----------------------------------------------------------------------
	//OpenGLVertexBuffer
	//----------------------------------------------------------------------
	OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, uint32_t size, VertexBufferUsage usage)
		:m_Size(size), m_Usage(usage)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, m_Size, data, OpenGLVertexBufferUsage(m_Usage));
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		m_Size = size;
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	}

	//----------------------------------------------------------------------
	//OpenGLIndexBuffer
	//----------------------------------------------------------------------
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* data, uint32_t size):
		m_Size(size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Size, data, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void OpenGLIndexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
	{
		m_Size = size;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_Size, offset, data);
	}
}
#include "pch.h"
#include "OpenGLBuffer.h"
#include "Engine/Renderer/Renderer.h"

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
		Renderer::Submit([this]()
			{
				glCreateBuffers(1, &m_RendererID);
				glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
				glBufferData(GL_ARRAY_BUFFER, m_Size, nullptr, OpenGLVertexBufferUsage(m_Usage));
			}
		);
	}

	//----------------------------------------------------------------------
	//OpenGLVertexBuffer
	//----------------------------------------------------------------------
	OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, uint32_t size, VertexBufferUsage usage)
		:m_Size(size), m_Usage(usage)
	{
		m_LocalData = Buffer::Copy(data, size);

		Renderer::Submit([this]()
			{
				glCreateBuffers(1, &m_RendererID);
				glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
				glBufferData(GL_ARRAY_BUFFER, m_Size, m_LocalData.Data, OpenGLVertexBufferUsage(m_Usage));
			}
		);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]()
			{
				glDeleteBuffers(1, &rendererID);
			}
		);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		Renderer::Submit([this]()
			{
				glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			}
		);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		Renderer::Submit([this]()
			{
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		);
	}

	void OpenGLVertexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
	{
		m_LocalData = Buffer::Copy(data, size);
		m_Size = size;
		Renderer::Submit([this, offset]()
			{
				glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
				glBufferSubData(GL_ARRAY_BUFFER, offset, m_Size, m_LocalData.Data);
			}
		);
	}

	//----------------------------------------------------------------------
	//OpenGLIndexBuffer
	//----------------------------------------------------------------------
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* data, uint32_t size):
		m_Size(size)
	{
		m_LocalData = Buffer::Copy(data, size);

		Renderer::Submit([this]()
			{
				glCreateBuffers(1, &m_RendererID);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Size, m_LocalData.Data, GL_STATIC_DRAW);
			}
		);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]()
			{
				glDeleteBuffers(1, &rendererID);
			}
		);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		Renderer::Submit([this]()
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
			}
		);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		Renderer::Submit([this]()
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
		);
	}

	void OpenGLIndexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
	{
		m_LocalData = Buffer::Copy(data, size);
		m_Size = size;
		Renderer::Submit([this, offset]()
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_Size, offset, m_LocalData.Data);
			}
		);
	}
}
#include "pch.h"
#include "OpenGLIndexBuffer.h"
#include "Engine/Renderer/Renderer.h"

#include <glad/glad.h>

namespace Engine
{
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t size)
		:m_Size(size)
	{
		Renderer::Submit([this]()
			{
				glCreateBuffers(1, &m_RendererID);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Size, nullptr, GL_DYNAMIC_DRAW);

				RENDERCOMMAND_INFO("RenderCommand: Construct indexBuffer({0})", m_RendererID);
			}
		);
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(void* data, uint32_t size) :
		m_Size(size)
	{
		m_LocalData = Buffer::Copy(data, size);

		Renderer::Submit([this]()
			{
				glCreateBuffers(1, &m_RendererID);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Size, m_LocalData.Data, GL_STATIC_DRAW);

				RENDERCOMMAND_INFO("RenderCommand: Construct indexBuffer({0})", m_RendererID);
			}
		);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]()
			{
				RENDERCOMMAND_INFO("RenderCommand: Destroy indexBuffer({0})", rendererID);

				glDeleteBuffers(1, &rendererID);
			}
		);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		Renderer::Submit([this]()
			{
				RENDERCOMMAND_INFO("RenderCommand: Bind indexBuffer({0})", m_RendererID);

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
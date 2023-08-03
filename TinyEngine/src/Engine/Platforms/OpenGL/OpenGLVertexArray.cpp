#include "pch.h"
#include "OpenGLVertexArray.h"
#include "Engine/Renderer/Renderer.h"
#include <glad/glad.h>

namespace Engine
{

	OpenGLVertexArray::OpenGLVertexArray()
	{
		Renderer::Submit([this]()
			{
				glCreateVertexArrays(1, &m_RendererID);
				RENDERCOMMAND_TRACE("RenderCommand: Construct vertexArray({0})", m_RendererID);
			}
		);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Destroy vertexArray({0})", rendererID);
				glDeleteVertexArrays(1, &rendererID);
			}
		);
	}

	void OpenGLVertexArray::Bind()
	{
		Renderer::Submit([this]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Bind vertexArray({0})", m_RendererID);
				glBindVertexArray(m_RendererID);
			}
		);
	}

	void OpenGLVertexArray::Unbind()
	{
		Renderer::Submit([this]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Unind vertexArray({0})", m_RendererID);
				glBindVertexArray(0);
			}
		);
	}
}
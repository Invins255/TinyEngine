#include "pch.h"
#include "OpenGLVertexArray.h"
#include "Engine/Renderer/Renderer.h"
#include <glad/glad.h>

namespace Engine
{

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Engine::ShaderDataType::Float:		return GL_FLOAT;
		case Engine::ShaderDataType::Float2:	return GL_FLOAT;
		case Engine::ShaderDataType::Float3:	return GL_FLOAT;
		case Engine::ShaderDataType::Float4:	return GL_FLOAT;
		case Engine::ShaderDataType::Int:		return GL_INT;
		case Engine::ShaderDataType::Int2:		return GL_INT;
		case Engine::ShaderDataType::Int3:		return GL_INT;
		case Engine::ShaderDataType::Int4:		return GL_INT;
		case Engine::ShaderDataType::Mat3:		return GL_FLOAT;
		case Engine::ShaderDataType::Mat4:		return GL_FLOAT;
		case Engine::ShaderDataType::Bool:		return GL_BOOL;
		default:
			ENGINE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		Renderer::Submit([this]()
			{
				glCreateVertexArrays(1, &m_RendererID);
			}
		);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]()
			{
				glDeleteVertexArrays(1, &rendererID);
			}
		);
	}

	void OpenGLVertexArray::Bind()
	{
		Renderer::Submit([this]()
			{
				glBindVertexArray(m_RendererID);
			}
		);
	}

	void OpenGLVertexArray::Unbind()
	{
		Renderer::Submit([this]()
			{
				glBindVertexArray(0);
			}
		);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		ENGINE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "VertexBuffer has no layout!");

		Bind();
		vertexBuffer->Bind();

		Renderer::Submit([this, vertexBuffer]()
			{
				const auto& layout = vertexBuffer->GetLayout();
				uint32_t index = 0;
				for (const auto& element : layout) {
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(
						index,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)element.Offset
					);
					index++;
				}
			}
		);

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		Bind();
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer; 
	}
}
#include "pch.h"
#include "OpenGLPipeline.h"

#include <glad/glad.h>
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return GL_FLOAT;
		case ShaderDataType::Float2:   return GL_FLOAT;
		case ShaderDataType::Float3:   return GL_FLOAT;
		case ShaderDataType::Float4:   return GL_FLOAT;
		case ShaderDataType::Mat3:     return GL_FLOAT;
		case ShaderDataType::Mat4:     return GL_FLOAT;
		case ShaderDataType::Int:      return GL_INT;
		case ShaderDataType::Int2:     return GL_INT;
		case ShaderDataType::Int3:     return GL_INT;
		case ShaderDataType::Int4:     return GL_INT;
		case ShaderDataType::Bool:     return GL_BOOL;
		}
		ENGINE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

    OpenGLPipeline::OpenGLPipeline(const PipelineSpecification& spec)
		:m_Specification(spec)
    {
		Initialize();
    }

    OpenGLPipeline::~OpenGLPipeline()
    {
		GLuint rendererID = m_VertexArrayRendererID;
		Renderer::Submit([rendererID]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Destroy vertexArray({0})", rendererID);
				glDeleteVertexArrays(1, &rendererID);
			}
		);
    }

    void OpenGLPipeline::Bind()
    {
		Renderer::Submit([this]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Bind vertexArray({0})", m_VertexArrayRendererID);
				glBindVertexArray(m_VertexArrayRendererID);

				const auto& layout = m_Specification.Layout;
				uint32_t attribIndex = 0;
				for (const auto& element : layout)
				{
					auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);
					glEnableVertexAttribArray(attribIndex);
					if (glBaseType == GL_INT)
					{
						glVertexAttribIPointer(
							attribIndex,
							element.GetComponentCount(),
							glBaseType,
							layout.GetStride(),
							(const void*)(intptr_t)element.Offset
						);
					}
					else
					{
						glVertexAttribPointer(
							attribIndex,
							element.GetComponentCount(),
							glBaseType,
							element.Normalized ? GL_TRUE : GL_FALSE,
							layout.GetStride(),
							(const void*)(intptr_t)element.Offset
						);
					}
					attribIndex++;
				}
			}
		);
    }

    void OpenGLPipeline::Initialize()
    {
		ENGINE_ASSERT(m_Specification.Layout.GetElements().size(), "Layout is empty!");

		Renderer::Submit([this]()
			{
				if (m_VertexArrayRendererID)
					glDeleteVertexArrays(1, &m_VertexArrayRendererID);
				
				glGenVertexArrays(1, &m_VertexArrayRendererID);

				RENDERCOMMAND_TRACE("RenderCommand: Construct vertexArray({0})", m_VertexArrayRendererID);
			}
		);
    }
}
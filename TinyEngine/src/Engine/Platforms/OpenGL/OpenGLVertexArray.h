#pragma once

#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/VertexBuffer.h"

namespace Engine
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();

		virtual uint32_t GetRendererID() const { return m_RendererID; };
		virtual void Bind() override;
		virtual void Unbind() override;

	private:
		uint32_t m_RendererID = 0;
	};
}
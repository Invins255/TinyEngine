#pragma once

#include "Engine/Core/Buffer.h"
#include "Engine/Renderer/VertexBuffer.h"

namespace Engine
{
	//-------------------------------------------------------------------------
	//OpenGLVertexBuffer
	//-------------------------------------------------------------------------
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
		OpenGLVertexBuffer(void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
		virtual ~OpenGLVertexBuffer();

		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual uint32_t GetSize() const override { return m_Size; }

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(void* data, uint32_t size, uint32_t offset = 0) override;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size;
		VertexBufferUsage m_Usage;

		Buffer m_LocalData;
	};
}
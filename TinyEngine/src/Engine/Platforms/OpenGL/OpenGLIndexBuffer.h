#pragma once

#include "Engine/Core/Buffer.h"
#include "Engine/Renderer/IndexBuffer.h"

namespace Engine
{
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t size);
		OpenGLIndexBuffer(void* data, uint32_t size);
		virtual ~OpenGLIndexBuffer();

		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(void* data, uint32_t size, uint32_t offset = 0) override;
		virtual uint32_t GetCount() const override { return m_Size / sizeof(uint32_t); }
		virtual uint32_t GetSize() const override { return m_Size; }
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Size;

		Buffer m_LocalData;
	};
}
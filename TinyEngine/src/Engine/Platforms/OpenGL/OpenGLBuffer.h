#pragma once

#include "Engine/Renderer/Buffer.h"

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
		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		virtual uint32_t GetSize() const override { return m_Size; }
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

	private:
		uint32_t m_RendererID = 0;
		BufferLayout m_Layout;
		uint32_t m_Size;
		VertexBufferUsage m_Usage;
	};

	//-------------------------------------------------------------------------
	//OpenGLIndexBuffer
	//-------------------------------------------------------------------------
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t* data, uint32_t size);
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
	};
}
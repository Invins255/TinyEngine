#pragma once

#include "Engine/Core/Ref.h"

namespace Engine
{
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual uint32_t GetRendererID() const = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetSize() const = 0;

		/// <summary>
		/// IndexBuffer工厂函数
		/// </summary>
		/// <param name="size">数组总长度</param>
		static Ref<IndexBuffer> IndexBuffer::Create(uint32_t size);
		/// <summary>
		/// IndexBuffer工厂函数
		/// </summary>
		/// <param name="data">顶点下标数组</param>
		/// <param name="size">数组总长度</param>
		static Ref<IndexBuffer> Create(void* data, uint32_t size);
	};
}
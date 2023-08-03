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
		/// IndexBuffer��������
		/// </summary>
		/// <param name="size">�����ܳ���</param>
		static Ref<IndexBuffer> IndexBuffer::Create(uint32_t size);
		/// <summary>
		/// IndexBuffer��������
		/// </summary>
		/// <param name="data">�����±�����</param>
		/// <param name="size">�����ܳ���</param>
		static Ref<IndexBuffer> Create(void* data, uint32_t size);
	};
}
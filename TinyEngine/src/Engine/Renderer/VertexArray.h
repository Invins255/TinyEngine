#pragma once

#include <memory>
#include "Engine/Renderer/VertexBuffer.h"
#include "Engine/Renderer/IndexBuffer.h"

namespace Engine
{
	/// <summary>
	/// VertexArray��¼��һϵ�ж����������ݡ�һ��VertexArray����ӵ��һ��IndexBuffer��һ������VertexBuffer�����ǹ�ͬ���һ������������������ݡ�
	/// </summary>
	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual uint32_t GetRendererID() const = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

		/// <summary>
		/// VertexArray��������
		/// </summary>
		static Ref<VertexArray> Create();
	};
}
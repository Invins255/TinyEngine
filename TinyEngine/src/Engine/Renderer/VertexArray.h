#pragma once

#include <memory>
#include "Engine/Renderer/VertexBuffer.h"
#include "Engine/Renderer/IndexBuffer.h"

namespace Engine
{
	/// <summary>
	/// VertexArray记录了一系列顶点的相关数据。一个VertexArray可以拥有一个IndexBuffer和一个或多个VertexBuffer，它们共同组成一个顶点数组的所有数据。
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
		/// VertexArray工厂函数
		/// </summary>
		static Ref<VertexArray> Create();
	};
}
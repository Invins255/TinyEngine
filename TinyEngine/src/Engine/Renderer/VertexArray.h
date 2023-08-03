#pragma once

#include <memory>
#include "Engine/Renderer/VertexBuffer.h"
#include "Engine/Renderer/IndexBuffer.h"

namespace Engine
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual uint32_t GetRendererID() const = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		/// <summary>
		/// VertexArray¹¤³§º¯Êý
		/// </summary>
		static Ref<VertexArray> Create();
	};
}
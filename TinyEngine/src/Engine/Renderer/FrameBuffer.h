#pragma once

#include <glm/glm.hpp>
#include "Engine/Core/Core.h"

namespace Engine
{
	struct  FrameBufferSpecification
	{
		uint32_t Width, Height;
		glm::vec4 ClearColor;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual const FrameBufferSpecification& GetSpecification() const = 0;
		virtual uint32_t GetColorAttachmentID() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);	
	};
}


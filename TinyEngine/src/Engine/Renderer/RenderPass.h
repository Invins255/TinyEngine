#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/FrameBuffer.h"

namespace Engine
{
	struct RenderPassSpecification
	{
		Ref<FrameBuffer> TargetFramebuffer;
	};

	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& spec);
	};
}
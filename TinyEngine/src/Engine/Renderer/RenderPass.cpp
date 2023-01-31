#include "pch.h"
#include "RenderPass.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Platforms/OpenGL/OpenGLRenderPass.h"

namespace Engine
{
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			return CreateRef<OpenGLRenderPass>(spec);
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}
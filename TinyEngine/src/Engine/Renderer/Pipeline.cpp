#include "pch.h"
#include "Pipeline.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Platforms/OpenGL/OpenGLPipeline.h"

namespace Engine
{
	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
	{
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			return CreateRef<OpenGLPipeline>(spec);
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}
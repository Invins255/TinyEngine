#include "pch.h"
#include "FrameBuffer.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Platforms/OpenGL/OpenGLFrameBuffer.h"

namespace Engine
{
	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
	{
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			return CreateRef<OpenGLFrameBuffer>(spec);
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}
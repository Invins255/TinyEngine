#include "pch.h"
#include "VertexArray.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Platforms/OpenGL/OpenGLVertexArray.h"

namespace Engine
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			return CreateRef<OpenGLVertexArray>();
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
	}
}
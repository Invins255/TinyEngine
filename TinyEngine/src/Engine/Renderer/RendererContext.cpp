#include "pch.h"
#include "RendererContext.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Platforms/OpenGL/OpenGLContext.h"

namespace Engine
{
    Ref<RendererContext> RendererContext::Create(GLFWwindow* windowHandle)
    {
		switch (Renderer::GetAPIType())
		{
		case RendererAPI::RendererAPIType::None:
			ENGINE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::RendererAPIType::OpenGL:
			return CreateRef<OpenGLContext>(windowHandle);
		default:
			ENGINE_ASSERT(false, "Unknown RendererAPI!");
			return nullptr;
		}
    }
}

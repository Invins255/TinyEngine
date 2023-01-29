#include "pch.h"
#include "Renderer.h"

#include "Engine/Platforms/OpenGL/OpenGLShader.h"
#include "Engine/Renderer/Renderer2D.h"

namespace Engine
{
	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}
}
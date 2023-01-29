#pragma once

#include "Engine/Renderer/RenderCommand.h"

namespace Engine
{
	class Renderer
	{
	public:
		static void Init();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static void OnWindowResize(uint32_t width, uint32_t height);
	
	private:

	};
}
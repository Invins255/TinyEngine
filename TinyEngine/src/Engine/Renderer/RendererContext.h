#pragma once

#include <GLFW/glfw3.h>
#include "Engine/Core/Core.h"

namespace Engine
{
	class RendererContext
	{
	public:
		RendererContext() = default;
		virtual ~RendererContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;

		static Ref<RendererContext> Create(GLFWwindow* windowHandle);
	};
}